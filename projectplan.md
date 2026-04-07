# NuttX Porting Plan for PIC32CZCA90 MCU
# Target Board: PIC32CZ CA80/CA90 Curiosity Ultra (EV16W43A)

## Project Overview

Porting NuttX RTOS to the Microchip PIC32CZ8110CA90208 (208-pin) on the PIC32CZ CA80/CA90
Curiosity Ultra development board (EV16W43A), using the existing PIC32CZCA70 port as reference.
Both are Cortex-M7 based. The CA90 has a different memory map, peripheral base addresses, and
clock system compared to the CA70/SAMV71 family.

| Property        | PIC32CZCA70 (reference) | PIC32CZCA90 (target)    |
|-----------------|------------------------|-------------------------|
| Core            | Cortex-M7 (ARMv7-M)   | Cortex-M7 (ARMv7-M)    |
| Flash           | 2048 KB @ 0x00400000   | 8192 KB @ 0x0C000000    |
| RAM             | 512 KB @ 0x20400000    | 1024 KB @ 0x20020000    |
| Peripherals     | 0x40000000+            | 0x44000000+             |
| Clock system    | PMC (SAMV71-style)     | GCLK/MCLK/OSCCTRL       |
| Console UART    | UART1 (PA05/PA06)      | SERCOM1 (PC4 TX, PC7 RX)|
| Package         | 064/100/144 pin        | 064/100/144/208 pin      |
| HSM             | No                     | Yes                     |

---

## Board Hardware: PIC32CZ CA80/CA90 Curiosity Ultra (EV16W43A)

### On-board Peripherals

| # | Component | Detail |
|---|-----------|--------|
| 1 | Target MCU | PIC32CZ8110CA90208 (or CA80208), 300 MHz, 8M Flash, 1M SRAM |
| 2 | Debugger | PKoB4 (PICkit On-Board 4) — programming, debugging, Virtual COM |
| 3 | USB | USB0: Type-C Host/Device DRD; USB1: Micro A/B Host/Device DRD |
| 4 | Ethernet | KSZ9031MNXIC G-bit PHY (GMII/RMII/MII) |
| 5 | QSPI Flash | SST26VF032BAT-104I/SM (32 Mbit) |
| 6 | SD Card | SDIO interface with SDMMC controller |
| 7 | Serial EEPROM | 24LC256T-I/MS (256Kbit I2C) + AT24MAC402-MAHM-T (EUI-48) |
| 8 | CAN | Dual ATA6561-GBQW transceivers → CAN3 (J701), CAN4 (J702) |
| 9 | Audio | X32 connector: SSC (I2S0) + I2S1 |
| 10 | Graphics | GFX connector (EBI parallel, 16/24-bit LCD data) |
| 11 | Temperature | On-board sensor |
| 12 | Extension | EXT1, EXT2 (Xplained Pro 20-pin), mikroBUS socket, Arduino Uno R3 |

### Confirmed Board Pin Mappings (from DS70005522C schematic)

| Function | Port Pin | Notes |
|----------|----------|-------|
| **Console** | | |
| SERCOM1 TX (VCOM) | PC4 | PKoB4 Virtual COM → /dev/ttyACM0, 115200 baud |
| SERCOM1 RX (VCOM) | PC7 | |
| **LEDs** | | |
| User LED0 | PB21 | Digital output, active-low |
| User LED1 | PB22 | Digital output, active-low |
| **Buttons** | | |
| User SW0 | PB24 | Digital input, active-low |
| User SW1 | PC23 | Digital input, active-low |
| **mikroBUS** | | |
| UART TX | PC21 | SERCOM? |
| UART RX | PC22 | |
| SPI MOSI | PC12 | |
| SPI MISO | PC15 | |
| SPI SCK | PC13 | |
| SPI CS | PC14 | |
| I2C SDA | PC25 | |
| I2C SCL | PC26 | |
| AN | PA14 | ADC |
| RST | PB27 | |
| INT | PA8 | |
| PWM | PA22 | |
| **EXT1 Header** | | |
| RX2 | PC7 | Shares with SERCOM1 RX |
| TX2 | PC4 | Shares with SERCOM1 TX |
| SDA2 | PC0 | |
| SCL2 | PC1 | |
| SPI MOSI | PC8 | |
| SPI MISO | PC11 | |
| SPI SCK | PC9 | |
| SPI SS | PC10 | |
| ADC0(+) | PA18 | |
| ADC0(-) | PA17 | |
| **EXT2 Header** | | |
| RX2 | PC22 | |
| TX2 | PC21 | |
| SDA2 | PC25 | |
| SCL2 | PC26 | |
| SPI MOSI | PC12 | |
| SPI MISO | PC15 | |
| SPI SCK | PC13 | |
| SPI SS | PC14 | |
| **Arduino Uno R3** | | |
| D0/RX | PC22 | |
| D1/TX | PC21 | |
| D14/SDA | PC25 | |
| D15/SCL | PC26 | |
| A0–A5 | PA18, PA17, PA16, PA15, PA29, PA28 | ADC |
| **Audio X32** | | |
| I2S0 WS | PC0 | SSC |
| I2S0 DIN | PC2 | |
| I2S0 SCK | PC1 | |
| I2S0 DOUT | PC3 | |
| I2S0 MCLK | PC4 | |
| I2S1 WS | PA30 | |
| I2S1 SCK | PE1 | |
| I2S1 DIN | PE2 | |
| I2S1 DOUT | PE0 | |
| I2S1 MCLK | PA31 | |
| UART RX (audio) | PC9 | |
| UART TX (audio) | PC8 | |
| I2C SCL | PC13 | |
| I2C SDA | PC12 | |

---

## Memory Map (CA90 Actual)

| Region | Address | Size |
|--------|---------|------|
| Flash (FCR_PFM) | `0x0C000000` | 8 MB |
| FLEXRAM | `0x20020000` | 1 MB |
| MCLK | `0x44052000` | |
| GCLK | `0x44050000` | |
| OSCCTRL | `0x44040000` | |
| SERCOM0 | `0x46000000` | |
| SERCOM1 | `0x46002000` | |
| SERCOM2 | `0x46004000` | |
| PORT (GPIO) | `0x44840000` | |
| USB0 | `0x45000000` | |
| USB1 | `0x45002000` | |
| GMAC (Ethernet) | `0x45800000` | |
| MCAN0/1 | `0x4601C000 / 0x4601E000` | CAN |
| QSPI | `0x4B000000` | controller |
| QSPI memory | `0x08000000` | XIP window |
| SDMMC0 | `0x45900000` | SD card |
| HSM | `0x43000000` | |

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
- [x] Added `config ARCH_CHIP_PIC32CZCA90` family bool with FPU, DPFPU, ICACHE, DCACHE, ITCM, DTCM selects

### Phase 2: Kconfig — Board Registration (DONE)

**File: `nuttx/boards/Kconfig`**

- [x] Added `ARCH_BOARD_PIC32CZCA90_CURIOSITY` config entry
- [x] Added `default "pic32czca90-curiosity"` in `ARCH_BOARD` string choice
- [x] Added `source` conditional block

### Phase 3: Board Directory (DONE)

**Directory: `nuttx/boards/arm/samv7/pic32czca90-curiosity/`**

- [x] Copied entire `pic32czca70-curiosity/` directory as starting point
- [x] Updated `Kconfig`, `configs/nsh/defconfig`, `configs/max/defconfig`
- [x] Updated `include/board.h`, `src/sam_board.h`, `scripts/Make.defs`, `src/Make.defs`
- [x] Updated all `.c` files in `src/` and `kernel/`

### Phase 3.5: Architecture Header Fixes (DONE)

All architecture-level headers/source files that had `#ifdef CONFIG_ARCH_CHIP_PIC32CZCA70`
conditionals were updated to include CA90:

| File | What was added |
|------|---------------|
| `arch/arm/include/samv7/chip.h` | Full CA90 chip definition block |
| `arch/arm/include/samv7/irq.h` | CA90 added to SAMV71 IRQ include conditional |
| `arch/arm/src/samv7/sam_periphclks.h` | CA90 added to peripheral clocks include |
| `arch/arm/src/samv7/sam_eefc.c` | CA90 added to EEFC flash sector config |
| `arch/arm/src/samv7/sam_hsmci.c` | CA90 added to HSMCI bus interface |
| `arch/arm/src/samv7/hardware/sam_memorymap.h` | CA90 added to memory map include |
| `arch/arm/src/samv7/hardware/sam_pinmap.h` | CA90 added to pin map include |
| `arch/arm/src/samv7/sam_progmem.c` | CA90 added to flash sector config |
| `arch/arm/src/samv7/sam_chipid.c` | CA90 chip ID detection |

### Phase 4: Memory Map + Early Console (DONE — NOT YET FLASHED)

#### 4.1 Linker script addresses
**`nuttx/boards/arm/samv7/common/scripts/flash.ld.template`**
- `FLASH_START_ADDR = 0x0C000000`
- `SRAM_START_ADDR  = 0x20020000`

**defconfig (nsh + max):**
- `CONFIG_RAM_START=0x20020000`
- `CONFIG_INTELHEX_BINARY=y`

#### 4.2 Clock (sam_clockconfig.c) — no-op for CA90
CA90 boot ROM configures clocks (DFLL at 48 MHz on GCLK0) before jumping to user code.
PMC registers (0x400E06xx) do not exist on CA90. Added early `return` under `#ifdef CONFIG_ARCH_CHIP_PIC32CZCA90`.

#### 4.3 GPIO init (sam_gpio.c) — no-op for CA90
CA90 uses PORT peripheral (not PIO/MATRIX). Added early `return`.

#### 4.4 SERCOM1 early console (sam_lowputc.c)
- `ca90_sercom1_init()`: MCLK → SERCOM1 APB clock, GCLK0 → SERCOM1, PORT mux PC4/PC7 = D
- `ca90_sercom1_putc()`: polls DRE flag
- BAUD = 63019 (115200 @ 48 MHz DFLL, 16× oversampling)

#### 4.5 program.sh
- MDB script: `Device PIC32CZ8110CA90208 / Hwtool PKOB4 / Program / Reset / Quit`
- ELA flash record detection at `0x0C00` (not CA70's `0x0040`)

**Build result:**
```
flash: 102900 B / 8 MB (1.23%)
sram:    8768 B / 1 MB (0.84%)
Entry point: 0x0C000169
```

---

## Current Status

**Build state:** ✅ Fully building. Entry point `0x0C000169`. Binary ready to flash.

**Prerequisites installed:**
- [x] `arm-none-eabi-gcc` 13.2.1
- [x] `kconfiglib` (kconfig-conf)
- [x] MPLAB X v6.25 — MDB at `/opt/microchip/mplabx/v6.25/mplab_platform/bin/mdb.sh`

**Working directory:** `/home/vivek-kumar/vivi/PIC32CZCA90_NUTTX_1/pic32czca_nuttx/`

---

## Phase 5: Boot Verification — Flash and Console (NEXT)

**Goal:** Prove the chip boots and NuttX starts up with a working serial console.

### 5.1 Build and flash

```bash
cd nuttx
make distclean
./tools/configure.sh pic32czca90-curiosity:nsh
make -j$(nproc)
arm-none-eabi-objcopy -O ihex nuttx nuttx.hex
cd ..
./program.sh --flash-only
```

### 5.2 Verify serial console

```bash
screen /dev/ttyACM0 115200
```

Expected output:
```
NuttShell (NSH) NuttX-12.x.x
nsh> 
```

Run smoke tests:
```
nsh> help
nsh> ps
nsh> free
nsh> uname -a
```

### 5.3 Debug if no console output

GCLK0 may not be 48 MHz DFLL after reset. Try alternate BAUD values:

| GCLK0 freq | BAUD value |
|-----------|-----------|
| 12 MHz | 63626 |
| 24 MHz | 63282 |
| 48 MHz | 63019 ← current |
| 120 MHz | 62500 |

If chip does not boot at all: check linker script addresses, vector table at `0x0C000000`,
and stack pointer. Use PKoB4/SWD debug via MPLAB X to inspect PC and SP at reset.

**Success criteria:** `nsh>` prompt visible on `/dev/ttyACM0` at 115200 baud.

---

## Phase 6: GPIO — LEDs and Buttons

**Goal:** NuttX `ioctl` control of board LEDs; button press readable from NSH.

### 6.1 Correct pin assignments (from EV16W43A schematic, DS70005522C)

| Signal | Port Pin | Direction | Active |
|--------|----------|-----------|--------|
| User LED0 | **PB21** | Output | Low |
| User LED1 | **PB22** | Output | Low |
| User SW0 | **PB24** | Input | Low |
| User SW1 | **PC23** | Input | Low |

**Note:** Current `board.h` has wrong CA70 pins (PD29, PD23, PA01, PA09). Must update.

### 6.2 Files to update

- `nuttx/boards/arm/samv7/pic32czca90-curiosity/include/board.h`
  - `BOARD_NLEDS = 2`
  - `LED0_PIN = GPIO_OUTPUT | GPIO_PORTB | GPIO_PIN21`
  - `LED1_PIN = GPIO_OUTPUT | GPIO_PORTB | GPIO_PIN22`
  - `BUTTON_SW0 = GPIO_INPUT | GPIO_PORTB | GPIO_PIN24`
  - `BUTTON_SW1 = GPIO_INPUT | GPIO_PORTC | GPIO_PIN23`

- `nuttx/boards/arm/samv7/pic32czca90-curiosity/src/sam_bringup.c`
  - Register LED and button drivers if not already done

### 6.3 CA90 PORT peripheral

CA90 uses the PORT peripheral (not PIO). The existing `sam_gpio.c` no-ops for CA90.
A new `ca90_port.c` must implement:
- `ca90_configport(pin)` — direction, pull, mux, drive strength
- `ca90_portwrite(pin, val)`
- `ca90_portread(pin)`

PORT base: `0x44840000` (PORT group A). Groups: PA=+0x000, PB=+0x100, PC=+0x200, PD=+0x300, etc.

Registers per group (32-bit):
- `DIR` / `DIRCLR` / `DIRSET` / `DIRTGL` (+0x00/04/08/0C)
- `OUT` / `OUTCLR` / `OUTSET` / `OUTTGL` (+0x10/14/18/1C)
- `IN` (+0x20)
- `CTRL` (+0x24)
- `WRCONFIG` (+0x28)
- `PMUX[n]` (+0x30) — peripheral mux nibbles
- `PINCFG[n]` (+0x40) — PMUXEN, INEN, PULLEN, DRVSTR

**Success criteria:** `nsh> ls /dev/userleds`, toggle LEDs, button press detected.

---

## Phase 7: Full SERCOM USART Driver (Interrupt-Driven)

**Goal:** Replace polled early console with interrupt-driven NuttX serial driver.

### 7.1 Architecture

CA90 has 8 SERCOMs (SERCOM0–7). SERCOM1 is the console (Virtual COM via PKoB4).
Each SERCOM can operate as: USART, SPI master/slave, or I2C master/slave.

NuttX driver approach: new `ca90_sercom_usart.c` in `arch/arm/src/samv7/`.

### 7.2 SERCOM1 (console) configuration

| Parameter | Value |
|-----------|-------|
| Base address | `0x46002000` |
| TX pin | PC4, PMUX=D (SERCOM1_PAD0) |
| RX pin | PC7, PMUX=D (SERCOM1_PAD3) |
| GCLK channel | 24 (SERCOM1_CORE) |
| GCLK source | GCLK0 (48 MHz DFLL) |
| MCLK bit | MCLK_CLKMSK0 bit 24 |
| RXPO | 3 (PAD3) |
| TXPO | 0 (PAD0) |
| BAUD @ 48 MHz, 115200 | 63019 |
| IRQ | SERCOM1_0 through SERCOM1_4 |

### 7.3 defconfig changes

```
CONFIG_SAMV7_SERCOM1=y
CONFIG_SERCOM1_SERIAL_CONSOLE=y
CONFIG_SERCOM1_BAUD=115200
CONFIG_SERCOM1_BITS=8
CONFIG_SERCOM1_PARITY=0
CONFIG_SERCOM1_2STOP=0
# Remove:
# CONFIG_SAMV7_UART1=y
# CONFIG_UART1_SERIAL_CONSOLE=y
```

**Success criteria:** Full `nsh>` prompt via interrupt-driven SERCOM1. Command history, Ctrl-C work.

---

## Phase 8: SPI Driver (SERCOM-based)

**Goal:** SPI master driver for QSPI flash and expansion headers.

### 8.1 Board SPI instances

| SERCOM | Use | Pins |
|--------|-----|------|
| SERCOM2 | EXT1 SPI | MOSI=PC8, MISO=PC11, SCK=PC9, SS=PC10 |
| SERCOM3 | EXT2/mikroBUS SPI | MOSI=PC12, MISO=PC15, SCK=PC13, SS=PC14 |

### 8.2 Implementation

- `ca90_sercom_spi.c` — implements NuttX SPI interface (`struct spi_ops_s`)
- CTRLA: MODE=3 (SPI master, int clock), CPOL/CPHA config
- BAUD = (f_ref / (2 × f_spi)) − 1
- DMA support (optional, Phase 11+)
- Register via `spi_register()` for `/dev/spi0`, `/dev/spi1`

**Success criteria:** `nsh> dd if=/dev/spi0 bs=1 count=4` returns device ID bytes.

---

## Phase 9: I2C Driver (SERCOM-based)

**Goal:** I2C master driver for EEPROM, temperature sensor, audio codec control.

### 9.1 Board I2C instances

| SERCOM | Use | Pins | Speed |
|--------|-----|------|-------|
| SERCOM4 | EXT1 I2C (SDA2=PC0, SCL2=PC1) | PC0/PC1 | 400 kHz |
| SERCOM5 | mikroBUS + EXT2 I2C (SDA=PC25, SCL=PC26) | PC25/PC26 | 400 kHz |

### 9.2 On-board I2C devices

| Device | Address | Bus |
|--------|---------|-----|
| 24LC256T EEPROM | 0x50 | SERCOM5 (EPROM_I2C) |
| AT24MAC402 EUI-48 | 0x58 | SERCOM5 |
| Temperature sensor | varies | SERCOM5 |

### 9.3 Implementation

- `ca90_sercom_i2c.c` — implements NuttX I2C interface (`struct i2c_master_s`)
- CTRLA: MODE=5 (I2C master), SPEED=1 (FM 400 kHz)
- BAUD = (f_gclk / (2 × f_scl)) − 5
- Handle ARBLOST, BUSERR, NACK interrupts

**Success criteria:** `nsh> i2cget -b 0 0x50 0 1` reads EEPROM byte 0.

---

## Phase 10: USB Driver

**Goal:** USB CDC/ACM device providing a second serial port; USB host for mass storage.

### 10.1 USB instances on CA90 Curiosity Ultra

| Instance | Connector | Mode | CA90 Peripheral |
|----------|-----------|------|----------------|
| USB0 | Type-C (J2) | Host/Device DRD | USB0 @ `0x45000000` |
| USB1 | Micro A/B (J3) | Host/Device DRD | USB1 @ `0x45002000` |

The PKoB4 Virtual COM uses a separate USB path (PKoB USB = J6).

### 10.2 CA90 USB vs SAMV71 USB

CA90 uses a different USB IP from SAMV71 (which uses USBHS). CA90 likely uses:
- USB Full-Speed Device (similar to SAMD5x/SAME5x USB)
- GCLK-based (48 MHz from DFLL)
- Endpoint-based (not DMA descriptor ring like USBHS)

Research needed: Check CA90 datasheet USB register map vs existing `sam_usbdev.c`.
If register-compatible with SAMD5x, port the existing `samd5e5` USB driver.

### 10.3 defconfig changes (CDC/ACM)

```
CONFIG_USBDEV=y
CONFIG_CDCACM=y
CONFIG_CDCACM_CONSOLE=n
CONFIG_SAMV7_USB=y           # or CA90-specific symbol
```

**Success criteria:** `/dev/ttyACM1` appears when USB cable plugged into J2 or J3.

---

## Phase 11: Ethernet (GMAC + KSZ9031 PHY)

**Goal:** Network connectivity via the on-board G-bit Ethernet.

### 11.1 Hardware

| Component | Detail |
|-----------|--------|
| Controller | GMAC in CA90 @ `0x45800000` |
| PHY | KSZ9031MNXIC (G-bit PHY) |
| Interface | RMII (board design note: signals mux with GFX) |
| PHY address | Likely 0x01 (check schematic) |
| PHY IRQ | via GPIO |
| Crystal | DSC1001DI5-025.0000 (25 MHz for PHY) |

### 11.2 NuttX driver

The existing `sam_emac.c` (GMAC driver for SAMV71) may be register-compatible with CA90 GMAC.
Steps:
1. Verify CA90 GMAC register map vs SAMV71 GMAC — likely identical (both from Arm/Synopsys)
2. Add `#ifdef CONFIG_ARCH_CHIP_PIC32CZCA90` path to `sam_emac.c` with CA90 base address
3. Implement KSZ9031 PHY driver (or reuse existing if present in NuttX)
4. Configure RMII pins in PORT mux

### 11.3 defconfig changes

```
CONFIG_NET=y
CONFIG_NET_ETHERNET=y
CONFIG_SAMV7_EMAC0=y
CONFIG_SAMV7_EMAC0_PHYADDR=1
CONFIG_SAMV7_EMAC0_RMII=y
CONFIG_NET_IPv4=y
CONFIG_NET_TCP=y
CONFIG_NET_UDP=y
CONFIG_NETINIT_IPADDR=...
```

**Success criteria:** `nsh> ifconfig eth0`, `nsh> ping <host>` works.

---

## Phase 12: SD Card (SDMMC)

**Goal:** SD/SDHC card access via the on-board SD card slot.

### 12.1 Hardware

| Item | Detail |
|------|--------|
| Controller | SDMMC0 @ `0x45900000` |
| Interface | 4-bit SDIO |
| SD detect | GPIO (check schematic for pin) |
| Write protect | GPIO (check schematic for pin) |

### 12.2 NuttX driver

CA90 SDMMC is different from SAMV71 HSMCI. Need to check if NuttX has an SDMMC driver
compatible with SAME5x/CA90 (which share the same Microchip SDMMC IP).

Options:
- Port SAMD5x SDMMC driver if present in NuttX
- Implement new `ca90_sdmmc.c` using SDMMC register set

### 12.3 defconfig changes

```
CONFIG_MMCSD=y
CONFIG_MMCSD_SDIO=y
CONFIG_SAMV7_HSMCI=n          # disable CA70 driver
CONFIG_CA90_SDMMC0=y          # new config symbol
```

**Success criteria:** `nsh> mount -t vfat /dev/mmcsd0 /mnt`, SD card files visible.

---

## Phase 13: QSPI Flash (MTD Driver)

**Goal:** XIP-capable QSPI flash for extended storage.

### 13.1 Hardware

| Item | Detail |
|------|--------|
| Device | SST26VF032BAT-104I/SM (32 Mbit = 4 MB) |
| Controller | QSPI @ `0x4B000000` (control), XIP @ `0x08000000` |
| Interface | QSPI (4-bit) |

### 13.2 NuttX driver

- Add CA90 QSPI base address to hardware map
- Reuse or adapt `sam_qspi.c` from SAMV71 (check register compatibility)
- Register as MTD device → `/dev/mtdblock0`
- Optional: map as XIP for execute-in-place

### 13.3 defconfig changes

```
CONFIG_SAMV7_QSPI=y           # or CA90-specific
CONFIG_MTD=y
CONFIG_MTD_SST26=y
CONFIG_FS_SMARTFS=y           # or FAT
```

**Success criteria:** `nsh> mtd_test` passes; flash erase/write/read verified.

---

## Phase 14: CAN (MCAN)

**Goal:** CAN FD communication on CAN3 and CAN4 connectors.

### 14.1 Hardware

| Instance | Connector | Transceiver |
|----------|-----------|-------------|
| MCAN0 (CAN3) | J701 | ATA6561-GBQW |
| MCAN1 (CAN4) | J702 | ATA6561-GBQW |

### 14.2 NuttX driver

MCAN (ISO 11898-1:2015 CAN FD) is present in CA90. NuttX has `sam_mcan.c` for SAMV71.
Steps:
1. Verify CA90 MCAN base addresses: likely `0x4601C000` (MCAN0), `0x4601E000` (MCAN1)
2. Add CA90 addresses to `sam_mcan.c` under `#ifdef CONFIG_ARCH_CHIP_PIC32CZCA90`
3. Configure PORT pins for CAN TX/RX (check schematic for exact pins)
4. Configure GCLK routing to MCAN

### 14.3 defconfig changes

```
CONFIG_CAN=y
CONFIG_SAMV7_MCAN0=y
CONFIG_SAMV7_MCAN1=y
CONFIG_SAMV7_MCAN_LOOPBACK=n
```

**Success criteria:** `nsh> candump can0` receives frames from external CAN node.

---

## Phase 15: Audio (SSC / I2S)

**Goal:** Audio I/O via the X32 audio connector (SSC + I2S).

### 15.1 Hardware (X32 connector, 32-pin)

| Signal | Port | SERCOM/Peripheral |
|--------|------|-------------------|
| I2S0_WS (Word Select) | PC0 | I2S0 |
| I2S0_SCK | PC1 | I2S0 |
| I2S0_DIN (to MCU) | PC2 | I2S0 |
| I2S0_DOUT (from MCU) | PC3 | I2S0 |
| I2S0_MCLK | PC4 | I2S0 (shares with SERCOM1 TX!) |
| I2S1_WS | PA30 | I2S1 |
| I2S1_SCK | PE1 | I2S1 |
| I2S1_DIN | PE2 | I2S1 |
| I2S1_DOUT | PE0 | I2S1 |
| I2S1_MCLK | PA31 | I2S1 |
| Codec UART TX | PC8 | SERCOM? |
| Codec UART RX | PC9 | SERCOM? |
| Codec I2C SCL | PC13 | SERCOM? |
| Codec I2C SDA | PC12 | SERCOM? |

**Note:** I2S0_MCLK on PC4 conflicts with SERCOM1 TX (console). Audio and console
cannot be used simultaneously without jumper/mux changes on the board.

### 15.2 NuttX driver

CA90 has a dedicated I2S peripheral (not SSC like SAMV71). Need new `ca90_i2s.c`
implementing `struct i2s_dev_s`. Check if NuttX SAMD5x I2S driver is compatible.

**Success criteria:** Audio data streaming via I2S in loopback or with codec board.

---

## Phase 16: HSM (Hardware Security Module)

**Goal:** Expose CA90-specific HSM for cryptographic operations.

### 16.1 Overview

The HSM is unique to PIC32CZCA90 (not present in CA70, CA80, or SAMV71).
It provides:
- True Random Number Generator (TRNG)
- AES-128/256 (ECB, CBC, CTR, GCM)
- SHA-256/384/512
- RSA / ECC (P-256, P-384)
- Key management and secure storage
- Secure boot attestation

HSM base address: `0x43000000`
HSM communicates via a mailbox interface (command/response registers).

### 16.2 NuttX integration

- New driver: `ca90_hsm.c` — implements `/dev/crypto` or NuttX crypto interface
- TRNG: register as `/dev/random` source
- AES/SHA: implement via NuttX `crypto_register()`
- Key storage: HSM-managed key slots (not accessible from normal world)

### 16.3 defconfig changes

```
CONFIG_CA90_HSM=y
CONFIG_DEV_RANDOM=y
CONFIG_CRYPTO=y
CONFIG_CRYPTO_AES=y
CONFIG_CRYPTO_SHA2=y
```

**Success criteria:** `nsh> dd if=/dev/random bs=32 count=1 | hexdump` produces entropy.
AES encryption verified with known test vectors.

---

## Phase 17: Defconfig Profiles

### nsh (minimal) — target for Phase 5

```
CONFIG_ARCH_BOARD_PIC32CZCA90_CURIOSITY=y
CONFIG_ARCH_CHIP_PIC32CZCA90=y
CONFIG_ARCH_CHIP_PIC32CZCA90144=y
CONFIG_ARCH_CHIP_SAMV7_MEM_FLASH=0x800000
CONFIG_ARCH_CHIP_SAMV7_MEM_RAM=0x100000
CONFIG_RAM_START=0x20020000
CONFIG_RAM_SIZE=0x100000
CONFIG_SAMV7_SERCOM1=y
CONFIG_SERCOM1_SERIAL_CONSOLE=y
CONFIG_NSH_LIBRARY=y
```

### max (full peripherals) — target for Phase 16

Adds: SPI, I2C, USB, Ethernet, SD card, QSPI, CAN, Audio, HSM, all enabled.

---

## Peripheral Priority Roadmap

| Phase | Peripheral | Dependency | Estimated Complexity |
|-------|-----------|-----------|---------------------|
| 5 | Boot / Console | Phase 4 done | Low — already built |
| 6 | GPIO LEDs/Buttons | Phase 5 | Low — fix pin numbers |
| 7 | SERCOM USART (IRQ) | Phase 5 | Medium — new driver |
| 8 | SERCOM SPI | Phase 7 | Medium — adapt from USART |
| 9 | SERCOM I2C | Phase 7 | Medium — adapt |
| 10 | USB Device/Host | Phase 5 | High — new USB IP |
| 11 | Ethernet GMAC | Phase 5 | Medium — likely reuse sam_emac |
| 12 | SD Card SDMMC | Phase 9 | High — new IP vs HSMCI |
| 13 | QSPI Flash MTD | Phase 8 | Medium — check compat |
| 14 | CAN MCAN | Phase 7 | Medium — likely reuse sam_mcan |
| 15 | Audio I2S | Phase 9 | High — new I2S IP |
| 16 | HSM | Phase 5 | Very High — new driver, HSM mailbox |

---

## Summary of All Modified Files

### New files to create (upcoming):

```
nuttx/arch/arm/src/samv7/ca90_port.c           — CA90 PORT GPIO driver
nuttx/arch/arm/src/samv7/ca90_sercom_usart.c   — SERCOM interrupt UART
nuttx/arch/arm/src/samv7/ca90_sercom_spi.c     — SERCOM SPI master
nuttx/arch/arm/src/samv7/ca90_sercom_i2c.c     — SERCOM I2C master
nuttx/arch/arm/src/samv7/ca90_sdmmc.c          — SDMMC SD card
nuttx/arch/arm/src/samv7/ca90_i2s.c            — I2S audio
nuttx/arch/arm/src/samv7/ca90_hsm.c            — HSM crypto driver
```

### Files already created (Phases 1–4):

```
nuttx/boards/arm/samv7/pic32czca90-curiosity/  (entire directory — 19 files)
nuttx/arch/arm/src/samv7/Kconfig
nuttx/arch/arm/include/samv7/chip.h
nuttx/arch/arm/include/samv7/irq.h
nuttx/arch/arm/src/samv7/sam_periphclks.h
nuttx/arch/arm/src/samv7/sam_eefc.c
nuttx/arch/arm/src/samv7/sam_hsmci.c
nuttx/arch/arm/src/samv7/sam_progmem.c
nuttx/arch/arm/src/samv7/sam_chipid.c
nuttx/arch/arm/src/samv7/hardware/sam_memorymap.h
nuttx/arch/arm/src/samv7/hardware/sam_pinmap.h
nuttx/arch/arm/src/samv7/sam_clockconfig.c
nuttx/arch/arm/src/samv7/sam_gpio.c
nuttx/arch/arm/src/samv7/sam_lowputc.c
nuttx/boards/Kconfig
projectplan.md
CLAUDE.md
.gitmodules
program.sh
```
