# PIC32CZCA90 NuttX — Build and Compilation Guide

## Prerequisites

### 1. ARM Toolchain

```bash
sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi
```

Verify installation:

```bash
arm-none-eabi-gcc --version
# Expected: arm-none-eabi-gcc (15:13.2.rel1-2) 13.2.1 or similar
```

### 2. Kconfig Tools

```bash
pip3 install --break-system-packages kconfiglib
```

Verify installation:

```bash
kconfig-conf --help
# Should print usage info
```

### 3. Other Build Dependencies

```bash
sudo apt install build-essential gperf flex bison texinfo libncurses-dev
```

### 4. Filesystem Requirement

The project **must** be on a Linux-native filesystem (ext4, btrfs, etc.). NTFS/FAT partitions will cause `sed -i` permission errors during `configure.sh`.

---

## Build Steps

### Step 1: Initialize Submodules (first time only)

```bash
cd /media/nimish/Linux_Work/work/nuttx/pic32czca_nuttx
git submodule update --init --recursive
```

### Step 2: Clean Previous Build

```bash
cd /media/nimish/Linux_Work/work/nuttx/pic32czca_nuttx/nuttx
make distclean
```

### Step 3: Configure

**For minimal NSH shell (recommended for first boot):**

```bash
./tools/configure.sh pic32czca90-curiosity:nsh
```

**For full-featured config (SD card, USB, PROGMEM):**

```bash
./tools/configure.sh pic32czca90-curiosity:max
```

### Step 4: Build

```bash
make -j$(nproc)
```

### Step 5: Verify Output

On success, the following files are generated in the `nuttx/` directory:

| File | Description |
|------|-------------|
| `nuttx` | ELF binary (for debugging with GDB) |
| `nuttx.bin` | Raw binary (for flashing to hardware) |
| `nuttx.hex` | Intel HEX format |
| `System.map` | Symbol map |

```bash
ls -la nuttx nuttx.bin
arm-none-eabi-size nuttx
```

---

## Configuration Options

### Interactive Configuration (menuconfig)

After running `configure.sh`, you can modify the config interactively:

```bash
make menuconfig
```

Key menus:
- **System Type** → Chip selection, memory config
- **Board Selection** → Board-specific options
- **Application Configuration** → Enable/disable apps
- **Device Drivers** → Enable peripherals

### Save Configuration Changes

After modifying via `menuconfig`, save back to defconfig:

```bash
make savedefconfig
# This creates a minimal `defconfig` file
# Copy it to preserve your changes:
cp defconfig boards/arm/samv7/pic32czca90-curiosity/configs/nsh/defconfig
```

---

## Available Board Configurations

### `nsh` — Minimal NSH Shell

Minimal config for first boot and serial console testing.

- Serial console on UART1 (115200 baud)
- NSH shell with builtin commands
- FAT filesystem support
- GPIO IRQ for buttons
- DMA (XDMAC)
- I-Cache and D-Cache enabled
- **Flash:** 8 MB, **RAM:** 1 MB

### `max` — Full Featured

Everything in `nsh` plus:

- SD Card (HSMCI0) with automount
- USB CDC/ACM device
- PROGMEM (on-chip flash as storage)
- Button input driver
- D-Cache write-through mode

---

## Flashing

### Using OpenOCD + ST-Link

```bash
openocd -f boards/arm/samv7/common/tools/pic32cz-curiosity-stlink.cfg \
        -c "program nuttx.bin 0x00400000 verify reset exit"
```

### Using Microchip MPLAB IPE

1. Open MPLAB IPE
2. Select device: PIC32CZCA90
3. Connect debugger
4. Load `nuttx.hex`
5. Program and verify

---

## Serial Console

After flashing, connect to the serial console:

```bash
# Using minicom
minicom -D /dev/ttyUSB0 -b 115200

# Using screen
screen /dev/ttyUSB0 115200

# Using picocom
picocom -b 115200 /dev/ttyUSB0
```

**Expected output:**

```
NuttShell (NSH) NuttX-12.x.x
nsh>
```

**Basic test commands:**

```
nsh> help        # List available commands
nsh> uname -a    # Show system info
nsh> free        # Show memory usage
nsh> ps          # List running tasks
```

---

## Troubleshooting

### Error: `kconfig-conf: command not found`

```bash
pip3 install --break-system-packages kconfiglib
```

### Error: `sed: preserving permissions ... Operation not permitted`

Project is on an NTFS/FAT partition. Move to ext4:

```bash
cp -r /media/nimish/work/... ~/pic32czca_nuttx
```

### Error: `#error "Unknown SAMV7 chip type"`

A source file has a `#ifdef CONFIG_ARCH_CHIP_PIC32CZCA70` conditional that doesn't include CA90. Fix by adding `|| defined(CONFIG_ARCH_CHIP_PIC32CZCA90)`:

```c
// Find the line like:
#if defined(CONFIG_ARCH_CHIP_SAMV71) || defined(CONFIG_ARCH_CHIP_PIC32CZCA70)

// Change to:
#if defined(CONFIG_ARCH_CHIP_SAMV71) || defined(CONFIG_ARCH_CHIP_PIC32CZCA70) || defined(CONFIG_ARCH_CHIP_PIC32CZCA90)
```

### Error: `arm-none-eabi-gcc: command not found`

```bash
sudo apt install gcc-arm-none-eabi
```

### Build uses host `gcc` instead of `arm-none-eabi-gcc`

The `configure.sh` failed silently. Do a clean reconfigure:

```bash
make distclean
./tools/configure.sh pic32czca90-curiosity:nsh
make -j$(nproc)
```

### Rebuild After Code Changes

```bash
# Incremental rebuild (fast):
make -j$(nproc)

# Full clean rebuild:
make distclean
./tools/configure.sh pic32czca90-curiosity:nsh
make -j$(nproc)
```

---

## Project Structure Reference

```
pic32czca_nuttx/
├── nuttx/                          # NuttX kernel (git submodule)
│   ├── arch/arm/src/samv7/         # SAM V7 architecture (shared code)
│   ├── boards/arm/samv7/
│   │   ├── pic32czca90-curiosity/  # Our board port
│   │   ├── pic32czca70-curiosity/  # Reference board
│   │   └── common/                 # Shared linker scripts, tools
│   ├── tools/configure.sh          # Configuration tool
│   └── Makefile                    # Top-level build
└── apps/                           # NuttX apps (git submodule)
    └── (built automatically by make)
```
