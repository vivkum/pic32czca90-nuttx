# NuttX PIC32CZCA90 Porting Project

## Purpose
Porting NuttX RTOS to the Microchip PIC32CZCA90 microcontroller, using the existing PIC32CZCA70 port as a reference. The PIC32CZCA90 is pin-to-pin and binary compatible with the SAMV7/SAME70 family (Cortex-M7).

## Directory Structure

```
pic32czca_nuttx/
├── CLAUDE.md                  # This file
├── projectplan.md             # Development plan, status, and all changes
├── info.md                    # Brief project description
├── .gitmodules                # Submodule definitions
├── .github/                   # GitHub workflows (Gemini CI)
├── nuttx/                     # NuttX kernel (git submodule → apache/nuttx)
│   ├── arch/arm/
│   │   ├── include/samv7/
│   │   │   ├── chip.h         # MODIFIED — CA90 chip peripheral definitions
│   │   │   └── irq.h          # MODIFIED — CA90 IRQ include conditional
│   │   └── src/samv7/         # SAM V7 architecture source
│   │       ├── Kconfig         # MODIFIED — CA90 chip + memory entries
│   │       ├── sam_chipid.c    # MODIFIED — CA90 chip ID detection
│   │       ├── sam_eefc.c      # MODIFIED — CA90 EEFC conditional
│   │       ├── sam_hsmci.c     # MODIFIED — CA90 HSMCI conditional
│   │       ├── sam_periphclks.h# MODIFIED — CA90 peripheral clocks conditional
│   │       ├── sam_progmem.c   # MODIFIED — CA90 PROGMEM conditional
│   │       ├── hardware/
│   │       │   ├── sam_memorymap.h  # MODIFIED — CA90 memory map conditional
│   │       │   └── sam_pinmap.h     # MODIFIED — CA90 pin map conditional
│   │       └── ... (91 files total)
│   ├── boards/
│   │   ├── Kconfig             # MODIFIED — CA90 board selection entries
│   │   └── arm/samv7/
│   │       ├── common/         # Shared board support (linker scripts, tools)
│   │       ├── pic32czca70-curiosity/  # Reference port (unmodified)
│   │       ├── pic32czca90-curiosity/  # NEW — Target port
│   │       │   ├── Kconfig
│   │       │   ├── configs/nsh/defconfig
│   │       │   ├── configs/max/defconfig
│   │       │   ├── include/board.h
│   │       │   ├── scripts/Make.defs
│   │       │   ├── kernel/
│   │       │   └── src/ (sam_board.h, sam_boot.c, sam_bringup.c, etc.)
│   │       ├── same70-qmtech/
│   │       ├── same70-xplained/
│   │       └── samv71-xult/
│   ├── tools/                  # Build tools, configure.sh
│   └── Makefile
└── apps/                       # NuttX apps (git submodule → apache/nuttx-apps)
```

## Target Hardware: PIC32CZCA90

- **Architecture:** ARM Cortex-M7 (ARMv7-M with FPU, D-Cache, I-Cache)
- **Flash:** 8192 KB (0x800000)
- **RAM:** 1024 KB (0x100000)
- **Unique feature:** HSM (Hardware Security Module)
- **Package variants:** 064, 100, 144 pin
- **Compatibility:** Pin-to-pin and binary compatible with SAMV7/SAME70

## Reference Hardware: PIC32CZCA70

- **Flash:** 2048 KB (0x200000)
- **RAM:** 512 KB (0x80000)
- **CPU:** 300 MHz (PLLA), MCK 150 MHz
- **Crystal:** 12 MHz main, 32.768 kHz slow
- **Console:** UART1 (PA05 RX, PA06 TX)

## Build Commands

```bash
cd nuttx
make distclean
./tools/configure.sh pic32czca90-curiosity:nsh
make -j$(nproc)
```

## Porting Pattern

When the build hits `#error "Unknown SAMV7 chip type"` or similar errors from files that only check for CA70, the fix is always:

```c
// Before:
#if defined(CONFIG_ARCH_CHIP_SAMV71) || defined(CONFIG_ARCH_CHIP_PIC32CZCA70)

// After:
#if defined(CONFIG_ARCH_CHIP_SAMV71) || defined(CONFIG_ARCH_CHIP_PIC32CZCA70) || defined(CONFIG_ARCH_CHIP_PIC32CZCA90)
```

## Submodule Info

- **nuttx:** `https://github.com/apache/nuttx.git` @ `f4a6e62f7f72c47df8d0146a19b4060c48f508aa`
- **apps:** `https://github.com/apache/nuttx-apps.git` @ `45d4c7098bb3a7a6d9b5642efc47df5998c048d5`
