#!/bin/bash

# ============================================
# Steganography TUI - Terminal User Interface
# ============================================

BINARY="./build/steganography"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
WHITE='\033[1;37m'
GRAY='\033[0;90m'
NC='\033[0m' # No Color
BOLD='\033[1m'
DIM='\033[2m'

# Check if binary exists
check_binary() {
    if [ ! -f "$BINARY" ]; then
        echo -e "${RED}[ERROR] Binary not found at $BINARY${NC}"
        echo -e "${YELLOW}Building project...${NC}"
        mkdir -p build && cd build && cmake .. && make
        if [ $? -ne 0 ]; then
            echo -e "${RED}[ERROR] Build failed. Please build manually first.${NC}"
            exit 1
        fi
        cd ..
        echo -e "${GREEN}[OK] Build successful!${NC}"
    fi
}

# Print banner
print_banner() {
    clear
    echo -e "${CYAN}"
    echo "╔══════════════════════════════════════════════════════╗"
    echo "║                                                      ║"
    echo "║        ░█████╗░██╗░░░██╗████████╗███████╗           ║"
    echo "║        ██╔══██╗██║░░░██║╚══██╔══╝██╔════╝           ║"
    echo "║        ██║░░╚═╝██║░░░██║░░░██║░░░█████╗░░           ║"
    echo "║        ██║░░██╗██║░░░██║░░░██║░░░██╔══╝░░           ║"
    echo "║        ╚█████╔╝╚██████╔╝░░░██║░░░███████╗           ║"
    echo "║        ░╚════╝░░╚═════╝░░░░╚═╝░░░╚══════╝           ║"
    echo "║                                                      ║"
    echo "║          ${WHITE}Terminal User Interface${NC}${CYAN}                    ║"
    echo "║          ${DIM}Hide secrets in plain sight${NC}${CYAN}               ║"
    echo "║                                                      ║"
    echo "╚══════════════════════════════════════════════════════╝"
    echo -e "${NC}"
}

# Print section header
print_section() {
    echo -e "\n${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${WHITE}${BOLD}  $1${NC}"
    echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}\n"
}

# Print success message
print_success() {
    echo -e "\n${GREEN}  ✓ $1${NC}"
}

# Print error message
print_error() {
    echo -e "\n${RED}  ✗ $1${NC}"
}

# Print info message
print_info() {
    echo -e "${BLUE}  ℹ $1${NC}"
}

# Print warning message
print_warning() {
    echo -e "${YELLOW}  ⚠ $1${NC}"
}

# Get file with tab completion hint
get_file() {
    local prompt="$1"
    local default="$2"
    local file_type="$3" # "image", "text", or "any"

    while true; do
        echo -ne "${YELLOW}  → ${prompt}: ${NC}"
        read -r filepath

        # Use default if empty
        if [ -z "$filepath" ] && [ -n "$default" ]; then
            filepath="$default"
            echo -e "${DIM}    (using default: $filepath)${NC}"
        fi

        if [ -z "$filepath" ]; then
            print_error "Please enter a file path."
            continue
        fi

        # Validate file exists (for input files)
        if [ "$file_type" = "input" ] && [ ! -f "$filepath" ]; then
            print_error "File not found: $filepath"
            continue
        fi

        echo "$filepath"
        return 0
    done
}

# Get message input
get_message() {
    local prompt="$1"
    while true; do
        echo -ne "${YELLOW}  → ${prompt}: ${NC}"
        read -r message

        if [ -z "$message" ]; then
            print_error "Please enter a message."
            continue
        fi

        echo "$message"
        return 0
    done
}

# Run command with progress display
run_with_progress() {
    local title="$1"
    shift

    echo -e "\n${BLUE}  ⏳ ${title}...${NC}\n"

    # Run command and capture output
    output=$("$@" 2>&1)
    exit_code=$?

    # Print output
    while IFS= read -r line; do
        if [[ "$line" == *"Error"* ]] || [[ "$line" == *"Failed"* ]] || [[ "$line" == *"error"* ]]; then
            echo -e "  ${RED}│${NC} $line"
        elif [[ "$line" == *"Success"* ]] || [[ "$line" == *"success"* ]] || [[ "$line" == *"✓"* ]]; then
            echo -e "  ${GREEN}│${NC} $line"
        elif [[ "$line" == *"Info"* ]] || [[ "$line" == *"Info:"* ]]; then
            echo -e "  ${BLUE}│${NC} $line"
        elif [[ "$line" == *"Message:"* ]]; then
            echo -e "  ${MAGENTA}│${NC} $line"
        elif [[ "$line" == *"Encoded"* ]] || [[ "$line" == *"Decoded"* ]]; then
            echo -e "  ${CYAN}│${NC} $line"
        elif [[ "$line" == *"width"* ]] || [[ "$line" == *"height"* ]] || [[ "$line" == *"capacity"* ]] || [[ "$line" == *"size"* ]]; then
            echo -e "  ${GRAY}│${NC} $line"
        else
            echo -e "  ${WHITE}│${NC} $line"
        fi
    done <<< "$output"

    echo ""
    if [ $exit_code -eq 0 ]; then
        print_success "Operation completed successfully!"
    else
        print_error "Operation failed (exit code: $exit_code)"
    fi

    return $exit_code
}

# =====================
# ENCODE MENU
# =====================
encode_menu() {
    print_section "🔒 ENCODE - Hide Data in Image"

    echo -e "  ${WHITE}Select encoding mode:${NC}\n"
    echo -e "  ${CYAN}[1]${NC} 📄 Hide a file in an image"
    echo -e "  ${CYAN}[2]${NC} 💬 Hide a message in an image"
    echo -e "  ${CYAN}[0]${NC} ← Back to main menu"
    echo ""

    echo -ne "${YELLOW}  → Select option (0-2): ${NC}"
    read -r choice

    case $choice in
        1) encode_file ;;
        2) encode_message ;;
        0) return ;;
        *)
            print_error "Invalid option"
            sleep 1
            encode_menu
            ;;
    esac
}

# Encode file
encode_file() {
    print_section "📄 ENCODE FILE - Hide Any File"

    echo -e "  ${DIM}Hide any file inside an image using LSB steganography.${NC}\n"
    echo -e "  ${WHITE}Supported secret file types:${NC}"
    echo -e "  ${GREEN}Documents:${NC} PDF, DOC, DOCX, XLS, XLSX, TXT, CSV, JSON, XML"
    echo -e "  ${GREEN}Audio:${NC}     MP3, WAV, FLAC, AAC, OGG"
    echo -e "  ${GREEN}Images:${NC}    JPG, PNG, GIF, BMP, SVG, TIFF"
    echo -e "  ${GREEN}Video:${NC}     MP4, AVI, MKV"
    echo -e "  ${GREEN}Archives:${NC}  ZIP, TAR, GZ"
    echo -e "  ${GREEN}Crypto:${NC}    PEM, KEY, P12, PFX"
    echo -e "  ${GREEN}Source:${NC}    PY, JS, C, CPP, and any other file\n"

    # Get source image
    echo -ne "${YELLOW}  → Source image path: ${NC}"
    read -r src_image
    if [ ! -f "$src_image" ]; then
        print_error "File not found: $src_image"
        sleep 1
        encode_file
        return
    fi

    # Get secret file
    echo -ne "${YELLOW}  → Secret file path: ${NC}"
    read -r secret_file
    if [ ! -f "$secret_file" ]; then
        print_error "File not found: $secret_file"
        sleep 1
        encode_file
        return
    fi

    # Get output file (optional)
    echo -ne "${YELLOW}  → Output image path ${DIM}[default: encode.bmp]${NC}: ${NC}"
    read -r output_image
    if [ -z "$output_image" ]; then
        output_image="encode.bmp"
    fi

    # Show file info
    local file_ext="${secret_file##*.}"
    local file_size=$(stat --printf="%s" "$secret_file" 2>/dev/null || stat -f%z "$secret_file" 2>/dev/null || echo "unknown")
    local file_size_hr="$file_size B"
    if [ "$file_size" != "unknown" ]; then
        if [ "$file_size" -gt 1048576 ]; then
            file_size_hr="$(echo "scale=1; $file_size/1048576" | bc 2>/dev/null || echo "$file_size") MB"
        elif [ "$file_size" -gt 1024 ]; then
            file_size_hr="$(echo "scale=1; $file_size/1024" | bc 2>/dev/null || echo "$file_size") KB"
        fi
    fi

    echo -e "\n${WHITE}  Summary:${NC}"
    echo -e "  ${GRAY}Source:${NC}  $src_image"
    echo -e "  ${GRAY}Secret:${NC}  $secret_file"
    echo -e "  ${GRAY}Type:${NC}    .$file_ext ($file_size_hr)"
    echo -e "  ${GRAY}Output:${NC}  $output_image"

    echo -ne "\n${YELLOW}  → Proceed? (y/N): ${NC}"
    read -r confirm
    if [ "$confirm" != "y" ] && [ "$confirm" != "Y" ]; then
        print_warning "Operation cancelled."
        sleep 1
        encode_menu
        return
    fi

    run_with_progress "Encoding .$file_ext file ($file_size_hr) into image" "$BINARY" -e "$src_image" "$secret_file" "$output_image"

    echo -ne "\n${YELLOW}  → Press Enter to continue...${NC}"
    read -r
    encode_menu
}

# Encode message
encode_message() {
    print_section "💬 ENCODE MESSAGE"

    echo -e "  ${DIM}Enter the source image and your secret message.${NC}"
    echo -e "  ${DIM}Supports any image type (BMP, PNG, JPG, etc.) as carrier.${NC}\n"

    # Get source image
    echo -ne "${YELLOW}  → Source image path: ${NC}"
    read -r src_image
    if [ ! -f "$src_image" ]; then
        print_error "File not found: $src_image"
        sleep 1
        encode_message
        return
    fi

    # Get message
    echo -ne "${YELLOW}  → Secret message: ${NC}"
    read -r message
    if [ -z "$message" ]; then
        print_error "Message cannot be empty."
        sleep 1
        encode_message
        return
    fi

    # Get output file (optional)
    echo -ne "${YELLOW}  → Output image path ${DIM}[default: encode.bmp]${NC}: ${NC}"
    read -r output_image
    if [ -z "$output_image" ]; then
        output_image="encode.bmp"
    fi

    echo -e "\n${WHITE}  Summary:${NC}"
    echo -e "  ${GRAY}Source:${NC}  $src_image"
    echo -e "  ${GRAY}Message:${NC} ${MAGENTA}\"$message\"${NC}"
    echo -e "  ${GRAY}Output:${NC}  $output_image"

    echo -ne "\n${YELLOW}  → Proceed? (y/N): ${NC}"
    read -r confirm
    if [ "$confirm" != "y" ] && [ "$confirm" != "Y" ]; then
        print_warning "Operation cancelled."
        sleep 1
        encode_menu
        return
    fi

    run_with_progress "Encoding message into image" "$BINARY" -e "$src_image" -m "$message" "$output_image"

    echo -ne "\n${YELLOW}  → Press Enter to continue...${NC}"
    read -r
    encode_menu
}

# =====================
# DECODE MENU
# =====================
decode_menu() {
    print_section "🔓 DECODE - Extract Hidden Data"

    echo -e "  ${WHITE}Select decoding mode:${NC}\n"
    echo -e "  ${CYAN}[1]${NC} 📄 Decode to file"
    echo -e "  ${CYAN}[2]${NC} 💬 Decode to terminal (print message)"
    echo -e "  ${CYAN}[0]${NC} ← Back to main menu"
    echo ""

    echo -ne "${YELLOW}  → Select option (0-2): ${NC}"
    read -r choice

    case $choice in
        1) decode_to_file ;;
        2) decode_to_terminal ;;
        0) return ;;
        *)
            print_error "Invalid option"
            sleep 1
            decode_menu
            ;;
    esac
}

# Decode to file
decode_to_file() {
    print_section "📄 DECODE TO FILE"

    echo -e "  ${DIM}Enter the stego image path to extract hidden data.${NC}\n"

    # Get stego image
    echo -ne "${YELLOW}  → Stego image path: ${NC}"
    read -r stego_image
    if [ ! -f "$stego_image" ]; then
        print_error "File not found: $stego_image"
        sleep 1
        decode_to_file
        return
    fi

    # Get output file (optional)
    echo -ne "${YELLOW}  → Output file path ${DIM}[default: decode.txt]${NC}: ${NC}"
    read -r output_file
    if [ -z "$output_file" ]; then
        output_file="decode.txt"
    fi

    echo -e "\n${WHITE}  Summary:${NC}"
    echo -e "  ${GRAY}Stego:${NC}   $stego_image"
    echo -e "  ${GRAY}Output:${NC}  $output_file"

    echo -ne "\n${YELLOW}  → Proceed? (y/N): ${NC}"
    read -r confirm
    if [ "$confirm" != "y" ] && [ "$confirm" != "Y" ]; then
        print_warning "Operation cancelled."
        sleep 1
        decode_menu
        return
    fi

    run_with_progress "Decoding image" "$BINARY" -d "$stego_image" "$output_file"

    # Show decoded content
    if [ -f "$output_file" ]; then
        echo -e "\n${WHITE}  📋 Decoded content:${NC}"
        echo -e "${CYAN}  ┌────────────────────────────────────────────┐${NC}"
        while IFS= read -r line; do
            echo -e "${CYAN}  │${NC} ${GREEN}$line${NC}"
        done < "$output_file"
        echo -e "${CYAN}  └────────────────────────────────────────────┘${NC}"
    fi

    echo -ne "\n${YELLOW}  → Press Enter to continue...${NC}"
    read -r
    decode_menu
}

# Decode to terminal
decode_to_terminal() {
    print_section "💬 DECODE TO TERMINAL"

    echo -e "  ${DIM}Enter the stego image path to print hidden message.${NC}\n"

    # Get stego image
    echo -ne "${YELLOW}  → Stego image path: ${NC}"
    read -r stego_image
    if [ ! -f "$stego_image" ]; then
        print_error "File not found: $stego_image"
        sleep 1
        decode_to_terminal
        return
    fi

    echo -e "\n${WHITE}  Summary:${NC}"
    echo -e "  ${GRAY}Stego:${NC}   $stego_image"
    echo -e "  ${GRAY}Output:${NC}  Terminal"

    echo -ne "\n${YELLOW}  → Proceed? (y/N): ${NC}"
    read -r confirm
    if [ "$confirm" != "y" ] && [ "$confirm" != "Y" ]; then
        print_warning "Operation cancelled."
        sleep 1
        decode_menu
        return
    fi

    run_with_progress "Decoding message from image" "$BINARY" -d "$stego_image"

    echo -ne "\n${YELLOW}  → Press Enter to continue...${NC}"
    read -r
    decode_menu
}

# =====================
# HELP MENU
# =====================
help_menu() {
    print_section "📖 HELP - Usage Guide"

    echo -e "${WHITE}  Command Line Usage:${NC}\n"
    echo -e "  ${CYAN}Encode from file:${NC}"
    echo -e "    ./steganography -e <image> <secret_file> [output.bmp]\n"
    echo -e "  ${CYAN}Encode message:${NC}"
    echo -e "    ./steganography -e <image> -m \"message\" [output.bmp]\n"
    echo -e "  ${CYAN}Decode to file:${NC}"
    echo -e "    ./steganography -d <stego_image> [output.txt]\n"
    echo -e "  ${CYAN}Decode to terminal:${NC}"
    echo -e "    ./steganography -d <stego_image>\n"
    echo -e "  ${CYAN}Show help:${NC}"
    echo -e "    ./steganography -h | --help\n"

    echo -e "${WHITE}  Carrier Image Types (best to worst):${NC}\n"
    echo -e "  ${GREEN}✓${NC} BMP (best - image displays normally even with overflow)"
    echo -e "  ${GREEN}✓${NC} PNG, JPG, GIF, TIFF (treated as raw bytes)\n"

    echo -e "${WHITE}  Secret File Types:${NC}\n"
    echo -e "  ${GREEN}Documents:${NC} PDF, DOC, DOCX, XLS, XLSX, TXT, CSV, JSON, XML"
    echo -e "  ${GREEN}Audio:${NC}     MP3, WAV, FLAC, AAC, OGG"
    echo -e "  ${GREEN}Images:${NC}    JPG, PNG, GIF, BMP, SVG, TIFF"
    echo -e "  ${GREEN}Video:${NC}     MP4, AVI, MKV"
    echo -e "  ${GREEN}Archives:${NC}  ZIP, TAR, GZ"
    echo -e "  ${GREEN}Crypto:${NC}    PEM, KEY, P12, PFX"
    echo -e "  ${GREEN}Source:${NC}    PY, JS, C, CPP, and any other file\n"

    echo -e "${WHITE}  How It Works:${NC}\n"
    echo -e "  ${DIM}The tool hides data in the least significant bits (LSB) of image${NC}"
    echo -e "  ${DIM}bytes. Changes are imperceptible to the human eye. If the secret${NC}"
    echo -e "  ${DIM}data exceeds the image's LSB capacity, overflow data is appended${NC}"
    echo -e "  ${DIM}after the image content (works seamlessly for BMP files).${NC}\n"

    echo -e "${WHITE}  Examples:${NC}\n"
    echo -e "  ${CYAN}# Hide a PDF document${NC}"
    echo -e "  ./steganography -e photo.bmp report.pdf hidden.bmp\n"
    echo -e "  ${CYAN}# Hide an audio file${NC}"
    echo -e "  ./steganography -e photo.bmp music.mp3 hidden.bmp\n"
    echo -e "  ${CYAN}# Hide a crypto key${NC}"
    echo -e "  ./steganography -e photo.bmp private.pem hidden.bmp\n"
    echo -e "  ${CYAN}# Hide a quick message${NC}"
    echo -e "  ./steganography -e photo.bmp -m \"Meet me at 5\" hidden.bmp\n"
    echo -e "  ${CYAN}# Extract to file${NC}"
    echo -e "  ./steganography -d hidden.bmp recovered.pdf\n"
    echo -e "  ${CYAN}# Print message directly${NC}"
    echo -e "  ./steganography -d hidden.bmp\n"

    echo -ne "\n${YELLOW}  → Press Enter to continue...${NC}"
    read -r
}

# =====================
# FILE INFO MENU
# =====================
file_info_menu() {
    print_section "🔍 FILE INFO - Inspect Stego Image"

    echo -e "  ${DIM}Check if an image contains hidden data and view its details.${NC}\n"

    # Get stego image
    echo -ne "${YELLOW}  → Stego image path: ${NC}"
    read -r stego_image
    if [ ! -f "$stego_image" ]; then
        print_error "File not found: $stego_image"
        sleep 1
        return
    fi

    # Show image file size
    local img_size=$(stat --printf="%s" "$stego_image" 2>/dev/null || stat -f%z "$stego_image" 2>/dev/null || echo "unknown")
    echo -e "  ${GRAY}Image file size: $img_size bytes${NC}"

    # Run decode to extract metadata (it will fail gracefully if no hidden data)
    echo -e "\n${BLUE}  ⏳ Scanning for hidden data...${NC}\n"
    output=$("$BINARY" -d "$stego_image" 2>&1)
    exit_code=$?

    if echo "$output" | grep -q "Decoded magic string successfully"; then
        echo -e "  ${GREEN}✓ Hidden data detected!${NC}\n"
        echo -e "${WHITE}  Metadata:${NC}"
        echo -e "${CYAN}  ┌────────────────────────────────────────────┐${NC}"
        while IFS= read -r line; do
            if [[ "$line" == *"extn"* ]] || [[ "$line" == *"size"* ]] || [[ "$line" == *"Extension"* ]] || [[ "$line" == *"ext"* ]]; then
                echo -e "${CYAN}  │${NC} ${GREEN}$line${NC}"
            elif [[ "$line" == *"Message:"* ]]; then
                echo -e "${CYAN}  │${NC} ${MAGENTA}$line${NC}"
            fi
        done <<< "$output"
        echo -e "${CYAN}  └────────────────────────────────────────────┘${NC}"
    else
        echo -e "  ${RED}✗ No hidden data found in this image.${NC}"
    fi

    echo -ne "\n${YELLOW}  → Press Enter to continue...${NC}"
    read -r
}

# =====================
# MAIN MENU
# =====================
main_menu() {
    while true; do
        print_banner

        echo -e "  ${WHITE}Main Menu:${NC}\n"
        echo -e "  ${CYAN}[1]${NC} 🔒 ${BOLD}Encode${NC}    - Hide data in an image"
        echo -e "  ${CYAN}[2]${NC} 🔓 ${BOLD}Decode${NC}    - Extract hidden data"
        echo -e "  ${CYAN}[3]${NC} 🔍 ${BOLD}File Info${NC} - Inspect stego image"
        echo -e "  ${CYAN}[4]${NC} 📖 ${BOLD}Help${NC}      - Usage guide & examples"
        echo -e "  ${CYAN}[0]${NC} 🚪 ${BOLD}Exit${NC}"
        echo ""
        echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"

        echo -ne "\n${YELLOW}  → Select option (0-4): ${NC}"
        read -r choice

        case $choice in
            1) encode_menu ;;
            2) decode_menu ;;
            3) file_info_menu ;;
            4) help_menu ;;
            0)
                print_banner
                echo -e "  ${GREEN}Goodbye! Stay secure. 🔐${NC}\n"
                exit 0
                ;;
            *)
                print_error "Invalid option. Please select 0-4."
                sleep 1
                ;;
        esac
    done
}

# =====================
# ENTRY POINT
# =====================
check_binary
main_menu
