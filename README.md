# NuttX Port for PIC32CZCA90

NuttX RTOS port for the Microchip PIC32CZCA90 microcontroller (Cortex-M7), based on the existing PIC32CZCA70 Curiosity board support.

## Hardware

| Feature | Specification |
|---------|--------------|
| MCU | PIC32CZCA90 (ARM Cortex-M7) |
| Flash | 8192 KB |
| RAM | 1024 KB |
| Clock | 300 MHz CPU, 150 MHz MCK |
| Board | PIC32CZ CA90 Curiosity Ultra |
| HSM | Hardware Security Module (CA90 unique) |

## Repository Structure

```
├── nuttx/          # NuttX kernel source (based on Apache NuttX)
├── apps/           # NuttX applications (based on Apache NuttX Apps)
├── BUILD.md        # Compilation and flashing instructions
├── CHANGES.md      # Detailed log of all porting changes
├── CLAUDE.md       # Project reference documentation
└── projectplan.md  # Development plan and task tracking
```

## Quick Start

### Prerequisites

```bash
sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi build-essential gperf flex bison texinfo libncurses-dev
pip3 install --break-system-packages kconfiglib
```

### Build

```bash
cd nuttx
make distclean
./tools/configure.sh pic32czca90-curiosity:nsh
make -j$(nproc)
```

### Flash

```bash
openocd -f boards/arm/samv7/common/tools/pic32cz-curiosity-stlink.cfg \
        -c "program nuttx.bin 0x00400000 verify reset exit"
```

### Serial Console

Connect to UART1 at 115200 baud. Expected output:

```
NuttShell (NSH) NuttX-12.x.x
nsh>
```

## Board Configurations

| Config | Command | Description |
|--------|---------|-------------|
| `nsh` | `./tools/configure.sh pic32czca90-curiosity:nsh` | Minimal NSH shell |
| `max` | `./tools/configure.sh pic32czca90-curiosity:max` | Full featured (SD, USB, PROGMEM) |

## What Was Ported

- Kconfig chip definitions for PIC32CZCA90 (064/100/144 pin variants)
- Memory configuration: 8MB flash, 1MB RAM
- Board support package: `pic32czca90-curiosity`
- Architecture header patches (chip.h, irq.h, memory map, pin map, peripheral clocks, flash controller, SD/MMC, PROGMEM, chip ID)

See [CHANGES.md](CHANGES.md) for the complete list of modifications.

## Base NuttX Version

- **NuttX kernel:** Apache NuttX @ `f4a6e62f7f72c47df8d0146a19b4060c48f508aa`
- **NuttX apps:** Apache NuttX Apps @ `45d4c7098bb3a7a6d9b5642efc47df5998c048d5`

## License

NuttX is licensed under the Apache License 2.0. See [nuttx/LICENSE](nuttx/LICENSE).
