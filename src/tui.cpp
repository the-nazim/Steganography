#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/screen/terminal.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <iomanip>
#include <sys/stat.h>

#include "stegano_wrapper.h"
#include <cstdlib>

using namespace ftxui;

// Helper: format bytes to human-readable string
std::string format_bytes(long bytes) {
    if (bytes < 1024) return std::to_string(bytes) + " B";
    if (bytes < 1024 * 1024) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << (bytes / 1024.0) << " KB";
        return ss.str();
    }
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1) << (bytes / (1024.0 * 1024.0)) << " MB";
    return ss.str();
}

// Helper: get file size from path
long get_file_size_from_path(const std::string &path) {
    struct stat st;
    if (stat(path.c_str(), &st) == 0) return st.st_size;
    return -1;
}

// Helper: file type description from extension
std::string file_type_description(const std::string &ext) {
    if (ext == ".pdf") return "PDF Document";
    if (ext == ".doc" || ext == ".docx") return "Word Document";
    if (ext == ".xls" || ext == ".xlsx") return "Spreadsheet";
    if (ext == ".mp3") return "Audio (MP3)";
    if (ext == ".wav") return "Audio (WAV)";
    if (ext == ".flac") return "Audio (FLAC)";
    if (ext == ".aac") return "Audio (AAC)";
    if (ext == ".ogg") return "Audio (OGG)";
    if (ext == ".jpg" || ext == ".jpeg") return "Image (JPEG)";
    if (ext == ".png") return "Image (PNG)";
    if (ext == ".gif") return "Image (GIF)";
    if (ext == ".bmp") return "Image (BMP)";
    if (ext == ".svg") return "Image (SVG)";
    if (ext == ".mp4") return "Video (MP4)";
    if (ext == ".avi") return "Video (AVI)";
    if (ext == ".mkv") return "Video (MKV)";
    if (ext == ".zip") return "Archive (ZIP)";
    if (ext == ".tar" || ext == ".gz") return "Archive (TAR/GZ)";
    if (ext == ".pem" || ext == ".key") return "Crypto Key";
    if (ext == ".p12" || ext == ".pfx") return "Certificate Bundle";
    if (ext == ".txt") return "Text File";
    if (ext == ".json") return "JSON Data";
    if (ext == ".xml") return "XML Data";
    if (ext == ".csv") return "CSV Data";
    if (ext == ".py" || ext == ".js" || ext == ".c" || ext == ".cpp") return "Source Code";
    if (ext == "") return "Unknown";
    return "File" + ext;
}

// ============================================
// Main TUI Application
// ============================================
int main()
{
    auto screen = ScreenInteractive::Fullscreen();

    // State
    std::string status_message;
    bool show_status = false;
    bool is_error = false;
    int current_page = 0;

    // Input fields
    std::string src_image = "";
    std::string secret_file = "";
    std::string secret_message = "";
    std::string output_file = "";
    std::string stego_image = "";
    std::string decode_output = "";
    std::string info_stego_image = "";
    std::string info_result = "";
    bool info_found = false;

    // ============================================
    // Components for each page
    // ============================================

    // Main menu
    auto main_menu = Container::Vertical({
        Button(" [1] Encode    - Hide data in an image ", [&]() { current_page = 1; show_status = false; }),
        Button(" [2] Decode    - Extract hidden data ", [&]() { current_page = 4; show_status = false; }),
        Button(" [3] File Info - Inspect stego image ", [&]() { current_page = 8; show_status = false; info_result = ""; }),
        Button(" [4] Help      - Usage guide ", [&]() { current_page = 7; show_status = false; }),
        Button(" [0] Exit ", [&]() { screen.Exit(); }),
    });

    // Encode mode selection
    auto encode_select = Container::Vertical({
        Button(" [1] Hide a file in an image ", [&]() { current_page = 2; show_status = false; }),
        Button(" [2] Hide a message in an image ", [&]() { current_page = 3; show_status = false; }),
        Button(" [0] Back ", [&]() { current_page = 0; show_status = false; }),
    });

    // Encode file
    auto enc_file_src = Input(&src_image, "Source image path");
    auto enc_file_sec = Input(&secret_file, "Secret file path");
    auto enc_file_out = Input(&output_file, "Output image [default: encode.bmp]");
    auto enc_file_btn = Button(" Encode ", [&]() {
        if (src_image.empty() || secret_file.empty()) {
            status_message = "Please fill in source image and secret file";
            is_error = true;
            show_status = true;
            return;
        }
        if (output_file.empty()) output_file = "encode.bmp";
        int ret = stegano_encode_file(src_image.c_str(), secret_file.c_str(), output_file.c_str());
        status_message = (ret == 0) ? "File encoded successfully into " + output_file : "Failed to encode file";
        is_error = (ret != 0);
        show_status = true;
    });
    auto enc_file_back = Button(" Back ", [&]() { current_page = 0; show_status = false; });
    auto enc_file_container = Container::Vertical({
        enc_file_src, enc_file_sec, enc_file_out,
        Container::Horizontal({enc_file_btn, enc_file_back}),
    });

    // Encode message
    auto enc_msg_src = Input(&src_image, "Source image path");
    auto enc_msg_sec = Input(&secret_message, "Secret message");
    auto enc_msg_out = Input(&output_file, "Output image [default: encode.bmp]");
    auto enc_msg_btn = Button(" Encode ", [&]() {
        if (src_image.empty() || secret_message.empty()) {
            status_message = "Please fill in source image and message";
            is_error = true;
            show_status = true;
            return;
        }
        if (output_file.empty()) output_file = "encode.bmp";
        int ret = stegano_encode_message(src_image.c_str(), secret_message.c_str(), output_file.c_str());
        status_message = (ret == 0) ? "Message encoded successfully into " + output_file : "Failed to encode message";
        is_error = (ret != 0);
        show_status = true;
    });
    auto enc_msg_back = Button(" Back ", [&]() { current_page = 0; show_status = false; });
    auto enc_msg_container = Container::Vertical({
        enc_msg_src, enc_msg_sec, enc_msg_out,
        Container::Horizontal({enc_msg_btn, enc_msg_back}),
    });

    // Decode mode selection
    auto decode_select = Container::Vertical({
        Button(" [1] Decode to file ", [&]() { current_page = 5; show_status = false; }),
        Button(" [2] Decode to terminal ", [&]() { current_page = 6; show_status = false; }),
        Button(" [0] Back ", [&]() { current_page = 0; show_status = false; }),
    });

    // Decode to file
    auto dec_file_stego = Input(&stego_image, "Stego image path");
    auto dec_file_out = Input(&output_file, "Output file [default: decode.txt]");
    auto dec_file_btn = Button(" Decode ", [&]() {
        if (stego_image.empty()) {
            status_message = "Please enter stego image path";
            is_error = true;
            show_status = true;
            return;
        }
        if (output_file.empty()) output_file = "decode.txt";
        int ret = stegano_decode_to_file(stego_image.c_str(), output_file.c_str());
        status_message = (ret == 0) ? "Decoded successfully to " + output_file : "Failed to decode";
        is_error = (ret != 0);
        show_status = true;
    });
    auto dec_file_back = Button(" Back ", [&]() { current_page = 0; show_status = false; });
    auto dec_file_container = Container::Vertical({
        dec_file_stego, dec_file_out,
        Container::Horizontal({dec_file_btn, dec_file_back}),
    });

    // Decode to terminal
    auto dec_term_stego = Input(&stego_image, "Stego image path");
    auto dec_term_btn = Button(" Decode ", [&]() {
        if (stego_image.empty()) {
            status_message = "Please enter stego image path";
            is_error = true;
            show_status = true;
            return;
        }
        char *result = stegano_decode_to_string(stego_image.c_str());
        if (result != NULL) {
            decode_output = result;
            status_message = "Message decoded successfully";
            is_error = false;
            free(result);
        } else {
            decode_output = "";
            status_message = "Failed to decode";
            is_error = true;
        }
        show_status = true;
    });
    auto dec_term_back = Button(" Back ", [&]() { current_page = 0; show_status = false; });
    auto dec_term_container = Container::Vertical({
        dec_term_stego,
        Container::Horizontal({dec_term_btn, dec_term_back}),
    });

    // File Info
    auto info_stego_input = Input(&info_stego_image, "Stego image path");
    auto info_btn = Button(" Inspect ", [&]() {
        if (info_stego_image.empty()) {
            status_message = "Please enter a stego image path";
            is_error = true;
            show_status = true;
            return;
        }
        SteganoFileInfo finfo;
        int ret = stegano_get_info(info_stego_image.c_str(), &finfo);
        if (ret == 0 && finfo.has_hidden_data) {
            std::string ext = finfo.extension;
            std::string type_desc = file_type_description(ext);
            std::ostringstream ss;
            ss << "Hidden file: " << type_desc << "\n"
               << "Extension: " << ext << "\n"
               << "File size: " << format_bytes(finfo.file_size);
            info_result = ss.str();
            info_found = true;
            status_message = "Hidden data detected in " + info_stego_image;
            is_error = false;
        } else {
            info_result = "No hidden data found.";
            info_found = false;
            status_message = "No hidden data found in " + info_stego_image;
            is_error = true;
        }
        show_status = true;
    });
    auto info_back = Button(" Back ", [&]() { current_page = 0; show_status = false; });
    auto info_container = Container::Vertical({
        info_stego_input,
        Container::Horizontal({info_btn, info_back}),
    });

    // Help
    auto help_back = Button(" Back ", [&]() { current_page = 0; show_status = false; });

    // ============================================
    // Tab container
    // ============================================
    auto tabs = Container::Tab({
        main_menu,
        encode_select,
        enc_file_container,
        enc_msg_container,
        decode_select,
        dec_file_container,
        dec_term_container,
        help_back,
        info_container,
    }, &current_page);

    // ============================================
    // Renderer
    // ============================================
    auto renderer = Renderer(tabs, [&]() -> Element {

        Elements content;

        // ── Page: Main Menu ──
        if (current_page == 0) {
            content.push_back(border(
                vbox({
                    text("  S T E G A N O  ") | bold | color(Color::Cyan) | center,
                    text("  Hide secrets in plain sight  ") | dim | center,
                })
            ) | color(Color::Cyan));
            content.push_back(text(""));
            content.push_back(text("  Main Menu:") | bold | color(Color::White));
            content.push_back(text(""));
            content.push_back(hbox({text("  [1] "), text("Encode    ") | bold, text(" - Hide data in an image") | dim}));
            content.push_back(hbox({text("  [2] "), text("Decode    ") | bold, text(" - Extract hidden data") | dim}));
            content.push_back(hbox({text("  [3] "), text("File Info ") | bold, text(" - Inspect stego image") | dim}));
            content.push_back(hbox({text("  [4] "), text("Help      ") | bold, text(" - Usage guide") | dim}));
            content.push_back(hbox({text("  [0] "), text("Exit      ") | bold}));
            content.push_back(text(""));
            content.push_back(text("  Select an option:") | color(Color::Yellow));
            content.push_back(text(""));
            content.push_back(main_menu->Render() | border | color(Color::White));
        }
        // ── Page: Encode Mode Selection ──
        else if (current_page == 1) {
            content.push_back(text("  ENCODE - Select Mode") | bold | color(Color::Cyan));
            content.push_back(text(""));
            content.push_back(text("  Select encoding mode:") | bold | color(Color::White));
            content.push_back(text(""));
            content.push_back(encode_select->Render() | border | color(Color::White));
        }
        // ── Page: Encode File ──
        else if (current_page == 2) {
            content.push_back(text("  ENCODE FILE") | bold | color(Color::Cyan));
            content.push_back(text(""));
            content.push_back(text("  Hide any file inside an image using LSB steganography.") | dim);
            content.push_back(text(""));
            content.push_back(text("  Supported secret file types:") | bold | color(Color::White));
            content.push_back(text("  Documents:  PDF, DOC, DOCX, XLS, XLSX, TXT, CSV, JSON, XML") | dim);
            content.push_back(text("  Audio:      MP3, WAV, FLAC, AAC, OGG") | dim);
            content.push_back(text("  Images:     JPG, PNG, GIF, BMP, SVG, TIFF") | dim);
            content.push_back(text("  Video:      MP4, AVI, MKV") | dim);
            content.push_back(text("  Archives:   ZIP, TAR, GZ") | dim);
            content.push_back(text("  Crypto:     PEM, KEY, P12, PFX") | dim);
            content.push_back(text("  Source:     PY, JS, C, CPP, and any other file") | dim);
            content.push_back(text(""));
            content.push_back(enc_file_container->Render() | border | color(Color::White));
        }
        // ── Page: Encode Message ──
        else if (current_page == 3) {
            content.push_back(text("  ENCODE MESSAGE") | bold | color(Color::Cyan));
            content.push_back(text(""));
            content.push_back(text("  Hide a text message directly inside an image.") | dim);
            content.push_back(text(""));
            content.push_back(enc_msg_container->Render() | border | color(Color::White));
        }
        // ── Page: Decode Mode Selection ──
        else if (current_page == 4) {
            content.push_back(text("  DECODE - Select Mode") | bold | color(Color::Cyan));
            content.push_back(text(""));
            content.push_back(text("  Select decoding mode:") | bold | color(Color::White));
            content.push_back(text(""));
            content.push_back(decode_select->Render() | border | color(Color::White));
        }
        // ── Page: Decode to File ──
        else if (current_page == 5) {
            content.push_back(text("  DECODE TO FILE") | bold | color(Color::Cyan));
            content.push_back(text(""));
            content.push_back(text("  Extract hidden data from a stego image.") | dim);
            content.push_back(text(""));
            content.push_back(dec_file_container->Render() | border | color(Color::White));
        }
        // ── Page: Decode to Terminal ──
        else if (current_page == 6) {
            content.push_back(text("  DECODE TO TERMINAL") | bold | color(Color::Cyan));
            content.push_back(text(""));
            content.push_back(text("  Print hidden message from a stego image.") | dim);
            content.push_back(text(""));
            content.push_back(dec_term_container->Render() | border | color(Color::White));

            if (!decode_output.empty()) {
                content.push_back(text(""));
                content.push_back(text("  Decoded Message:") | bold | color(Color::White));
                content.push_back(text("  +--------------------------------------+") | color(Color::Cyan));
                content.push_back(hbox({text("  | ") | color(Color::Cyan), text(decode_output) | color(Color::Green)}));
                content.push_back(text("  +--------------------------------------+") | color(Color::Cyan));
            }
        }
        // ── Page: Help ──
        else if (current_page == 7) {
            content.push_back(text("  HELP - Usage Guide") | bold | color(Color::Cyan));
            content.push_back(text(""));
            content.push_back(text("  Command Line Usage:") | bold | color(Color::White));
            content.push_back(text(""));
            content.push_back(hbox({text("  Encode file:   "), text("./steganography -e <image> <secret> [out.bmp]") | color(Color::Green)}));
            content.push_back(hbox({text("  Encode message:"), text("./steganography -e <image> -m \"msg\" [out.bmp]") | color(Color::Green)}));
            content.push_back(hbox({text("  Decode to file:"), text("./steganography -d <stego> [out.txt]") | color(Color::Green)}));
            content.push_back(hbox({text("  Decode to term:"), text("./steganography -d <stego>") | color(Color::Green)}));
            content.push_back(text(""));
            content.push_back(text("  Carrier Image Types (best to worst):") | bold | color(Color::White));
            content.push_back(text("  BMP (best) > PNG > JPG > GIF > TIFF > any file") | dim);
            content.push_back(text(""));
            content.push_back(text("  Secret File Types:") | bold | color(Color::White));
            content.push_back(text("  Any file can be hidden: documents, audio, images, video,") | dim);
            content.push_back(text("  archives, crypto keys, source code, and more."));
            content.push_back(text(""));
            content.push_back(text("  How It Works:") | bold | color(Color::White));
            content.push_back(text("  Hides data in LSB of image bytes. Changes are invisible.") | dim);
            content.push_back(text("  Overflow data is appended for BMP (image still displays).") | dim);
            content.push_back(text(""));
            content.push_back(help_back->Render());
        }
        // ── Page: File Info ──
        else if (current_page == 8) {
            content.push_back(text("  FILE INFO - Inspect Stego Image") | bold | color(Color::Cyan));
            content.push_back(text(""));
            content.push_back(text("  Check if an image contains hidden data and view its details.") | dim);
            content.push_back(text(""));
            content.push_back(info_container->Render() | border | color(Color::White));

            if (!info_result.empty()) {
                content.push_back(text(""));
                if (info_found) {
                    content.push_back(text("  Hidden Data Details:") | bold | color(Color::Green));
                    content.push_back(text("  +--------------------------------------+") | color(Color::Cyan));
                    std::istringstream iss(info_result);
                    std::string line;
                    while (std::getline(iss, line)) {
                        content.push_back(hbox({text("  | ") | color(Color::Cyan), text(line) | color(Color::Green)}));
                    }
                    content.push_back(text("  +--------------------------------------+") | color(Color::Cyan));
                } else {
                    content.push_back(text("  No hidden data found.") | color(Color::Red));
                }
            }
        }

        // Status message at bottom
        if (show_status) {
            content.push_back(text(""));
            if (is_error) {
                content.push_back(hbox({text("  X ") | color(Color::Red), text(status_message) | color(Color::Red)}));
            } else {
                content.push_back(hbox({text("  OK ") | color(Color::Green), text(status_message) | color(Color::Green)}));
            }
        }

        return vbox(content);
    });

    screen.Loop(renderer);
    return 0;
}
