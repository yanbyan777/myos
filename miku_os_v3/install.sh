#!/bin/bash
# 🎵 Miku OS - Скрипт быстрой установки для Arch Linux
# Запуск: ./install.sh

set -e

echo "🎵 Miku OS Installer for Arch Linux"
echo "===================================="
echo ""

# Проверка что мы на Arch Linux
if [ ! -f /etc/arch-release ]; then
    echo "❌ Этот скрипт предназначен только для Arch Linux!"
    echo "   Для других дистрибутивов установите зависимости вручную."
    exit 1
fi

echo "✅ Обнаружен Arch Linux"
echo ""

# Установка зависимостей
echo "📦 Установка зависимостей..."
sudo pacman -S --needed --noconfirm \
    base-devel \
    git \
    nasm \
    qemu-base \
    grub \
    xorriso \
    gdb

echo ""
echo "✅ Все зависимости установлены!"
echo ""

# Проверка версии инструментов
echo "🔍 Версии установленных инструментов:"
gcc --version | head -n1
nasm -v | head -n1
qemu-system-x86_64 --version | head -n1
grub-install --version | head -n1
echo ""

# Создание директорий если не существуют
echo "📁 Проверка структуры проекта..."
mkdir -p arch/x86_64/boot
mkdir -p kernel
mkdir -p fs
mkdir -p mm
mkdir -p ipc
mkdir -p drivers/vga
mkdir -p drivers/keyboard
mkdir -p include
mkdir -p build

echo "✅ Структура проекта готова!"
echo ""

# Предложение запустить сборку
echo "🎉 Готово к сборке!"
echo ""
echo "Для сборки и запуска выполните:"
echo "  make"
echo ""
echo "Или по отдельности:"
echo "  make iso    - создать ISO образ"
echo "  make run    - запустить в QEMU"
echo "  make clean  - очистить сборку"
echo "  make debug  - сборка с отладкой"
echo ""
echo "🎵 Спасибо за использование Miku OS!"
