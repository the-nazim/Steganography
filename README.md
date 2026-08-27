# 🔐 Steganography Tool

A C-based steganography tool that hides secret files or messages inside image files using **LSB (Least Significant Bit)** encoding. The changes are imperceptible to the human eye — the output image looks completely normal.

## ✨ Features

- **Hide files** inside images (any format: BMP, PNG, JPG, etc.)
- **Hide messages** directly via command line (`-m` flag)
- **Extract hidden data** to file or print to terminal
- **Any image type** accepted — no format restrictions
- **Overflow handling** — if secret data exceeds image capacity, the file grows without affecting the image display
- **Interactive TUI** — colorful terminal menu for easy use
- **Magic string verification** — detects whether an image contains hidden data

## 🚀 Quick Start

### Build

```bash
mkdir build && cd build
cmake ..
make
cd ..
```

### Run the TUI

```bash
./stegano_tui.sh
```

### Or use the CLI directly

```bash
# Encode a file into an image
./build/steganography -e beautiful.bmp secret.txt hidden.bmp

# Encode a message into an image
./build/steganography -e beautiful.bmp -m "Meet me at 5" hidden.bmp

# Decode to file
./build/steganography -d hidden.bmp decoded.txt

# Decode to terminal
./build/steganography -d hidden.bmp

# Show help
./build/steganography -h
```

## 📖 Usage

### Command Line Interface

```
For Encoding:  ./steganography -e <image> <secret_file> [output.bmp]
For Message:   ./steganography -e <image> -m "message" [output.bmp]
For Decoding:  ./steganography -d <stego_image> [output.txt]
Show Help:     ./steganography -h | --help
```

| Argument | Description |
|----------|-------------|
| `-e` | Encode mode |
| `-d` | Decode mode |
| `-m` | Hide a text message directly |
| `-h` / `--help` | Show usage guide |

### Terminal User Interface (TUI)

Two TUI options are available:

#### Option 1: C++ FTXUI TUI (Recommended)

```bash
# Auto-builds if not compiled yet
./stegano_tui.sh
```

A native C++ interactive terminal UI built with [FTXUI](https://github.com/jusonleung/FTXUI). Features:
- Animated transitions and spinner animations
- File browser with directory navigation
- Real-time input validation
- Color-coded status messages
- Spinner animation during encode/decode operations

#### Option 2: Bash TUI (Fallback)

If the C++ TUI binary is not available, the launcher script falls back to a bash-based menu:

```
  [1] 🔒 Encode  - Hide data in an image
  [2] 🔓 Decode  - Extract hidden data
  [3] 📖 Help    - Usage guide & examples
  [0] 🚪 Exit
```

## 🔧 How It Works

### LSB Steganography

The tool modifies the **least significant bit** of each byte in the image to encode secret data:

```
Original pixel byte:  11010110
Secret data bit:      1
Modified pixel byte:  11010111  (only last bit changed)
```

Changing the last bit of a byte alters its value by at most 1 (out of 255), making the change **invisible to the human eye**.

### Encoding Process

1. **Detect image type** — identifies BMP by magic bytes `BM`
2. **Copy image header** — preserves format metadata (54 bytes for BMP)
3. **Encode magic string** — `#*` marker to verify hidden data exists
4. **Encode file extension** — so decoder knows the original file type
5. **Encode file size** — so decoder knows how much data to extract
6. **Encode secret data** — using LSB on image bytes
7. **Handle overflow** — if data exceeds capacity, append after image content

### Overflow Handling

When secret data is larger than the image's LSB capacity:

- **BMP files**: Overflow data is appended after pixel data. BMP viewers ignore extra bytes, so the image displays normally.
- **Other formats**: Data is appended after the file content.

```
┌─────────────────────────────────────────┐
│ BMP Header (54 bytes)                   │
├─────────────────────────────────────────┤
│ Pixel Data                              │
│  └─ LSB-encoded secret data             │
├─────────────────────────────────────────┤
│ Remaining pixel data (unchanged)        │
├─────────────────────────────────────────┤
│ Overflow data (appended)                │
└─────────────────────────────────────────┘
```

## 📁 Project Structure

```
Steganography/
├── CMakeLists.txt              # Build configuration (C + C++ with FTXUI)
├── README.md                   # This file
├── stegano_tui.sh              # Bash TUI launcher script
├── include/
│   ├── common.h                # Magic string definition
│   ├── types.h                 # Custom types (Status, OperationType)
│   ├── encode.h                # Encoding structures and prototypes
│   ├── decode.h                # Decoding structures and prototypes
│   └── stegano_wrapper.h       # C wrapper for C++ TUI integration
├── src/
│   ├── main.c                  # CLI entry point and argument parsing
│   ├── encode.c                # Encoding logic (LSB embedding)
│   ├── decode.c                # Decoding logic (LSB extraction)
│   ├── stegano_wrapper.c       # C wrapper implementation
│   └── tui.cpp                 # FTXUI-based interactive TUI (C++)
└── build/                      # Build output (generated)
    ├── steganography           # CLI binary
    └── stegano_tui             # Interactive TUI binary
```

## 🛠️ Build Requirements

- **C compiler** (GCC recommended)
- **C++ compiler** with C++17 support (GCC 8+ / Clang 7+)
- **CMake** 3.14 or higher
- **Linux/macOS** (Windows with WSL or MinGW)
- **FTXUI** — fetched automatically via CMake (no manual install needed)

### Install Dependencies

```bash
# Ubuntu/Debian
sudo apt install gcc g++ cmake

# macOS (with Homebrew)
brew install gcc cmake

# Fedora/RHEL
sudo dnf install gcc gcc-c++ cmake
```

> **Note**: The FTXUI library is downloaded automatically during the CMake build step via `FetchContent`. No manual installation is required.

## 📋 Examples

### Example 1: Hide a Password File

```bash
# Create a secret file
echo "admin:password123" > passwords.txt

# Hide it in an image
./build/steganography -e beautiful.bmp passwords.txt secret.bmp

# Later, extract it
./build/steganography -d secret.bmp recovered.txt
cat recovered.txt
# Output: admin:password123
```

### Example 2: Send a Secret Message

```bash
# Hide a message
./build/steganography -e photo.bmp -m "Meet me at the cafe at 5pm" msg.bmp

# Print the message directly
./build/steganography -d msg.bmp
# Output: Message: Meet me at the cafe at 5pm
```

### Example 3: Overflow Test (Small Image)

```bash
# Even a tiny image can hold large messages
./build/steganography -e tiny.bmp -m "This message is much longer than the image can hold!" overflow.bmp

# The image still displays normally, and the full message is extracted
./build/steganography -d overflow.bmp
# Output: Message: This message is much longer than the image can hold!
```

## 🧪 Supported Formats

| Format | Support | Notes |
|--------|---------|-------|
| **BMP** | ✅ Full | Best support — image displays normally even with overflow |
| **PNG** | ✅ Raw | Treated as raw bytes |
| **JPG** | ✅ Raw | Treated as raw bytes |
| **GIF** | ✅ Raw | Treated as raw bytes |
| **TIFF** | ✅ Raw | Treated as raw bytes |
| **Any file** | ✅ Raw | Treated as raw bytes |

> **Note**: BMP files provide the best experience because the image displays normally even when overflow data is appended. Other formats may show artifacts if the data significantly modifies the file.

## ⚠️ Limitations

- **BMP recommended** — best compatibility and image quality
- **Not加密** — this is steganography (hiding), not encryption (scrambling). The data is hidden, not encrypted.
- **Single file** — can only hide one file per image
- **Text files only** — currently supports `.txt` file extension for encoded files

## 📜 License

This project is for educational purposes. Use responsibly.

---

**Made with ❤️ using LSB Steganography and FTXUI**
