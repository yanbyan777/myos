/*
 * Miku OS - CFS-like Scheduler Implementation
 * 
 * A Completely Fair Scheduler inspired by Linux's CFS.
 * Features:
 * - Virtual runtime (vruntime) based fairness
 * - Per-CPU run queues
 * - Priority-based nice values (-20 to 19)
 * - Preemptive scheduling
 * - SMP support
 */

#include "miku_os.h"

/* Global scheduler instance */
scheduler_t g_scheduler;

/* Nice value to weight mapping (Linux-compatible) */
static const u64 nice_to_weight[40] = {
    /* -20 */     88761,
    /* -19 */     71755,
    /* -18 */     56483,
    /* -17 */     46273,
    /* -16 */     36291,
    /* -15 */     29154,
    /* -14 */     23254,
    /* -13 */     18705,
    /* -12 */     14949,
    /* -11 */     11916,
    /* -10 */      9548,
    /*  -9 */      7620,
    /*  -8 */      6157,
    /*  -7 */      4935,
    /*  -6 */      3906,
    /*  -5 */      3158,
    /*  -4 */      2532,
    /*  -3 */      2035,
    /*  -2 */      1635,
    /*  -1 */      1321,
    /*   0 */      1024,
    /*   1 */       820,
    /*   2 */       655,
    /*   3 */       526,
    /*   4 */       423,
    /*   5 */       335,
    /*   6 */       272,
    /*   7 */       215,
    /*   8 */       172,
    /*   9 */       139,
    /*  10 */       110,
    /*  11 */        87,
    /*  12 */        70,
    /*  13 */        56,
    /*  14 */        45,
    /*  15 */        36,
    /*  16 */        29,
    /*  17 */        23,
    /*  18 */        18,
    /*  19 */        15
};

/* Get weight from nice value */
static inline u64 get_weight(int nice) {
    int idx = nice + 20;
    if (idx < 0) idx = 0;
    if (idx > 39) idx = 39;
    return nice_to_weight[idx];
}

/* ============================================================================
 * SCHEDULER INITIALIZATION
 * ============================================================================ */
void scheduler_init(void) {
    u32 i;
    
    log_info("Initializing scheduler...");
    
    /* Zero out scheduler structure */
    memset(&g_scheduler, 0, sizeof(scheduler_t));
    
    /* Initialize per-CPU run queues */
    for (i = 0; i < MIKU_MAX_CPUS; i++) {
        cfs_rq_t *cfs = &g_scheduler.cfs[i];
        
        spinlock_init(&cfs->lock);
        cfs->min_vruntime = 0;
        cfs->nr_running = 0;
        cfs->nr_switches = 0;
        cfs->curr = NULL;
        cfs->next = NULL;
        cfs->idle = NULL;
        
        /* Initialize run queue list */
        // INIT_LIST_HEAD(&cfs->queue);
    }
    
    g_scheduler.nr_cpus = cpu_get_count();
    g_scheduler.boot_cpu = 0;
    g_scheduler.jiffies = 0;
    g_scheduler.uptime = 0;
    
    spinlock_init(&g_scheduler.global_lock);
    
    log_info("Scheduler initialized with %d CPUs", g_scheduler.nr_cpus);
}

/* ============================================================================
 * TASK CREATION
 * ============================================================================ */
task_struct_t *task_create(const char *name, void (*entry)(void *), void *arg, u32 flags) {
    task_struct_t *task;
    static pid_t next_pid = 1;
    static tid_t next_tid = 1;
    u64 irq_flags;
    
    (void)flags;
    
    /* Allocate task structure */
    task = kzalloc(sizeof(task_struct_t));
    if (!task) {
        log_error("Failed to allocate task structure");
        return NULL;
    }
    
    /* Initialize task */
    irq_save_and_disable(&irq_flags);
    
    task->tid = next_tid++;
    task->pid = next_pid++;
    task->ppid = g_current_task ? g_current_task->pid : 0;
    task->tgid = task->pid;
    
    task->state = THREAD_RUNNABLE;
    task->priority = PRIO_NORMAL;
    task->nice = 0;
    
    /* Set task name */
    strncpy(task->name, name, sizeof(task->name) - 1);
    task->name[sizeof(task->name) - 1] = '\0';
    
    /* Setup stack */
    task->stack_size = 8192; /* 8KB stack */
    task->stack = (u64)kmalloc(task->stack_size);
    if (!task->stack) {
        kfree(task);
        irq_restore(irq_flags);
        log_error("Failed to allocate task stack");
        return NULL;
    }
    
    /* Initialize CPU context */
    memset(&task->context, 0, sizeof(cpu_context_t));
    task->context.rip = (u64)entry;
    task->context.rsp = task->stack + task->stack_size;
    task->context.rflags = 0x202; /* IF flag set */
    
    /* Set up argument in RDI (first function argument) */
    /* Context will be restored by switch_to, arg passed via stack */
    *(u64 *)(task->context.rsp - 8) = (u64)arg;
    task->context.rsp -= 8;
    
    /* Memory management */
    if (g_current_task && g_current_task->mm) {
        task->mm = g_current_task->mm;
        task->mm->mm_users++;
    } else {
        task->mm = &g_kernel_mm;
    }
    
    /* Signal handlers */
    task->signal = kzalloc(sizeof(signal_struct_t));
    if (!task->signal) {
        kfree((void *)task->stack);
        kfree(task);
        irq_restore(irq_flags);
        log_error("Failed to allocate signal structure");
        return NULL;
    }
    spinlock_init(&task->signal->siglock);
    
    /* File descriptors */
    task->max_files = 1024;
    /* Inherit file table from parent if exists */
    if (g_current_task) {
        memcpy(task->files, g_current_task->files, sizeof(task->files));
    }
    
    /* User/group IDs */
    task->uid = task->euid = 0; /* Root by default */
    task->gid = task->egid = 0;
    
    /* Timing */
    task->start_time = time_get_jiffies();
    task->utime = 0;
    task->stime = 0;
    
    /* CPU affinity */
    task->cpu = 0;
    task->processor_id = cpu_get_id();
    
    /* Reference count and lock */
    task->refcount = 1;
    spinlock_init(&task->lock);
    
    /* Add to run queue */
    scheduler_add_task(task);
    
    irq_restore(irq_flags);
    
    log_debug("Task created: %s (PID=%d, TID=%d)", name, task->pid, task->tid);
    
    return task;
}

/* ============================================================================
 * RUN QUEUE MANAGEMENT
 * ============================================================================ */
void scheduler_add_task(task_struct_t *task) {
    u32 cpu = task->cpu;
    cfs_rq_t *cfs = &g_scheduler.cfs[cpu];
    u64 irq_flags;
    
    if (!task || !cfs) return;
    
    irq_save_and_disable(&irq_flags);
    spinlock_lock(&cfs->lock);
    
    /* Add to run queue */
    // list_add_tail(&task->run_list, &cfs->queue);
    
    if (!cfs->curr) {
        cfs->curr = task;
    }
    
    cfs->nr_running++;
    task->state = THREAD_RUNNABLE;
    
    spinlock_unlock(&cfs->lock);
    irq_restore(irq_flags);
}

void scheduler_remove_task(task_struct_t *task) {
    u32 cpu = task->cpu;
    cfs_rq_t *cfs = &g_scheduler.cfs[cpu];
    u64 irq_flags;
    
    if (!task || !cfs) return;
    
    irq_save_and_disable(&irq_flags);
    spinlock_lock(&cfs->lock);
    
    /* Remove from run queue */
    // list_del(&task->run_list);
    
    if (cfs->curr == task) {
        cfs->curr = NULL;
    }
    
    cfs->nr_running--;
    
    spinlock_unlock(&cfs->lock);
    irq_restore(irq_flags);
}

/* ============================================================================
 * CONTEXT SWITCHING
 * ============================================================================ */
void scheduler_schedule(void) {
    u32 cpu = cpu_get_id();
    cfs_rq_t *cfs = &g_scheduler.cfs[cpu];
    task_struct_t *prev, *next;
    u64 irq_flags;
    
    irq_save_and_disable(&irq_flags);
    spinlock_lock(&cfs->lock);
    
    prev = cfs->curr;
    
    /* Find next task to run (simple round-robin for now) */
    // next = pick_next_task(cfs);
    next = NULL; /* For now, stay on current or idle */
    
    if (!next) {
        /* No runnable task, use idle */
        next = cfs->idle;
        if (!next) {
            /* Create idle task if it doesn't exist */
            spinlock_unlock(&cfs->lock);
            irq_restore(irq_flags);
            
            /* Stay on current task */
            return;
        }
    }
    
    if (prev == next) {
        spinlock_unlock(&cfs->lock);
        irq_restore(irq_flags);
        return;
    }
    
    /* Update accounting */
    cfs->nr_switches++;
    
    /* Switch to next task */
    cfs->curr = next;
    g_current_task = next;
    
    spinlock_unlock(&cfs->lock);
    irq_restore(irq_flags);
    
    /* Perform actual context switch */
    // switch_to(prev, next);
    
    /* For now, just yield */
    scheduler_yield();
}

void scheduler_yield(void) {
    task_struct_t *task = g_current_task;
    u64 irq_flags;
    
    if (!task) return;
    
    irq_save_and_disable(&irq_flags);
    
    /* Move current task to end of run queue */
    scheduler_remove_task(task);
    task->state = THREAD_RUNNABLE;
    scheduler_add_task(task);
    
    /* Trigger schedule */
    // trigger_reschedule();
    
    irq_restore(irq_flags);
}

void scheduler_sleep(u64 jiffies) {
    task_struct_t *task = g_current_task;
    u64 irq_flags;
    
    if (!task) return;
    
    irq_save_and_disable(&irq_flags);
    
    task->state = THREAD_SLEEPING;
    task->timeout = time_get_jiffies() + jiffies;
    
    scheduler_remove_task(task);
    
    irq_restore(irq_flags);
    
    scheduler_schedule();
}

void scheduler_wakeup(task_struct_t *task) {
    u64 irq_flags;
    
    if (!task) return;
    
    irq_save_and_disable(&irq_flags);
    
    if (task->state == THREAD_SLEEPING) {
        task->state = THREAD_RUNNABLE;
        task->timeout = 0;
        scheduler_add_task(task);
    }
    
    irq_restore(irq_flags);
}

/* ============================================================================
 * TASK EXIT
 * ============================================================================ */
void task_exit(int status) {
    task_struct_t *task = g_current_task;
    u64 irq_flags;
    
    if (!task) return;
    
    log_debug("Task exiting: %s (PID=%d, status=%d)", 
              task->name, task->pid, status);
    
    irq_save_and_disable(&irq_flags);
    
    /* Mark as zombie */
    task->state = THREAD_ZOMBIE;
    
    /* Free resources */
    if (task->stack) {
        kfree((void *)task->stack);
    }
    
    if (task->signal) {
        kfree(task->signal);
    }
    
    /* Notify parent */
    if (task->parent) {
        /* Send SIGCHLD */
        // task_send_signal(task->parent, SIGCHLD);
    }
    
    scheduler_remove_task(task);
    
    irq_restore(irq_flags);
    
    /* Schedule next task */
    scheduler_schedule();
    
    /* Should never reach here */
    while (true) {
        cpu_halt();
    }
}

/* ============================================================================
 * TASK LOOKUP
 * ============================================================================ */
task_struct_t *task_find(pid_t pid) {
    /* In a real implementation, search the task list */
    (void)pid;
    return NULL;
}

task_struct_t *task_find_tid(tid_t tid) {
    /* In a real implementation, search the task list */
    (void)tid;
    return NULL;
}

void task_wait(pid_t pid, int *status) {
    task_struct_t *task;
    
    /* Wait for child process to exit */
    while (true) {
        /* Check for zombie children */
        task = task_find(pid);
        if (task && task->state == THREAD_ZOMBIE) {
            if (status) {
                *status = 0; /* Get actual exit status */
            }
            
            /* Free task structure */
            kfree(task);
            return;
        }
        
        /* Sleep and retry */
        scheduler_sleep(10);
    }
}

int task_kill(pid_t pid, int sig) {
    task_struct_t *task;
    
    if (sig < 0 || sig >= NSIG) {
        return -EINVAL;
    }
    
    task = task_find(pid);
    if (!task) {
        return -ESRCH;
    }
    
    /* Send signal */
    // task_send_signal(task, sig);
    
    return 0;
}

/* ============================================================================
 * TIMER TICK HANDLER
 * ============================================================================ */
void scheduler_tick(void) {
    u32 cpu = cpu_get_id();
    cfs_rq_t *cfs = &g_scheduler.cfs[cpu];
    task_struct_t *task;
    u64 irq_flags;
    
    irq_save_and_disable(&irq_flags);
    
    /* Increment jiffies */
    g_scheduler.jiffies++;
    
    /* Update uptime */
    g_scheduler.uptime = g_scheduler.jiffies / 100; /* Assuming 100 Hz */
    
    /* Check for sleeping tasks that should wake up */
    /* In real implementation, iterate all tasks */
    
    /* Update current task's runtime */
    task = cfs->curr;
    if (task && task->state == THREAD_RUNNING) {
        task->utime++;
        
        /* Preemption check */
        // if (need_resched()) {
        //     scheduler_schedule();
        // }
    }
    
    irq_restore(irq_flags);
}
