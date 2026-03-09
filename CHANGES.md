# PIC32CZCA90 NuttX Port — Change Log

All changes made to port NuttX RTOS to the PIC32CZCA90 microcontroller.

## Overview

- **Base reference:** PIC32CZCA70 Curiosity board port (existing in upstream NuttX)
- **Target:** PIC32CZCA90 Curiosity board (8MB flash, 1MB RAM, HSM, Cortex-M7)
- **Approach:** CA90 is pin-to-pin compatible with CA70/SAMV7 family; reuses the `samv7` architecture code with added chip/board configs

---

## 1. Modified Files in `nuttx/` Submodule (11 files)

### 1.1 `arch/arm/src/samv7/Kconfig` — Chip and Memory Configuration

**Added CA90 chip variants** (3 package options inside the chip selection choice block):

```kconfig
config ARCH_CHIP_PIC32CZCA90064
    bool "PIC32CZ CA90 064"
    select ARCH_CHIP_PIC32CZCA90
    select SAMV7_MEM_FLASH_8192
    select SAMV7_MEM_RAM_1024
    select SAMV7_QSPI_IS_SPI
    select SAMV7_HAVE_USBFS
    select SAMV7_HAVE_ISI8

config ARCH_CHIP_PIC32CZCA90100
    bool "PIC32CZ CA90 100"
    # ... (same peripherals as CA70 100-pin + 8MB flash/1MB RAM)

config ARCH_CHIP_PIC32CZCA90144
    bool "PIC32CZ CA90 144"
    # ... (same peripherals as CA70 144-pin + 8MB flash/1MB RAM)
```

**Added CA90 chip family config:**

```kconfig
config ARCH_CHIP_PIC32CZCA90
    bool
    default n
    select ARCH_HAVE_FPU
    select ARCH_HAVE_DPFPU
    select ARMV7M_HAVE_ICACHE
    select ARMV7M_HAVE_DCACHE
    select ARMV7M_HAVE_ITCM
    select ARMV7M_HAVE_DTCM
```

**Added new memory size options:**

```kconfig
config SAMV7_MEM_FLASH_8192       # NEW — 8MB flash for CA90
    bool
    default n

config SAMV7_MEM_RAM_1024         # NEW — 1MB RAM for CA90
    bool
    default n
```

**Updated memory hex defaults** to include 8192/1024 cases:

```kconfig
config ARCH_CHIP_SAMV7_MEM_FLASH
    hex
    default 0x80000  if SAMV7_MEM_FLASH_512
    default 0x100000 if SAMV7_MEM_FLASH_1024
    default 0x200000 if SAMV7_MEM_FLASH_2048
    default 0x800000 if SAMV7_MEM_FLASH_8192    # NEW

config ARCH_CHIP_SAMV7_MEM_RAM
    hex
    default 0x40000  if SAMV7_MEM_RAM_256
    default 0x60000  if SAMV7_MEM_RAM_384
    default 0x80000  if SAMV7_MEM_RAM_512
    default 0x100000 if SAMV7_MEM_RAM_1024      # NEW
```

**Updated PROGMEM sectors:**

```kconfig
config SAMV7_PROGMEM_NSECTORS
    default 64 if SAMV7_MEM_FLASH_8192          # NEW
    range 1 64                                   # was: range 1 16
```

### 1.2 `arch/arm/include/samv7/chip.h` — Chip Peripheral Definitions

Added full CA90 block (119 lines) with:
- `SAMV7_FLASH_SIZE (8192*1024)` and `SAMV7_SRAM_SIZE (1024*1024)`
- Three package variants (064, 100, 144) each defining peripheral counts (PIO, DMA, ADC, DAC, timers, UART, SPI, I2C, CAN, Ethernet, USB, etc.)
- Peripheral counts match corresponding CA70 package variants

### 1.3 `arch/arm/include/samv7/irq.h` — IRQ Definitions

```c
// Before:
#if defined(CONFIG_ARCH_CHIP_SAMV71) || defined(CONFIG_ARCH_CHIP_PIC32CZCA70)

// After:
#if defined(CONFIG_ARCH_CHIP_SAMV71) || defined(CONFIG_ARCH_CHIP_PIC32CZCA70) || defined(CONFIG_ARCH_CHIP_PIC32CZCA90)
```

Maps CA90 to use SAMV71 IRQ definitions (same interrupt controller).

### 1.4 `arch/arm/src/samv7/sam_periphclks.h` — Peripheral Clock Selection

```c
// Added || defined(CONFIG_ARCH_CHIP_PIC32CZCA90)
// Maps CA90 to use SAMV71 peripheral clock definitions
```

### 1.5 `arch/arm/src/samv7/hardware/sam_memorymap.h` — Memory Map Selection

```c
// Added || defined(CONFIG_ARCH_CHIP_PIC32CZCA90)
// Maps CA90 to use SAMV71 memory map
```

### 1.6 `arch/arm/src/samv7/hardware/sam_pinmap.h` — Pin Map Selection

```c
// Added || defined(CONFIG_ARCH_CHIP_PIC32CZCA90)
// Maps CA90 to use SAMV71 pin map
```

### 1.7 `arch/arm/src/samv7/sam_eefc.c` — Embedded Flash Controller

```c
// Added || defined(CONFIG_ARCH_CHIP_PIC32CZCA90)
// CA90 uses same 128KB sector layout as other SAMV7 chips
```

### 1.8 `arch/arm/src/samv7/sam_hsmci.c` — SD/MMC Host Controller

```c
// Added || defined(CONFIG_ARCH_CHIP_PIC32CZCA90)
// CA90 uses same AHB bus interface configuration
```

### 1.9 `arch/arm/src/samv7/sam_progmem.c` — Program Memory (PROGMEM)

```c
// Added || defined(CONFIG_ARCH_CHIP_PIC32CZCA90)
// CA90 uses same 128KB sector layout
```

### 1.10 `arch/arm/src/samv7/sam_chipid.c` — Chip ID Detection

```c
// Before:
#ifdef CONFIG_ARCH_CHIP_PIC32CZCA70

// After:
#if defined(CONFIG_ARCH_CHIP_PIC32CZCA70) || defined(CONFIG_ARCH_CHIP_PIC32CZCA90)
```

CA90 always returns `true` for `sam_has_revb_periphs()` (same as CA70).

### 1.11 `boards/Kconfig` — Board Selection

Added three entries:

**Board config block:**
```kconfig
config ARCH_BOARD_PIC32CZCA90_CURIOSITY
    bool "Microchip PIC32CZ CA90 Curiosity Ultra Evaluation Board"
    depends on ARCH_CHIP_PIC32CZCA90144
    select ARCH_HAVE_LEDS
    select ARCH_HAVE_BUTTONS
    select ARCH_HAVE_IRQBUTTONS
    select ARCH_PHY_INTERRUPT
```

**Board name string:** `default "pic32czca90-curiosity" if ARCH_BOARD_PIC32CZCA90_CURIOSITY`

**Board Kconfig source:** `source "boards/arm/samv7/pic32czca90-curiosity/Kconfig"`

---

## 2. New Board Directory (20 files)

**`nuttx/boards/arm/samv7/pic32czca90-curiosity/`**

Created by copying from `pic32czca70-curiosity/` and updating all references.

```
pic32czca90-curiosity/
├── .gitignore
├── Kconfig                          # Board-specific Kconfig (guard: PIC32CZCA90_CURIOSITY)
├── configs/
│   ├── nsh/defconfig                # Minimal NSH shell config
│   └── max/defconfig                # Full-featured config (SD, USB, PROGMEM)
├── include/
│   └── board.h                      # Clock, LED, button, UART definitions
├── kernel/
│   ├── Makefile
│   └── sam_userspace.c
├── scripts/
│   ├── Make.defs                    # Linker script generation rules
│   └── flash.ld                     # Generated linker script (build artifact)
└── src/
    ├── Make.defs                    # Board source build rules
    ├── sam_appinit.c                # Application init hooks
    ├── sam_autoleds.c               # Automatic LED driver
    ├── sam_board.h                  # GPIO pin definitions (LEDs, buttons, SD, USB)
    ├── sam_boot.c                   # Board boot initialization
    ├── sam_bringup.c                # Peripheral bringup
    ├── sam_buttons.c                # Button driver
    ├── sam_progmem_partition.c       # PROGMEM partition setup
    ├── sam_sdcard.c                 # SD card support
    ├── sam_usbdev.c                 # USB device support
    └── sam_userleds.c               # User LED control
```

**Key config differences from CA70 (in defconfig files):**

| Config Key | CA70 Value | CA90 Value |
|------------|-----------|-----------|
| `CONFIG_ARCH_BOARD` | `pic32czca70-curiosity` | `pic32czca90-curiosity` |
| `CONFIG_ARCH_BOARD_*_CURIOSITY` | `PIC32CZCA70_CURIOSITY` | `PIC32CZCA90_CURIOSITY` |
| `CONFIG_ARCH_CHIP_PIC32CZCA*144` | `PIC32CZCA70144` | `PIC32CZCA90144` |
| `CONFIG_ARCH_CHIP_PIC32CZCA*` | `PIC32CZCA70` | `PIC32CZCA90` |
| `CONFIG_ARCH_CHIP_SAMV7_MEM_FLASH` | `0x200000` (2MB) | `0x800000` (8MB) |
| `CONFIG_ARCH_CHIP_SAMV7_MEM_RAM` | `0x80000` (512KB) | `0x100000` (1MB) |
| `CONFIG_RAM_SIZE` | `524288` | `1048576` |

---

## 3. Top-Level Project Files

| File | Status | Description |
|------|--------|-------------|
| `.gitmodules` | NEW | Submodule URLs for `nuttx` and `apps` |
| `CLAUDE.md` | NEW | Project documentation and reference |
| `CHANGES.md` | NEW | This file |
| `BUILD.md` | NEW | Compilation procedure |
| `projectplan.md` | UPDATED | Development plan with completed status |

---

## Summary Statistics

- **Files modified in NuttX kernel:** 11
- **Lines added to kernel:** 212
- **Lines removed from kernel:** 12
- **New board files created:** 20
- **Total new + modified files:** 31
