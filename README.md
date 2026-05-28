# Lucas's Projects 🚀

Welcome to my personal projects repository! Here you will find a collection of operating systems, terminal environments, command-line interface (CLI) tools, and low-level experiments built with C, Assembly, and PowerShell.

---

## ℹ️ Sincere Disclosure & Author's Note

Hi there! My name is **Lucas**, and I am **12 years old**. 

I am deeply fascinated by low-level computer science, operating systems, and how hardware works. Because I am still learning the fundamentals of software engineering and do not have formal training, **I use AI (such as ChatGPT and DeepSeek) as my low-level programming tutor** and code-structuring assistant to help me bring my ideas to life.

While I am actively studying and learning how to write C, Assembly, and build systems, many of the complex hardware drivers and structures in these repositories were generated or organized with the help of AI under my direction. 

My goal is to learn and build cool things, not to deceive anyone. I hope you enjoy exploring my projects, and I welcome any constructive feedback, low-level advice, or mentoring!

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

### 🔓 How to Compile and Make your Own OS with GenesisOS:
The Source Code for Genesis is at the latest release, download the file "GenesisOSSource.zip" and Extract It.

When You're done Making Changes and want to Test it out, Execute "compile.ps1" and QEMU will open. (If you Have QEMU, Obviously.)

Now, to Generate the ISO, Execute the following Command inside the "GenesisOS Source" Folder:

```bash
mkisofs -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table -o genesis.iso iso
```

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

Check the Page for more Projects!

## 📜 License

These projects are open-source and licensed under the **MIT License**. Feel free to use, modify, and distribute the code for study and educational purposes.

© 2026 LucasPR. Built with C, GNU toolchain, PowerShell and endless curiosity.
