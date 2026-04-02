/*
 * Miku OS - Inter-Process Communication (IPC) Subsystem
 * 
 * Features:
 * - Pipes (anonymous and named/FIFOs)
 * - System V Semaphores
 * - System V Message Queues
 * - System V Shared Memory
 * - POSIX Signals
 */

#include "miku_os.h"

/* IPC global structures */
#define MAX_PIPES       1024
#define MAX_SEMAPHORES  256
#define MAX_MSG_QUEUES  256
#define MAX_SHM_REGIONS 256

static struct pipe_device g_pipes[MAX_PIPES];
static sem_array_t g_semaphores[MAX_SEMAPHORES];
static msg_queue_t g_msg_queues[MAX_MSG_QUEUES];
static shmid_kernel_t g_shm_regions[MAX_SHM_REGIONS];

/* ============================================================================
 * IPC INITIALIZATION
 * ============================================================================ */
void ipc_init(void) {
    u32 i;
    
    log_info("Initializing IPC subsystem...");
    
    /* Initialize pipes */
    for (i = 0; i < MAX_PIPES; i++) {
        // g_pipes[i].used = false;
    }
    
    /* Initialize semaphores */
    for (i = 0; i < MAX_SEMAPHORES; i++) {
        memset(&g_semaphores[i], 0, sizeof(sem_array_t));
        spinlock_init(&g_semaphores[i].lock);
    }
    
    /* Initialize message queues */
    for (i = 0; i < MAX_MSG_QUEUES; i++) {
        memset(&g_msg_queues[i], 0, sizeof(msg_queue_t));
        spinlock_init(&g_msg_queues[i].lock);
    }
    
    /* Initialize shared memory regions */
    for (i = 0; i < MAX_SHM_REGIONS; i++) {
        memset(&g_shm_regions[i], 0, sizeof(shmid_kernel_t));
        spinlock_init(&g_shm_regions[i].lock);
    }
    
    log_info("IPC subsystem initialized");
}

/* ============================================================================
 * PIPES
 * ============================================================================ */
int sys_pipe(int pipefd[2]) {
    u32 i;
    int read_fd, write_fd;
    
    if (!pipefd) {
        return -EINVAL;
    }
    
    log_debug("IPC: Creating pipe");
    
    /* Find free pipe slot */
    for (i = 0; i < MAX_PIPES; i++) {
        // if (!g_pipes[i].used) {
        //     break;
        // }
    }
    
    if (i >= MAX_PIPES) {
        return -EMFILE;
    }
    
    /* Allocate file descriptors */
    // read_fd = alloc_fd();
    // write_fd = alloc_fd();
    read_fd = 100 + i * 2;     /* Placeholder */
    write_fd = 100 + i * 2 + 1; /* Placeholder */
    
    if (read_fd < 0 || write_fd < 0) {
        return -EMFILE;
    }
    
    /* Initialize pipe */
    // g_pipes[i].used = true;
    // g_pipes[i].buffer = kmalloc(PIPE_BUF_SIZE);
    // g_pipes[i].read_pos = 0;
    // g_pipes[i].write_pos = 0;
    // g_pipes[i].readers = 1;
    // g_pipes[i].writers = 1;
    // spinlock_init(&g_pipes[i].lock);
    
    /* Create file structures */
    // g_current_task->files[read_fd] = create_pipe_file(&g_pipes[i], O_RDONLY);
    // g_current_task->files[write_fd] = create_pipe_file(&g_pipes[i], O_WRONLY);
    
    pipefd[0] = read_fd;
    pipefd[1] = write_fd;
    
    log_debug("IPC: Pipe created (read=%d, write=%d)", read_fd, write_fd);
    
    return 0;
}

int sys_fifo(const char *path, mode_t mode) {
    if (!path) {
        return -EINVAL;
    }
    
    log_debug("IPC: Creating FIFO '%s'", path);
    
    /* Create special file in VFS */
    // return vfs_mknod(path, S_IFIFO | mode, 0);
    
    return -ENOSYS; /* Not implemented */
}

ssize_t pipe_read(struct file *file, char *buf, size_t count) {
    // struct pipe_device *pipe = file->private_data;
    ssize_t bytes_read = 0;
    
    if (!file || !buf || count == 0) {
        return -EINVAL;
    }
    
    /* Wait for data if pipe is empty */
    // while (pipe->read_pos == pipe->write_pos) {
    //     if (pipe->writers == 0) {
    //         return 0; /* EOF */
    //     }
    //     wait_event_interruptible(pipe->wait_read);
    // }
    
    /* Read data from pipe buffer */
    // bytes_read = min(count, pipe->write_pos - pipe->read_pos);
    // memcpy(buf, pipe->buffer + pipe->read_pos, bytes_read);
    // pipe->read_pos += bytes_read;
    
    /* Wake up writers */
    // wake_up(&pipe->wait_write);
    
    return bytes_read;
}

ssize_t pipe_write(struct file *file, const char *buf, size_t count) {
    // struct pipe_device *pipe = file->private_data;
    ssize_t bytes_written = 0;
    
    if (!file || !buf || count == 0) {
        return -EINVAL;
    }
    
    /* Wait for space if pipe is full */
    // while (pipe_is_full(pipe)) {
    //     if (pipe->readers == 0) {
    //         return -EPIPE;
    //     }
    //     wait_event_interruptible(pipe->wait_write);
    // }
    
    /* Write data to pipe buffer */
    // bytes_written = min(count, PIPE_BUF_SIZE - (pipe->write_pos - pipe->read_pos));
    // memcpy(pipe->buffer + pipe->write_pos, buf, bytes_written);
    // pipe->write_pos += bytes_written;
    
    /* Wake up readers */
    // wake_up(&pipe->wait_read);
    
    return bytes_written;
}

/* ============================================================================
 * SEMAPHORES
 * ============================================================================ */
int sys_semget(key_t key, int nsems, int flags) {
    u32 i;
    int sem_id;
    
    if (nsems <= 0 || nsems > SEMMSL) {
        return -EINVAL;
    }
    
    log_debug("IPC: Creating semaphore (key=%x, nsems=%d)", key, nsems);
    
    /* Check if semaphore with this key already exists */
    for (i = 0; i < MAX_SEMAPHORES; i++) {
        // if (g_semaphores[i].perm.key == key && key != IPC_PRIVATE) {
        //     return i;
        // }
    }
    
    /* Find free slot */
    for (i = 0; i < MAX_SEMAPHORES; i++) {
        if (g_semaphores[i].perm.key == 0) {
            break;
        }
    }
    
    if (i >= MAX_SEMAPHORES) {
        return -ENOSPC;
    }
    
    sem_id = i;
    
    /* Initialize semaphore array */
    g_semaphores[sem_id].perm.key = key;
    g_semaphores[sem_id].perm.uid = g_current_task ? g_current_task->uid : 0;
    g_semaphores[sem_id].perm.gid = g_current_task ? g_current_task->gid : 0;
    g_semaphores[sem_id].perm.mode = flags & 0777;
    g_semaphores[sem_id].nsems = nsems;
    g_semaphores[sem_id].otime = 0;
    g_semaphores[sem_id].ctime = time_get_unix_time();
    
    /* Allocate semaphores */
    // g_semaphores[sem_id].sem = kzalloc(nsems * sizeof(struct sem));
    
    /* Initialize each semaphore */
    // for (i = 0; i < nsems; i++) {
    //     g_semaphores[sem_id].sem[i].count = 1;
    //     INIT_LIST_HEAD(&g_semaphores[sem_id].sem[i].wait_queue);
    // }
    
    log_debug("IPC: Semaphore created (id=%d)", sem_id);
    
    return sem_id;
}

int sys_semop(int semid, struct sembuf *sops, unsigned nsops) {
    int i;
    
    if (semid < 0 || semid >= MAX_SEMAPHORES || !sops) {
        return -EINVAL;
    }
    
    if (nsops == 0) {
        return 0;
    }
    
    log_debug("IPC: Semaphore operation (semid=%d, nsops=%u)", semid, nsops);
    
    /* Perform semaphore operations */
    for (i = 0; i < nsops; i++) {
        // struct sem *sem = &g_semaphores[semid].sem[sops[i].sem_num];
        
        /* Wait or signal */
        // if (sops[i].sem_op < 0) {
        //     /* P operation (wait) */
        //     while (sem->count < -sops[i].sem_op) {
        //         add_to_wait_queue(sem, current);
        //         schedule();
        //     }
        //     sem->count += sops[i].sem_op;
        // } else if (sops[i].sem_op > 0) {
        //     /* V operation (signal) */
        //     sem->count += sops[i].sem_op;
        //     wake_up_waiters(sem);
        // }
    }
    
    g_semaphores[semid].otime = time_get_unix_time();
    
    return 0;
}

/* ============================================================================
 * MESSAGE QUEUES
 * ============================================================================ */
int sys_msgget(key_t key, int flags) {
    u32 i;
    int mq_id;
    
    log_debug("IPC: Creating message queue (key=%x)", key);
    
    /* Check if queue already exists */
    for (i = 0; i < MAX_MSG_QUEUES; i++) {
        // if (g_msg_queues[i].perm.key == key && key != IPC_PRIVATE) {
        //     return i;
        // }
    }
    
    /* Find free slot */
    for (i = 0; i < MAX_MSG_QUEUES; i++) {
        if (g_msg_queues[i].perm.key == 0) {
            break;
        }
    }
    
    if (i >= MAX_MSG_QUEUES) {
        return -ENOSPC;
    }
    
    mq_id = i;
    
    /* Initialize message queue */
    g_msg_queues[mq_id].perm.key = key;
    g_msg_queues[mq_id].perm.uid = g_current_task ? g_current_task->uid : 0;
    g_msg_queues[mq_id].perm.gid = g_current_task ? g_current_task->gid : 0;
    g_msg_queues[mq_id].perm.mode = flags & 0777;
    g_msg_queues[mq_id].qnum = 0;
    g_msg_queues[mq_id].qbytes = MSGMNB;
    g_msg_queues[mq_id].lspid = 0;
    g_msg_queues[mq_id].lrpid = 0;
    g_msg_queues[mq_id].stime = 0;
    g_msg_queues[mq_id].rtime = 0;
    g_msg_queues[mq_id].ctime = time_get_unix_time();
    
    // INIT_LIST_HEAD(&g_msg_queues[mq_id].q_messages);
    
    log_debug("IPC: Message queue created (id=%d)", mq_id);
    
    return mq_id;
}

int sys_msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg) {
    struct msg_msg *msg;
    
    if (msqid < 0 || msqid >= MAX_MSG_QUEUES || !msgp) {
        return -EINVAL;
    }
    
    if (msgsz > MSGMAX) {
        return -EINVAL;
    }
    
    log_debug("IPC: Sending message (msqid=%d, size=%zu)", msqid, msgsz);
    
    /* Allocate message */
    // msg = kmalloc(sizeof(struct msg_msg) + msgsz);
    // if (!msg) {
    //     return -ENOMEM;
    // }
    
    /* Copy message from user space */
    // copy_from_user(msg->mtext, msgp + sizeof(long), msgsz);
    // msg->mtype = *(long *)msgp;
    // msg->m_sz = msgsz;
    
    /* Add to queue */
    // list_add_tail(&msg->list, &g_msg_queues[msqid].q_messages);
    // g_msg_queues[msqid].qnum++;
    // g_msg_queues[msqid].qbytes += msgsz;
    // g_msg_queues[msqid].lspid = g_current_task->pid;
    // g_msg_queues[msqid].stime = time_get_unix_time();
    
    /* Wake up receivers */
    // wake_up(&g_msg_queues[msqid].wait_receive);
    
    return 0;
}

ssize_t sys_msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg) {
    struct msg_msg *msg;
    ssize_t ret;
    
    if (msqid < 0 || msqid >= MAX_MSG_QUEUES || !msgp) {
        return -EINVAL;
    }
    
    log_debug("IPC: Receiving message (msqid=%d, type=%ld)", msqid, msgtyp);
    
    /* Find matching message */
    // msg = find_message(&g_msg_queues[msqid], msgtyp);
    msg = NULL;
    
    if (!msg) {
        if (msgflg & IPC_NOWAIT) {
            return -ENOMSG;
        }
        
        /* Wait for message */
        // wait_event_interruptible(g_msg_queues[msqid].wait_receive);
        return -EINTR;
    }
    
    /* Copy message to user space */
    // *(long *)msgp = msg->mtype;
    // ret = min(msgsz, msg->m_sz);
    // copy_to_user(msgp + sizeof(long), msg->mtext, ret);
    
    /* Remove message from queue */
    // list_del(&msg->list);
    // g_msg_queues[msqid].qnum--;
    // g_msg_queues[msqid].qbytes -= msg->m_sz;
    // g_msg_queues[msqid].lrpid = g_current_task->pid;
    // g_msg_queues[msqid].rtime = time_get_unix_time();
    
    /* Free message */
    // kfree(msg);
    
    /* Wake up senders */
    // wake_up(&g_msg_queues[msqid].wait_send);
    
    return ret;
}

/* ============================================================================
 * SHARED MEMORY
 * ============================================================================ */
int sys_shmget(key_t key, size_t size, int flags) {
    u32 i;
    int shm_id;
    
    if (size == 0 || size > SHMMAX) {
        return -EINVAL;
    }
    
    log_debug("IPC: Creating shared memory (key=%x, size=%zu)", key, size);
    
    /* Check if segment already exists */
    for (i = 0; i < MAX_SHM_REGIONS; i++) {
        // if (g_shm_regions[i].perm.key == key && key != IPC_PRIVATE) {
        //     return i;
        // }
    }
    
    /* Find free slot */
    for (i = 0; i < MAX_SHM_REGIONS; i++) {
        if (g_shm_regions[i].perm.key == 0) {
            break;
        }
    }
    
    if (i >= MAX_SHM_REGIONS) {
        return -ENOSPC;
    }
    
    shm_id = i;
    
    /* Initialize shared memory region */
    g_shm_regions[shm_id].perm.key = key;
    g_shm_regions[shm_id].perm.uid = g_current_task ? g_current_task->uid : 0;
    g_shm_regions[shm_id].perm.gid = g_current_task ? g_current_task->gid : 0;
    g_shm_regions[shm_id].perm.mode = flags & 0777;
    g_shm_regions[shm_id].shm_segsz = size;
    g_shm_regions[shm_id].shm_lpid = 0;
    g_shm_regions[shm_id].shm_cpid = g_current_task ? g_current_task->pid : 0;
    g_shm_regions[shm_id].shm_nattch = 0;
    g_shm_regions[shm_id].shm_atim = 0;
    g_shm_regions[shm_id].shm_dtim = 0;
    g_shm_regions[shm_id].shm_ctim = time_get_unix_time();
    
    /* Allocate backing store */
    // g_shm_regions[shm_id].shm_file = alloc_file();
    // g_shm_regions[shm_id].shm_addr = vmalloc(size);
    
    log_debug("IPC: Shared memory created (id=%d)", shm_id);
    
    return shm_id;
}

void *sys_shmat(int shmid, const void *shmaddr, int shmflg) {
    void *addr;
    
    if (shmid < 0 || shmid >= MAX_SHM_REGIONS) {
        return (void *)(long)-EINVAL;
    }
    
    log_debug("IPC: Attaching shared memory (shmid=%d)", shmid);
    
    /* Find suitable address */
    // addr = find_map_region(shmaddr, g_shm_regions[shmid].shm_segsz);
    addr = (void *)0x40000000ULL; /* Placeholder */
    
    /* Map shared memory into process address space */
    // mm_map(current->mm, (u64)addr, g_shm_regions[shmid].shm_segsz, PROT_READ | PROT_WRITE, MAP_SHARED);
    
    g_shm_regions[shmid].shm_nattch++;
    g_shm_regions[shmid].shm_lpid = g_current_task ? g_current_task->pid : 0;
    g_shm_regions[shmid].shm_atim = time_get_unix_time();
    
    return addr;
}

int sys_shmdt(const void *shmaddr) {
    int i;
    
    if (!shmaddr) {
        return -EINVAL;
    }
    
    log_debug("IPC: Detaching shared memory (addr=%p)", shmaddr);
    
    /* Find the shared memory region */
    for (i = 0; i < MAX_SHM_REGIONS; i++) {
        // if (g_shm_regions[i].shm_addr == shmaddr) {
        //     break;
        // }
    }
    
    if (i >= MAX_SHM_REGIONS) {
        return -EINVAL;
    }
    
    /* Unmap from process address space */
    // mm_unmap(current->mm, (u64)shmaddr, g_shm_regions[i].shm_segsz);
    
    g_shm_regions[i].shm_nattch--;
    g_shm_regions[i].shm_dtim = time_get_unix_time();
    
    /* Destroy if no more attachments */
    if (g_shm_regions[i].shm_nattch == 0 && 
        (g_shm_regions[i].perm.mode & IPC_RMID)) {
        // destroy_shm(i);
    }
    
    return 0;
}

int sys_shmctl(int shmid, int cmd, struct shmid_ds *buf) {
    if (shmid < 0 || shmid >= MAX_SHM_REGIONS) {
        return -EINVAL;
    }
    
    switch (cmd) {
        case IPC_STAT:
            /* Get status */
            // copy_to_user(buf, &g_shm_regions[shmid], sizeof(struct shmid_ds));
            break;
            
        case IPC_SET:
            /* Set permissions */
            // copy_from_user(&g_shm_regions[shmid].perm, buf, sizeof(ipc_perm_t));
            break;
            
        case IPC_RMID:
            /* Mark for removal */
            g_shm_regions[shmid].perm.mode |= IPC_RMID;
            
            /* Destroy immediately if no attachments */
            if (g_shm_regions[shmid].shm_nattch == 0) {
                // destroy_shm(shmid);
            }
            break;
            
        default:
            return -EINVAL;
    }
    
    return 0;
}

/* ============================================================================
 * SIGNALS
 * ============================================================================ */
int task_send_signal(task_struct_t *task, int sig) {
    u64 irq_flags;
    
    if (!task || sig < 0 || sig >= NSIG) {
        return -EINVAL;
    }
    
    irq_save_and_disable(&irq_flags);
    spinlock_lock(&task->signal->siglock);
    
    /* Set pending signal */
    task->signal->sigpending |= (1ULL << sig);
    
    /* Wake up task if sleeping */
    if (task->state == THREAD_SLEEPING) {
        scheduler_wakeup(task);
    }
    
    spinlock_unlock(&task->signal->siglock);
    irq_restore(irq_flags);
    
    return 0;
}

int sys_kill(pid_t pid, int sig) {
    task_struct_t *task;
    
    if (sig < 0 || sig >= NSIG) {
        return -EINVAL;
    }
    
    if (pid == 0) {
        /* Send to all processes in same process group */
        return -ENOSYS;
    } else if (pid == -1) {
        /* Send to all processes except init */
        return -ENOSYS;
    } else if (pid < -1) {
        /* Send to process group */
        return -ENOSYS;
    }
    
    /* Send to specific PID */
    task = task_find(pid);
    if (!task) {
        return -ESRCH;
    }
    
    return task_send_signal(task, sig);
}

void handle_signals(void) {
    task_struct_t *task = g_current_task;
    u64 pending;
    int sig;
    
    if (!task || !task->signal) {
        return;
    }
    
    spinlock_lock(&task->signal->siglock);
    pending = task->signal->sigpending & ~task->signal->sigignore;
    spinlock_unlock(&task->signal->siglock);
    
    while (pending) {
        sig = __builtin_ctzll(pending);
        
        if (sig >= NSIG) {
            break;
        }
        
        /* Clear pending bit */
        task->signal->sigpending &= ~(1ULL << sig);
        
        /* Handle signal */
        if (task->signal->handlers[sig]) {
            /* Call user handler */
            // call_user_handler(task->signal->handlers[sig], sig);
        } else if (sig == SIGKILL || sig == SIGSTOP) {
            /* Default action */
            if (sig == SIGKILL) {
                task_exit(128 + sig);
            }
        }
        
        pending &= ~(1ULL << sig);
    }
}
