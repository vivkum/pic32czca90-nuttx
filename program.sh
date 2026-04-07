#!/bin/bash
# program.sh — Build NuttX and flash to PIC32CZ8110CA90208 via PKoB4/MDB
#
# Usage:
#   ./program.sh                        # Build then flash nuttx/nuttx.hex
#   ./program.sh --flash-only           # Flash existing nuttx/nuttx.hex (no build)
#   ./program.sh --build-only           # Build only, do not flash
#   ./program.sh <path/to/firmware.hex> # Flash a specific HEX file
#
# Requirements:
#   MPLAB X IDE v6.25 installed at /opt/microchip/mplabx/v6.25
#   arm-none-eabi-gcc toolchain on PATH
#   Board connected via USB (PKoB4 onboard debugger)
#   kconfiglib installed (for NuttX menuconfig/configure)

set -e

# ── Config ────────────────────────────────────────────────────────────────────
DEVICE="PIC32CZ8110CA90208"
MDB="/opt/microchip/mplabx/v6.25/mplab_platform/bin/mdb.sh"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
NUTTX_DIR="$SCRIPT_DIR/nuttx"
DEFAULT_HEX="$NUTTX_DIR/nuttx.hex"
BOARD_CONFIG="pic32czca90-curiosity:nsh"
TMP_SCRIPT="/tmp/mdb_program_$$.mdb"

# ── Argument parsing ──────────────────────────────────────────────────────────
BUILD=true
FLASH=true
HEX_FILE=""

for arg in "$@"; do
    case "$arg" in
        --flash-only) BUILD=false ;;
        --build-only) FLASH=false ;;
        --help|-h)
            echo "Usage: $0 [--flash-only | --build-only | <path/to/firmware.hex>]"
            exit 0
            ;;
        -*)
            echo "Unknown option: $arg"
            exit 1
            ;;
        *)
            HEX_FILE="$(realpath "$arg")"
            BUILD=false   # explicit hex → skip build
            ;;
    esac
done

# ── Build ─────────────────────────────────────────────────────────────────────
if [ "$BUILD" = true ]; then
    echo "=== Building NuttX for $DEVICE ==="
    echo "Board config: $BOARD_CONFIG"
    echo ""

    cd "$NUTTX_DIR"

    # Only reconfigure if not already configured for this board
    CURRENT_BOARD=""
    if [ -f .config ]; then
        CURRENT_BOARD=$(grep -s 'CONFIG_ARCH_BOARD=' .config | cut -d'"' -f2 || true)
    fi

    if [ "$CURRENT_BOARD" != "pic32czca90-curiosity" ]; then
        echo "Configuring..."
        make distclean -s 2>/dev/null || true
        ./tools/configure.sh "$BOARD_CONFIG"
    else
        echo "Already configured for pic32czca90-curiosity, skipping distclean."
    fi

    echo "Compiling ($(nproc) jobs)..."
    make -j"$(nproc)"

    echo ""
    echo "Build complete:"
    ls -lh "$NUTTX_DIR/nuttx.hex" "$NUTTX_DIR/nuttx.bin" 2>/dev/null || true
    echo ""

    cd "$SCRIPT_DIR"
fi

# ── Flash ─────────────────────────────────────────────────────────────────────
if [ "$FLASH" = false ]; then
    echo "Build-only mode — skipping flash."
    exit 0
fi

# Resolve HEX file
if [ -z "$HEX_FILE" ]; then
    HEX_FILE="$DEFAULT_HEX"
fi

if [ ! -f "$HEX_FILE" ]; then
    echo "Error: HEX file not found: $HEX_FILE"
    echo "       Run without --flash-only to build first."
    exit 1
fi

if [ ! -x "$MDB" ]; then
    echo "Error: mdb.sh not found at $MDB"
    echo "       Make sure MPLAB X v6.25 is installed."
    exit 1
fi

# ── Check USB connection ───────────────────────────────────────────────────────
if ! lsusb 2>/dev/null | grep -q "04d8"; then
    echo "Warning: No Microchip USB device detected. Is the board connected?"
fi

# ── Detect target address ──────────────────────────────────────────────────────
# PIC32CZCA90 flash starts at 0x00400000.
# Intel HEX Extended Linear Address record for 0x0040xxxx is ':020000040040BA'
# (upper 16-bit segment = 0x0040).
HAS_FLASH=$(grep -c "^:020000040C00" "$HEX_FILE" || true)

if [ "$HAS_FLASH" -gt 0 ]; then
    echo "=== Flashing $DEVICE via MDB/PKoB4 ==="
    echo "Device:   $DEVICE"
    echo "HEX file: $HEX_FILE"
    echo "Address:  0x0C000000 (FCR_PFM primary flash)"
    echo ""

    cat > "$TMP_SCRIPT" << EOF
Device $DEVICE
Hwtool PKOB4
Program "$HEX_FILE"
Reset
Quit
EOF

    "$MDB" "$TMP_SCRIPT" 2>&1 \
        | grep -v "^Apr\|^INFO:\|^SLF4J\|^WARNING:\|^\[20\|^Exception\|^\s*at \|ChronicleHash\|chronicle\|WatcherData\|NbPreferences\|NativeLoader\|StreamReader\|PlugNPlay\|setToolReserved\|clearToolReserved\|Tool Detach\|Tool Attach\|addNewReservation\|releaseTool\|MCT:USB\|MCSP:USB\|MCUSP:\|ReservationManager\|MPLABComm\|jvm.watcher" \
        || true

    rm -f "$TMP_SCRIPT"
    echo ""
    echo "Flash complete. Connect serial console at 115200 baud to see NuttX NSH prompt."
else
    echo "Error: HEX file does not appear to target PIC32CZCA90 flash (0x0C000000)."
    echo "       No Extended Linear Address record for 0x0C00 found in: $HEX_FILE"
    echo "       Expected record: :020000040C00EE"
    exit 1
fi
