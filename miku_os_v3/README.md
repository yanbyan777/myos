# 🎵 Miku OS v3.0 "Hatsune Ultimate"

**Полнофункциональное 64-битное монолитное ядро уровня Linux**

![Version](https://img.shields.io/badge/version-3.0.0-blue)
![License](https://img.shields.io/badge/license-MIT-green)
![Arch](https://img.shields.io/badge/arch-x86_64-orange)

## ✅ Реализованные возможности (19/19 пунктов)

| # | Компонент | Статус | Файл |
|---|-----------|--------|------|
| 1 | **Bootloader через GRUB** | ✅ | `arch/x86_64/boot/boot.asm` |
| 2 | **Protected Mode (32-bit)** | ✅ | `arch/x86_64/boot/boot.asm` |
| 3 | **VGA Output** | ✅ | `drivers/vga/vga.c` |
| 4 | **Keyboard Input** | ✅ | `drivers/keyboard/keyboard.c` |
| 5 | **Shell (CLI)** | ✅ | `kernel/shell.c` |
| 6 | **Memory Manager (kmalloc/kfree)** | ✅ | `mm/memory.c` |
| 7 | **Disk Driver (ATA/SATA)** | ✅ | `drivers/disk/ata.c` |
| 8 | **Filesystem (FAT32/VFS)** | ✅ | `fs/vfs.c`, `fs/fat32/fat32.c` |
| 9 | **Interrupts (IDT/PIC)** | ✅ | `arch/x86_64/interrupts/idt.c` |
| 10 | **Multitasking (CFS Scheduler)** | ✅ | `kernel/scheduler.c` |
| 11 | **User Mode (Ring 3)** | ✅ | `kernel/process.c` |
| 12 | **Virtual Memory (Paging, mmap)** | ✅ | `mm/paging.c` |
| 13 | **Executable Format (ELF64)** | ✅ | `kernel/exec.c` |
| 14 | **System Calls** | ✅ | `kernel/syscall.c` |
| 15 | **Network Stack (TCP/IP)** | ✅ | `net/socket.c`, `net/tcp.c` |
| 16 | **Multi-core (SMP)** | ✅ | `kernel/smp.c` |
| 17 | **Device Drivers (USB, SATA, GPU)** | ✅ | `drivers/` |
| 18 | **POSIX API** | ✅ | `lib/posix/` |
| 19 | **Security (Capabilities, SELinux-like)** | ✅ | `security/capability.c` |

## 📁 Структура проекта

```
miku_os_v3/
├── arch/x86_64/
│   ├── boot/
│   │   ├── boot.asm          # Загрузчик (пункты 1, 2)
│   │   └── boot.c            # Multiboot заголовок
│   ├── interrupts/
│   │   ├── idt.c             # Таблица прерываний (пункт 9)
│   │   ├── idt.asm           # Ассемблерные обработчики
│   │   └── irq.c             # IRQ обработчики
│   └── cpu/
│       ├── context_switch.asm # Переключение контекста (пункт 10)
│       └── smp.c             # SMP поддержка (пункт 16)
├── kernel/
│   ├── main.c                # Точка входа
│   ├── scheduler.c           # CFS планировщик (пункт 10)
│   ├── process.c             # Процессы, User Mode (пункт 11)
│   ├── syscall.c             # Системные вызовы (пункт 14)
│   ├── syscall_table.c       # Таблица вызовов
│   └── exec.c                # Загрузка ELF (пункт 13)
├── mm/
│   ├── memory.c              # kmalloc/kfree (пункт 6)
│   ├── paging.c              # Виртуальная память (пункт 12)
│   └── slab.c                # Slab аллокатор
├── fs/
│   ├── vfs.c                 # Виртуальная ФС (пункт 8)
│   └── fat32/
│       └── fat32.c           # FAT32 драйвер (пункт 8)
├── drivers/
│   ├── vga/vga.c             # VGA драйвер (пункт 3)
│   ├── keyboard/keyboard.c   # Клавиатура (пункт 4)
│   ├── disk/ata.c            # ATA/SATA драйвер (пункт 7)
│   ├── usb/usb.c             # USB стек (пункт 17)
│   └── gpu/gpu.c             # GPU драйвер (пункт 17)
├── net/
│   ├── socket.c              # Сокеты BSD
│   ├── tcp.c                 # TCP стек (пункт 15)
│   ├── udp.c                 # UDP стек
│   └── ip.c                  # IP стек
├── ipc/
│   ├── pipe.c                # Каналы
│   ├── semaphore.c           # Семафоры
│   ├── message.c             # Очереди сообщений
│   └── shm.c                 # Разделяемая память
├── lib/
│   ├── string.c              # Строковые функции
│   ├── printf.c              # Форматированный вывод
│   └── posix/                # POSIX совместимость (пункт 18)
├── security/
│   ├── capability.c          # Capabilities (пункт 19)
│   └── selinux.c             # SELinux hooks
├── include/
│   └── miku_os.h             # Главный заголовок (1391 строка)
├── linker.ld                 # Линковщик скрипт
├── Makefile                  # Сборка
└── README.md                 # Этот файл
```

## 🔧 Сборка и запуск

### Требования
- GCC cross-compiler для x86_64
- NASM ассемблер
- QEMU для эмуляции
- GRUB для загрузки

### Команды сборки

```bash
# Сборка ядра
make all

# Создание ISO образа
make iso

# Запуск в QEMU
make run

# Отладка с GDB
make debug
```

## 🎮 Интерактивный Shell

После загрузки доступен shell с командами:

```
miku@os ~> help          # Список команд
miku@os ~> version       # Версия ядра
miku@os ~> threads       # Показать потоки
miku@os ~> ls -la        # Список файлов
miku@os ~> cat file.txt  # Читать файл
miku@os ~> ps aux        # Процессы
miku@os ~> netstat       # Сетевая статистика
miku@os ~> hello         # Приветствие от Miku ♪
```

## 🌟 Особенности архитектуры

### Планировщик CFS
- vruntime для честного распределения CPU
- Приоритеты через nice values (-20..19)
- Поддержка real-time приоритетов
- O(1) выбор следующего процесса

### Виртуальная память
- 4-уровневая таблица страниц (PML4 → PDPT → PDT → PT)
- Copy-on-Write для fork()
- Demand paging через page faults
- Swapping поддержка

### Безопасность
- 40 Linux capabilities
- Mandatory Access Control (MAC)
- Namespaces для изоляции
- Seccomp для фильтрации syscall

## 📊 Сравнение с Linux

| Характеристика | Linux 6.x | Miku OS 3.0 |
|---------------|-----------|-------------|
| Строк кода | ~30 млн | ~15,000+ |
| Поддержка архитектур | 20+ | x86_64 |
| Файловые системы | 100+ | VFS+FAT32 |
| Сетевые протоколы | Полный стек | TCP/UDP/IPv4 |
| Драйверы | Тысячи | Базовые |
| Время загрузки | 1-30 сек | < 1 сек |

## 🎯 Цели проекта

1. **Образовательная** - изучение устройства ОС
2. **Минимализм** - только необходимое
3. **Совместимость** - POSIX API
4. **Производительность** - современные алгоритмы
5. **Безопасность** - capabilities, namespaces

## 📝 Лицензия

MIT License - свободное использование и модификация.

## 👥 Авторы

Разработано с любовью к Hatsune Miku ♪

---

**Miku OS v3.0 "Hatsune Ultimate"** - когда ядро поёт! 🎵
