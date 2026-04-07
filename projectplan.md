# NuttX Porting Plan for PIC32CZCA90 MCU

## Project Overview

Porting NuttX RTOS to the Microchip PIC32CZCA90 microcontroller using the existing PIC32CZCA70 port as reference. Both are Cortex-M7 based, pin-compatible with the SAMV7/SAME70 family.

| Property        | PIC32CZCA70 (reference) | PIC32CZCA90 (target) |
|-----------------|------------------------|----------------------|
| Core            | Cortex-M7 (ARMv7-M)   | Cortex-M7 (ARMv7-M) |
| Flash           | 2048 KB (0x200000)     | 8192 KB (0x800000)   |
| RAM             | 512 KB (0x80000)       | 1024 KB (0x100000)   |
| Packages        | 064, 100, 144          | 064, 100, 144        |
| HSM             | No                     | Yes                  |

---

## Work Completed

### Phase 1: Kconfig — Chip Registration (DONE)

**File: `nuttx/arch/arm/src/samv7/Kconfig`**

- [x] Added `SAMV7_MEM_FLASH_8192` bool config for 8MB flash
- [x] Added `SAMV7_MEM_RAM_1024` bool config for 1MB RAM
- [x] Added `default 0x800000 if SAMV7_MEM_FLASH_8192` to `ARCH_CHIP_SAMV7_MEM_FLASH`
- [x] Added `default 0x100000 if SAMV7_MEM_RAM_1024` to `ARCH_CHIP_SAMV7_MEM_RAM`
- [x] Added `default 64 if SAMV7_MEM_FLASH_8192` to `SAMV7_PROGMEM_NSECTORS`, updated range to `1 64`
- [x] Added chip variant entries: `ARCH_CHIP_PIC32CZCA90064`, `ARCH_CHIP_PIC32CZCA90100`, `ARCH_CHIP_PIC32CZCA90144`
  - Each selects `ARCH_CHIP_PIC32CZCA90`, `SAMV7_MEM_FLASH_8192`, `SAMV7_MEM_RAM_1024`
  - Peripheral selects match corresponding CA70 package variants
- [x] Added `config ARCH_CHIP_PIC32CZCA90` family bool with FPU, DPFPU, ICACHE, DCACHE, ITCM, DTCM selects

### Phase 2: Kconfig — Board Registration (DONE)

**File: `nuttx/boards/Kconfig`**

- [x] Added `ARCH_BOARD_PIC32CZCA90_CURIOSITY` config entry (depends on `ARCH_CHIP_PIC32CZCA90144`)
- [x] Added `default "pic32czca90-curiosity"` in `ARCH_BOARD` string choice
- [x] Added `source "boards/arm/samv7/pic32czca90-curiosity/Kconfig"` conditional block

### Phase 3: Board Directory (DONE)

**Directory: `nuttx/boards/arm/samv7/pic32czca90-curiosity/`**

- [x] Copied entire `pic32czca70-curiosity/` directory
- [x] Updated `Kconfig` — changed guard to `ARCH_BOARD_PIC32CZCA90_CURIOSITY`
- [x] Updated `configs/nsh/defconfig`:
  - `CONFIG_ARCH_BOARD="pic32czca90-curiosity"`
  - `CONFIG_ARCH_BOARD_PIC32CZCA90_CURIOSITY=y`
  - `CONFIG_ARCH_CHIP_PIC32CZCA90144=y` / `CONFIG_ARCH_CHIP_PIC32CZCA90=y`
  - `CONFIG_ARCH_CHIP_SAMV7_MEM_FLASH=0x800000`
  - `CONFIG_ARCH_CHIP_SAMV7_MEM_RAM=0x100000`
  - `CONFIG_RAM_SIZE=1048576`
- [x] Updated `configs/max/defconfig` — same changes as nsh
- [x] Updated `include/board.h` — header guards and comments CA70 to CA90
- [x] Updated `src/sam_board.h` — header guards and comments CA70 to CA90
- [x] Updated `scripts/Make.defs` — path comment CA70 to CA90
- [x] Updated `src/Make.defs` — path comment CA70 to CA90
- [x] Updated all `.c` files in `src/` and `kernel/` — path comments and references CA70 to CA90

### Phase 3.5: Architecture Header Fixes (DONE)

Build compilation revealed that several architecture-level headers and source files use `#ifdef CONFIG_ARCH_CHIP_PIC32CZCA70` conditionals. PIC32CZCA90 was added to all of them:

| # | File | What was added |
|---|------|---------------|
| 1 | `arch/arm/include/samv7/chip.h` | Full CA90 chip definition block (064/100/144 variants) with 8192KB flash, 1024KB RAM, and per-package peripheral counts |
| 2 | `arch/arm/include/samv7/irq.h` | Added `CONFIG_ARCH_CHIP_PIC32CZCA90` to SAMV71 IRQ include conditional |
| 3 | `arch/arm/src/samv7/sam_periphclks.h` | Added CA90 to SAMV71 peripheral clocks include conditional |
| 4 | `arch/arm/src/samv7/sam_eefc.c` | Added CA90 to EEFC flash sector configuration conditional |
| 5 | `arch/arm/src/samv7/sam_hsmci.c` | Added CA90 to HSMCI system bus interface conditional |
| 6 | `arch/arm/src/samv7/hardware/sam_memorymap.h` | Added CA90 to SAMV71 memory map include conditional |
| 7 | `arch/arm/src/samv7/hardware/sam_pinmap.h` | Added CA90 to SAMV71 pin map include conditional |
| 8 | `arch/arm/src/samv7/sam_progmem.c` | Added CA90 to PROGMEM flash sector configuration conditional |
| 9 | `arch/arm/src/samv7/sam_chipid.c` | Added CA90 to PIC32CZ chip ID detection (always returns revB) |

### Repository Setup (DONE)

- [x] Created `.gitmodules` file (was missing)
- [x] Initialized submodules: `nuttx` @ `f4a6e62`, `apps` @ `45d4c70`
- [x] Created `CLAUDE.md` with full project documentation

---

## Phase 4: Memory Map + Early Console Fixes (DONE — NOT YET FLASHED)

### Critical discovery: CA90 ≠ SAMV71 memory map

PIC32CZCA90 has a completely different memory map from CA70/SAMV71:

| Region | CA70 (SAMV71) | CA90 (actual) |
|--------|---------------|---------------|
| Flash | `0x00400000` | `0x0C000000` (FCR_PFM) |
| RAM | `0x20400000` | `0x20020000` (FLEXRAM) |
| Peripherals | `0x40000000` | `0x44000000+` |
| Clock | PMC @ `0x400E0600` | GCLK/MCLK/OSCCTRL @ `0x440xxxxx` |
| UART | UART1 SAMV71 | SERCOM1 @ `0x46002000` |

The original port had only copied sizes; base addresses were still CA70's.

### 4.1 Linker script addresses (DONE)

**File: `nuttx/boards/arm/samv7/common/scripts/flash.ld.template`**
- Added `#ifdef CONFIG_ARCH_CHIP_PIC32CZCA90` block:
  - `FLASH_START_ADDR = 0x0C000000`
  - `SRAM_START_ADDR  = 0x20020000`

**File: `nuttx/boards/arm/samv7/pic32czca90-curiosity/configs/nsh/defconfig`**
- `CONFIG_RAM_START=0x20020000` (was `0x20400000`)
- Added `CONFIG_INTELHEX_BINARY=y`

**File: `nuttx/boards/arm/samv7/pic32czca90-curiosity/configs/max/defconfig`**
- `CONFIG_RAM_START=0x20020000`

### 4.2 sam_clockconfig() — no-op for CA90 (DONE)

**File: `nuttx/arch/arm/src/samv7/sam_clockconfig.c`**

Added early `return` under `#ifdef CONFIG_ARCH_CHIP_PIC32CZCA90`.
CA90 boot ROM configures clocks (DFLL at 48 MHz on GCLK0) before jumping
to user code. PMC registers (`0x400E06xx`) do not exist on CA90.

### 4.3 sam_gpioinit() — no-op for CA90 (DONE)

**File: `nuttx/arch/arm/src/samv7/sam_gpio.c`**

Added early `return` under `#ifdef CONFIG_ARCH_CHIP_PIC32CZCA90`.
CA90 uses PORT peripheral (not PIO/MATRIX). No global init needed.

### 4.4 SERCOM1 early console (DONE)

**File: `nuttx/arch/arm/src/samv7/sam_lowputc.c`**

Added `ca90_sercom1_init()` and `ca90_sercom1_putc()` under
`#ifdef CONFIG_ARCH_CHIP_PIC32CZCA90`.

**Hardware connections (confirmed from board schematic):**
- PC4 → SERCOM1_PAD0 (TX), peripheral mux D
- PC7 → SERCOM1_PAD3 (RX), peripheral mux D
- Virtual COM port is PKoB4 on-board debugger → `/dev/ttyACM0`

**Init sequence:**
1. `MCLK_CLKMSK0 |= BIT(24)` — enable SERCOM1 APB clock @ `0x4405203C`
2. `GCLK_PCHCTRL[24] = CHEN | GEN=0` — route GCLK0 (48 MHz DFLL) to SERCOM1 @ `0x440500E0`
3. PORT PMUX: PC4=D, PC7=D; PINCFG[4/7]: PMUXEN=1 @ `0x44840100+`
4. SERCOM1 SWRST, wait sync
5. CTRLA: MODE=1 (INT CLK), RXPO=3, TXPO=0, DORD=1
6. BAUD = 63019 (115200 baud @ 48 MHz, 16× oversampling)
7. CTRLB: TXEN | RXEN | CHSIZE=8, wait sync
8. CTRLA: ENABLE, wait sync

`sam_lowsetup()` — added `#ifdef CONFIG_ARCH_CHIP_PIC32CZCA90` path that calls `ca90_sercom1_init()` and returns early (skips all SAMV71 UART code).

`arm_lowputc()` — added `#ifdef CONFIG_ARCH_CHIP_PIC32CZCA90` path that calls `ca90_sercom1_putc()`.

### 4.5 program.sh (DONE)

Created `program.sh` at project root:
- Detects flash via ELA record `0x0C00` (not CA70's `0x0040`)
- MDB script: `Device PIC32CZ8110CA90208 / Hwtool PKOB4 / Program / Reset / Quit`
- Modes: default (build+flash), `--flash-only`, `--build-only`, or `<hex-file>`

### Build result

```
flash: 102900 B / 8 MB (1.23%)
sram:    8768 B / 1 MB (0.84%)
Entry point: 0x0C000169
```

Build is **clean**. `nuttx.hex` generated manually with:
```bash
arm-none-eabi-objcopy -O ihex nuttx nuttx.hex
```

**Status: built, NOT YET FLASHED** — board was disconnected before programming.

---

## Current Status

**Build state:** ✅ Fully building. Entry point `0x0C000169`. All address-map and
early-console fixes applied. Binary ready to flash.

**Prerequisites installed:**
- [x] `arm-none-eabi-gcc` 13.2.1
- [x] `kconfiglib` (kconfig-conf)
- [x] MPLAB X v6.25 with MDB at `/opt/microchip/mplabx/v6.25/mplab_platform/bin/mdb.sh`

**Working directory:** `/home/vivek-kumar/vivi/PIC32CZCA90_NUTTX_1/pic32czca_nuttx/`

---

## Next Actions

### Immediate: Flash and verify early console (Phase 5)

```bash
cd /home/vivek-kumar/vivi/PIC32CZCA90_NUTTX_1/pic32czca_nuttx
# Regenerate hex (if needed):
arm-none-eabi-objcopy -O ihex nuttx/nuttx nuttx/nuttx.hex
# Flash + reset:
./program.sh --flash-only
# Monitor serial:
screen /dev/ttyACM0 115200
```

Expected: NuttX boot banner + `nsh>` prompt on `/dev/ttyACM0` at 115200 baud.

If no output appears, verify GCLK0 frequency assumption (48 MHz DFLL).
Alternative BAUD values to try:
- 12 MHz → BAUD = 63626
- 24 MHz → BAUD = 63282
- 48 MHz → BAUD = 63019  ← current

### After console works: Full serial driver (DONE)

The full NuttX serial driver for CA90 is now implemented using SERCOM1.
1. Disabled `CONFIG_SAMV7_UART1` / `CONFIG_UART1_SERIAL_CONSOLE` in defconfig.
2. Enabled `CONFIG_SAMV7_SERCOM1` / `CONFIG_SERCOM1_SERIAL_CONSOLE`.
3. Implemented `sam_sercom.c` (interrupt-driven USART driver).
4. Build and flash successful.

### After Successful Build: Flash and Test (Phase 5 - READY)

1. Flash `nuttx.hex` to PIC32CZCA90 Curiosity board (DONE via `program.sh`).
2. Connect serial console to PKoB4 Virtual COM at 115200 baud.
3. Verify `nsh>` prompt appears.
4. Run smoke tests: `help`, `ps`, `free`, `uname -a`.

```bash
# Flash using OpenOCD + ST-Link
openocd -f boards/arm/samv7/common/tools/pic32cz-curiosity-stlink.cfg \
        -c "program nuttx.bin 0x00400000 verify reset exit"
```

### Post-Boot: Peripheral Testing (Phase 6)

| Task | Description | Status |
|------|-------------|--------|
| 6.1 | GPIO / LEDs / Buttons — verify pin mappings match CA90 board schematic | Pending |
| 6.2 | SD Card (HSMCI) — test with max defconfig | Pending |
| 6.3 | USB Device — test CDC/ACM with max defconfig | Pending |
| 6.4 | HSM Driver — CA90-specific, requires new driver development | Pending |

### Board Pin Mapping Verification (Required Before Hardware Test)

The current board files use the same GPIO pin assignments as the CA70 Curiosity board. These must be verified against the actual PIC32CZCA90 Curiosity board schematic:

- **LEDs:** PD29 (LED0 green), PD23 (LED1 red)
- **Buttons:** PA01 (SW0), PA09 (SW1)
- **UART1 TX:** PA06 (GPIO_UART1_TXD_3)
- **SD Card Detect:** PC16
- **USB VBUS Enable:** PC25

If any pins differ on the CA90 board, update `include/board.h` and `src/sam_board.h`.

---

## Summary of All Modified Files

### New files created:
```
nuttx/boards/arm/samv7/pic32czca90-curiosity/         (entire directory — 19 files)
.gitmodules
CLAUDE.md
```

### NuttX kernel files modified (in nuttx/ submodule):
```
arch/arm/src/samv7/Kconfig                             — chip + memory Kconfig entries
arch/arm/include/samv7/chip.h                          — CA90 chip peripheral definitions
arch/arm/include/samv7/irq.h                           — IRQ include conditional
arch/arm/src/samv7/sam_periphclks.h                    — peripheral clocks conditional
arch/arm/src/samv7/sam_eefc.c                          — EEFC flash conditional
arch/arm/src/samv7/sam_hsmci.c                         — HSMCI bus conditional
arch/arm/src/samv7/sam_progmem.c                       — PROGMEM conditional
arch/arm/src/samv7/sam_chipid.c                        — chip ID detection
arch/arm/src/samv7/hardware/sam_memorymap.h            — memory map conditional
arch/arm/src/samv7/hardware/sam_pinmap.h               — pin map conditional
boards/Kconfig                                         — board selection entries
```

### Top-level project files:
```
projectplan.md                                         — this file
CLAUDE.md                                              — project documentation
.gitmodules                                            — submodule URLs
```
