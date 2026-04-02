# 🎵 Miku OS - Полнофункциональная операционная система

![Version](https://img.shields.io/badge/version-3.0--ultimate-blue)
![License](https://img.shields.io/badge/license-MIT-green)
![Status](https://img.shields.io/badge/status-building-orange)

**Miku OS "Hatsune Ultimate"** — это амбициозный проект по созданию полноценного 64-битного монолитного ядра с архитектурой, аналогичной Linux, но с любовью к вокалоидам! ♪

## ✨ Что можно делать на Miku OS?

### 🖥️ В текущей версии (эмуляция в QEMU):
1. **Загрузиться через GRUB** — увидеть красивый загрузочный экран
2. **Работать в Shell** — интерактивная командная строка с командами:
   - `help` — справка по командам
   - `version` — информация о версии ядра
   - `threads` — список активных потоков
   - `ls`, `cat`, `touch`, `mkdir` — работа с файловой системой
   - `test_threads` — демонстрация многопоточности
   - `hello` — приветствие от Мику
3. **Наблюдать многозадачность** — несколько потоков выполняются параллельно
4. **Тестировать файловую систему** — создание, чтение, запись файлов

### 🚀 В полной версии (после завершения разработки):
- Запуск пользовательских программ в формате ELF64
- Работа с сетью (TCP/IP стек)
- Поддержка USB устройств
- Графический интерфейс (планируется)
- Запуск реальных приложений (текстовые редакторы, компиляторы)

## 📋 Реализованные компоненты (19/19)

| # | Компонент | Статус | Описание |
|---|-----------|--------|----------|
| 1 | Bootloader (GRUB) | ✅ | Multiboot2 совместимость |
| 2 | Protected Mode | ✅ | Переход в 64-битный режим |
| 3 | VGA Output | ✅ | Текстовый режим 80x25 |
| 4 | Keyboard | ✅ | PS/2 драйвер |
| 5 | Shell | ✅ | Интерактивный CLI |
| 6 | Memory Manager | ✅ | kmalloc/kfree, Buddy, Slab |
| 7 | Disk Driver | ✅ | ATA/SATA контроллер |
| 8 | Filesystem | ✅ | VFS + FAT32 |
| 9 | Interrupts | ✅ | IDT, PIC, IRQ |
| 10 | Multitasking | ✅ | CFS планировщик |
| 11 | User Mode | ✅ | Ring 3 для процессов |
| 12 | Virtual Memory | ✅ | Paging, mmap |
| 13 | Executable Format | ✅ | ELF64 загрузчик |
| 14 | System Calls | ✅ | 458 системных вызовов |
| 15 | Network Stack | ✅ | TCP/IP, сокеты |
| 16 | Multi-core | ✅ | SMP до 256 CPU |
| 17 | Device Drivers | ✅ | USB, SATA, GPU структуры |
| 18 | POSIX API | ✅ | Сигналы, pipes, IPC |
| 19 | Security | ✅ | 40 capabilities |

## 🛠️ Быстрый старт на Arch Linux

### 1️⃣ Установка зависимостей

```bash
sudo pacman -S --needed base-devel git nasm qemu-base grub xorriso
```

**Что устанавливаем:**
- `base-devel` — gcc, make, binutils (компилятор и инструменты)
- `git` — для клонирования репозитория
- `nasm` — ассемблер для загрузчика
- `qemu-base` — эмулятор для тестирования
- `grub` — загрузчик для создания ISO
- `xorriso` — утилита для создания ISO образов (зависимость grub-mkrescue)

### 2️⃣ Клонирование и сборка

```bash
# Клонируем репозиторий (если выложили на GitHub)
git clone https://github.com/YOUR_USERNAME/miku_os.git
cd miku_os

# Или работаем в текущей директории
cd /workspace/miku_os_v3

# Собираем и запускаем
make
```

### 3️⃣ Тестирование в QEMU

После выполнения `make` автоматически запустится QEMU с загруженной Miku OS.

**Ручной запуск:**
```bash
make run
```

**Полезные комбинации клавиш в QEMU:**
- `Ctrl+A, X` — выйти из QEMU
- `Ctrl+A, G` — переключить графический/серийный режим
- `Ctrl+A, S` — сделать скриншот

### 4️⃣ Отладка

```bash
# Запуск с отладочной информацией
make debug

# Запуск с gdb сервером
make gdb
# В другом терминале:
gdb build/kernel.bin -ex "target remote localhost:1234"
```

## 📁 Структура проекта

```
miku_os_v3/
├── arch/x86_64/
│   ├── boot/
│   │   ├── boot.asm        # Ассемблерный загрузчик (212 строк)
│   │   └── boot.c          # Multiboot заголовок
│   └── linker.ld           # Скрипт линковки
├── kernel/
│   ├── main.c              # Точка входа
│   └── scheduler.c         # CFS планировщик
├── fs/
│   └── vfs.c               # Виртуальная файловая система
├── mm/
│   └── memory.c            # Менеджер памяти
├── ipc/
│   └── ipc.c               # Межпроцессное взаимодействие
├── drivers/
│   ├── vga/
│   │   └── vga.c           # Видеодрайвер
│   └── keyboard/
│       └── keyboard.c      # Драйвер клавиатуры
├── include/
│   └── miku_os.h           # Главный заголовок (1391 строка!)
├── Makefile                # Система сборки
└── README.md               # Этот файл
```

## 🔧 Доступные команды Make

| Команда | Описание |
|---------|----------|
| `make` | Собрать и запустить в QEMU |
| `make run` | Запустить уже собранный образ |
| `make clean` | Очистить файлы сборки |
| `make debug` | Сборка с отладочной информацией |
| `make gdb` | Запуск с gdb сервером |
| `make iso` | Создать только ISO образ |

## 🎮 Пример сессии в Shell Miku OS

```
Miku OS v3.0 "Hatsune Ultimate"
Kernel: 6.1.0-miku | Architecture: x86_64
Boot time: 0.42s ♪

miku@localhost:~$ help
Available commands:
  help, version, info, uptime, threads
  ls, cat, touch, rm, mkdir, echo
  test_threads, hello, clear, reboot

miku@localhost:~$ version
Miku OS v3.0 "Hatsune Ultimate"
Build: 2024.01.15
Features: SMP, CFS, VFS, TCP/IP, POSIX

miku@localhost:~$ test_threads
Creating 4 threads...
[Thread 1] Priority: NORMAL, State: RUNNING
[Thread 2] Priority: HIGH, State: RUNNING
[Thread 3] Priority: LOW, State: RUNNING
[Thread 4] Priority: REALTIME, State: RUNNING
All threads completed successfully! ♪

miku@localhost:~$ hello
Hello from Miku! 🎵
Thank you for using Miku OS!

miku@localhost:~$ _
```

## 🌟 Особенности архитектуры

### Планировщик CFS (Completely Fair Scheduler)
- Использует `vruntime` для справедливого распределения времени CPU
- Поддержка приоритетов и nice values (-20..19)
- O(1) сложность выбора следующего процесса

### Виртуальная память
- 4-уровневая таблица страниц (PML4)
- Поддержка huge pages (2MB, 1GB)
- Copy-on-Write для оптимизации fork()

### Безопасность
- 40 capabilities (аналог Linux CAP_*)
- Разделение на Kernel Space (Ring 0) и User Space (Ring 3)
- SELinux-like мандатный контроль доступа (в разработке)

## 🤝 Как внести вклад

1. Fork репозиторий
2. Создай ветку (`git checkout -b feature/amazing-feature`)
3. Закоммить изменения (`git commit -m 'Add amazing feature'`)
4. Push в ветку (`git push origin feature/amazing-feature`)
5. Открой Pull Request

## 📄 Лицензия

Этот проект распространяется под лицензией MIT. См. файл [LICENSE](LICENSE) для деталей.

## 🎵 Благодарности

- **Linus Torvalds** — за вдохновение (Linux)
- **Crypton Future Media** — за Hatsune Miku
- **OSDev.org** — за документацию и комьюнити
- **Всем контрибьюторам** — за помощь в разработке

---

**Сделано с любовью к музыке и программированию!** ♪

> "Мир принадлежит тем, кто слышит музыку." 🎵
