# Miku OS - A Modern Linux-like Kernel

![Miku OS](https://img.shields.io/badge/version-2.0.0-blue)
![License](https://img.shields.io/badge/license-MIT-green)
![Platform](https://img.shields.io/badge/platform-x86__64-orange)

## 🎵 About Miku OS

**Miku OS** is a modern, monolithic 64-bit operating system kernel inspired by Linux. 
It features preemptive multitasking, a virtual file system, advanced memory management, 
and comprehensive IPC mechanisms - all built with love for Hatsune Miku! ♪

## ✨ Features

### 🔸 Process & Thread Management
- **CFS-like Scheduler**: Completely Fair Scheduler with vruntime-based fairness
- **Preemptive Multitasking**: Priority-based scheduling with nice values (-20 to 19)
- **SMP Support**: Multi-core processor support with per-CPU run queues
- **POSIX Signals**: Full signal handling (SIGINT, SIGTERM, SIGKILL, etc.)
- **Thread Groups**: Native thread support with shared address spaces

### 🔸 Memory Management
- **Buddy Allocator**: Physical page allocation with order-based allocation
- **Slab Allocator**: Efficient kernel object caching
- **Virtual Memory**: Per-process address spaces with paging (4KB pages)
- **Memory Zones**: DMA, Normal, and HighMem zones
- **Copy-on-Write**: COW support for fork() and mmap()

### 🔸 Virtual File System (VFS)
- **Unified Interface**: Common API for multiple filesystem types
- **Path Resolution**: Full path lookup with dentry caching
- **File Operations**: open, read, write, close, lseek
- **Directory Operations**: mkdir, rmdir, readdir, rename
- **Mount Points**: Multiple filesystem mount support
- **Permissions**: Unix-style file permissions (rwx for user/group/other)

### 🔸 Inter-Process Communication (IPC)
- **Pipes**: Anonymous pipes for parent-child communication
- **FIFOs**: Named pipes for unrelated processes
- **Semaphores**: System V semaphores for synchronization
- **Message Queues**: System V message queues
- **Shared Memory**: System V shared memory segments

### 🔸 Hardware Support
- **x86_64 Architecture**: 64-bit long mode
- **APIC/x2APIC**: Advanced Programmable Interrupt Controller
- **PIT/PIC**: Timer and interrupt management
- **PS/2 Keyboard**: Basic input support
- **VGA Console**: Text-mode display output

## 📁 Project Structure

```
miku_os/
├── arch/
│   └── x86_64/          # Architecture-specific code
│       ├── boot.asm     # Bootloader entry point
│       ├── gdt.asm      # Global Descriptor Table
│       ├── idt.asm      # Interrupt Descriptor Table
│       └── switch.asm   # Context switching
├── kernel/
│   ├── main.c           # Kernel entry point & initialization
│   ├── scheduler.c      # CFS scheduler implementation
│   ├── console.c        # VGA console driver
│   └── logging.c        # Kernel logging
├── mm/
│   └── memory.c         # Memory management subsystem
├── fs/
│   └── vfs.c            # Virtual File System
├── ipc/
│   └── ipc.c            # IPC mechanisms
├── drivers/
│   ├── keyboard.c       # PS/2 keyboard driver
│   ├── disk.c           # Disk drivers
│   └── timer.c          # PIT timer driver
├── include/
│   └── miku_os.h        # Main kernel header
├── lib/
│   ├── string.c         # String functions
│   └── printf.c         # Printf implementation
└── Makefile             # Build configuration
```

## 🚀 Building Miku OS

### Prerequisites

```bash
# Install cross-compiler (x86_64-elf-gcc)
# Ubuntu/Debian:
sudo apt-get install build-essential bison flex libgmp3-dev \
                     libmpc-dev libmpfr-dev texinfo libisl-dev

# macOS:
brew install x86_64-elf-binutils x86_64-elf-gcc
```

### Compilation

```bash
cd miku_os

# Build the kernel
make

# Build and create bootable ISO
make iso

# Run in QEMU
make run
```

## 🛠️ Kernel API

### Process Creation

```c
// Create a new thread/process
task_struct_t *task = task_create("my_thread", my_function, arg, 0);

// Exit current task
task_exit(0);

// Send signal to process
task_kill(pid, SIGTERM);
```

### File Operations

```c
// Open a file
struct file *f = vfs_open("/etc/config", O_RDONLY, 0);

// Read from file
char buf[256];
ssize_t n = vfs_read(f, buf, sizeof(buf), NULL);

// Close file
vfs_close(f);

// Create directory
vfs_mkdir("/home/miku", 0755);
```

### Memory Allocation

```c
// Kernel memory allocation
void *ptr = kmalloc(1024);
void *zeroed = kzalloc(256);

// Page allocation
page_t *page = alloc_page(0);

// Slab allocation
slab_cache_t *cache = slab_create("my_cache", 64, NULL, NULL);
void *obj = slab_alloc(cache);
```

### IPC

```c
// Create pipe
int pipefd[2];
sys_pipe(pipefd);

// Create semaphore
int semid = sys_semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);

// Create message queue
int mqid = sys_msgget(IPC_PRIVATE, 0666 | IPC_CREAT);

// Create shared memory
int shmid = sys_shmget(IPC_PRIVATE, 4096, 0666 | IPC_CREAT);
```

## 📊 System Limits

| Resource | Limit |
|----------|-------|
| Max CPUs | 256 |
| Max Threads | 65,536 |
| Max Processes | 8,192 |
| Max Files | 1,048,576 |
| Max Path Length | 4,096 bytes |
| Max Mount Points | 64 |

## 🎨 Kernel Banner

```
  __  __ _     _     ___  ____   ____  _   _ _____ 
 |  \/  | |   | |   / _ \|  _ \ / __ \| | | | ____|
 | |\/| | |   | |  | | | | |_) | |  | | | | |  _|  
 | |  | | |___| |__| |_| |  _ <| |__| | |_| | |___ 
 |_|  |_|_____|_____\___/|_| \_\\____/ \___/|_____|
                                                   
           Miku OS v2.0.0 - "Hatsune Future"
        A modern, Linux-like 64-bit operating system
           Built with ♪ by Hatsune Miku fans
```

## 📝 Roadmap

- [ ] **Filesystem Drivers**: ext2, FAT32, ISO9660
- [ ] **Network Stack**: TCP/IP implementation
- [ ] **USB Support**: EHCI/xHCI drivers
- [ ] **Graphics**: Framebuffer & GPU drivers
- [ ] **User Space**: libc and basic utilities
- [ ] **Package Manager**: Miku package system
- [ ] **GUI**: Desktop environment

## 🤝 Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Inspired by Linux kernel design
- Thanks to the OSDev community
- Hatsune Miku for the inspiration ♪

---

**Made with ♪ and lots of caffeine**

*Thank you for using Miku OS!*
