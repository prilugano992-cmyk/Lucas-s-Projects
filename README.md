# Lucas's Projects 🚀

Welcome to my personal projects repository! Here you will find a collection of operating systems, terminal environments, command-line interface (CLI) tools, and low-level experiments built with C, Assembly, and PowerShell.

---

## 💻 Featured Project: GenesisOS

**GenesisOS** is a lightweight, 32-bit x86 graphical operating system built from scratch (bare-metal) natively on Windows. It combines modern desktop concepts, like modular movable docks and dynamic layout snapping, with nostalgia-inducing design aesthetics inspired by the early 2000s (Frutiger Aero / Windows XP Bliss).

### 📀 Quick Start: Booting the ISO
You don't need to compile anything to try it! You can download the pre-compiled, bootable ISO image and run it on any emulator (like VirtualBox or VMware), or run it directly on QEMU:

👉 **[Download GenesisOS Bootable ISO](https://github.com/prilugano992-cmyk/Lucas-s-Projects/releases/download/v1.0.0/genesis.iso)**

To run it via QEMU on Windows:
```bash
qemu-system-i386 -cdrom genesis.iso -drive file=disk.img,format=raw,index=0,media=disk
```

---

### 🛠️ Key Technical Features of GenesisOS:

*   **Custom ATA IDE Hard Disk Driver:** Reads and writes raw disk sectors over the primary master IDE channel in LBA28 PIO mode.
*   **GenesisOS File System (GFS):** A persistent custom file system implemented on Sector 1 of the virtual hard drive (`disk.img`), supporting dynamic sub-directory navigation, file renaming, creation, and deletion.
*   **Double Buffering:** Draws everything (background, windows, docks, mouse) into an invisible 3MB RAM buffer (`backbuffer`) and copies it to the actual screen at once, eliminating flickering.
*   **Bochs Graphics Adapter (BGA):** Direct port communication to set the video resolution to `1024x768x32bpp` (True Color) through ports `0x01CE`/`0x01CF`.
*   **PS/2 Physical Mouse Driver:** Custom data-cycle state machine tracking mouse movements with packets of 3 bytes, stabilized with byte synchronization checks.
*   **Interactive Desktop & Apps:** 
    *   **File Explorer:** Dynamic paths and folder navigation.
    *   **Notepad:** Interactive text editor with real-time keyboard mapping and saving back to the virtual hard drive.
    *   **Genesis Paint:** Scaled canvas (64x64) allowing physical mouse drawing and persistence.
    *   **Real-time CMOS Clock:** Independent floating clock widget reading hours, minutes, and seconds directly from the motherboard.

---

## 🖥️ Bang Desktop

A terminal-based visual desktop environment written in C using the **ncurses** library. It combines movable window panels, a simulated desktop grid with file icons, and native terminal text editing (vi/nano), serving as a proof-of-concept for hybrid text-graphics user interfaces.

### 🚀 How to Run
Make sure you have the `ncurses` development library installed on your system:

```bash
# Compile
gcc bang_desktop.c -o bang_desktop -lncurses

# Run
./bang_desktop
```

*   **Keyboard Controls:** 
    *   `TAB` - Switch focus between windows.
    *   `Arrow Keys` - Move the focused window or navigate desktop icons.
    *   `DEL` - Close the active window.
    *   `#` - Enter/Exit Desktop Mode.

---

## 🔍 WikiSearch

A terminal-based Wikipedia browser written in **PowerShell**. It interfaces with Wikipedia's official REST API to download and display summarized definitions of search queries instantly on the terminal. If an exact term isn't found, it automatically performs an opensearch to find the nearest match.

### 🚀 How to Run (Windows)
Open PowerShell inside the project directory and execute:

```powershell
# Run script
.\wikisearch.ps1
```

---

## 🧰 Languages and Technologies Used

*   **Languages:** C, x86 Assembly (NASM), PowerShell
*   **Libraries:** ncurses
*   **Emulators & Virtualization:** QEMU
*   **Compilers:** GCC cross-compiler (`i686-elf-gcc`)

---

## 📜 License

These projects are open-source and licensed under the **MIT License**. Feel free to use, modify, and distribute the code for study and educational purposes.

© 2026 Lucas Prado Coelho. Built with C, GNU toolchain, PowerShell and endless curiosity.
