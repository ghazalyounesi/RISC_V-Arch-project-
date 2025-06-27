//
// Created by ghazal on 6/17/25.
//


// parser.cpp

/*
#include "parser.h"
#include <stdexcept>
#include <iomanip>

Parser::Parser(const std::string& input, const std::string& output)
        : input_filename(input), output_filename(output) {}

void Parser::assemble() {
    std::cout << "Starting first pass..." << std::endl;
    first_pass();
    std::cout << "First pass complete. Starting second pass..." << std::endl;
    second_pass();
    std::cout << "Assembly successful! Output written to " << output_filename << std::endl;
}

void Parser::first_pass() {
    std::ifstream input_file(input_filename);
    if (!input_file.is_open()) {
        throw std::runtime_error("Could not open input file: " + input_filename);
    }

    symbol_table.clear();
    uint32_t current_address = 0;
    std::string line;

    while (getline(input_file, line)) {
        // Remove comments
        size_t comment_pos = line.find('#');
        if (comment_pos != std::string::npos) {
            line = line.substr(0, comment_pos);
        }
        line = Utils::trim(line);
        if (line.empty()) continue;

        // Check for labels
        size_t label_pos = line.find(':');
        if (label_pos != std::string::npos) {
            std::string label = Utils::trim(line.substr(0, label_pos));
            symbol_table.add_label(label, current_address);
            line = Utils::trim(line.substr(label_pos + 1));
        }

        if (line.empty()) continue;

        // Parse instruction and operands
        std::stringstream ss(line);
        std::string instruction;
        ss >> instruction;

        std::vector<std::string> operands;
        std::string operand_part;
        std::getline(ss, operand_part);
        operands = Utils::split(operand_part, ',');

        if (instruction[0] == '.') { // Directive
            handle_directive(instruction, operands, current_address, nullptr);
        } else if (pseudo_handler.is_pseudo(instruction)) {
            current_address += pseudo_handler.get_instruction_size(instruction, operands);
        } else {
            current_address += 4; // Real instruction
        }
    }
}

void Parser::second_pass() {
    std::ifstream input_file(input_filename);
    std::ofstream output_file(output_filename, std::ios::binary);

    if (!input_file.is_open()) throw std::runtime_error("Could not open input file.");
    if (!output_file.is_open()) throw std::runtime_error("Could not open output file.");

    uint32_t current_address = 0;
    std::string line;

    while (getline(input_file, line)) {
        size_t comment_pos = line.find('#');
        if (comment_pos != std::string::npos) {
            line = line.substr(0, comment_pos);
        }
        line = Utils::trim(line);
        if (line.empty()) continue;

        size_t label_pos = line.find(':');
        if (label_pos != std::string::npos) {
            line = Utils::trim(line.substr(label_pos + 1));
        }
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string instruction;
        ss >> instruction;

        std::string operand_part;
        std::getline(ss, operand_part);
        std::vector<std::string> operands = Utils::split(operand_part, ',');

        if (instruction[0] == '.') {
            handle_directive(instruction, operands, current_address, &output_file);
        } else if (pseudo_handler.is_pseudo(instruction)) {
            auto expanded_instructions = pseudo_handler.expand(instruction, operands);
            for (const auto& real_instr_str : expanded_instructions) {
                std::stringstream expanded_ss(real_instr_str);
                std::string real_instr;
                expanded_ss >> real_instr;
                std::string real_op_part;
                std::getline(expanded_ss, real_op_part);
                auto real_operands = Utils::split(real_op_part, ',');

                uint32_t machine_code = encoder.encode(real_instr, real_operands, current_address, symbol_table);
                output_file.write(reinterpret_cast<const char*>(&machine_code), sizeof(machine_code));
                current_address += 4;
            }
        } else {
            uint32_t machine_code = encoder.encode(instruction, operands, current_address, symbol_table);
            output_file.write(reinterpret_cast<const char*>(&machine_code), sizeof(machine_code));
            current_address += 4;
        }
    }
}

void Parser::handle_directive(const std::string& directive, const std::vector<std::string>& args,
                              uint32_t& address, std::ofstream* out_file) {
    if (directive == ".org") {
        address = Utils::string_to_int(args[0]);
    } else if (directive == ".word") {
        uint32_t value = Utils::string_to_int(args[0]);
        if (out_file) out_file->write(reinterpret_cast<const char*>(&value), 4);
        address += 4;
    } else if (directive == ".half") {
        uint16_t value = Utils::string_to_int(args[0]);
        if (out_file) out_file->write(reinterpret_cast<const char*>(&value), 2);
        address += 2;
    } else if (directive == ".byte") {
        uint8_t value = Utils::string_to_int(args[0]);
        if (out_file) out_file->write(reinterpret_cast<const char*>(&value), 1);
        address += 1;
    } else if (directive == ".align") {
        uint32_t alignment = 1 << Utils::string_to_int(args[0]);
        uint32_t padding = (alignment - (address % alignment)) % alignment;
        if (out_file) {
            for (uint32_t i = 0; i < padding; ++i) {
                char zero = 0;
                out_file->write(&zero, 1);
            }
        }
        address += padding;
    }
}

*/

#include "parser.h"
#include <sstream>      // برای کار با string stream
#include <stdexcept>
#include <iomanip>      // برای فرمت هگز
#include <algorithm>    // برای std::transform

#include <iomanip>      // برای std::hex

// ====================================================================================
// بخش عمومی: اینترفیس برای ارتباط با UI
// ====================================================================================

// سازنده فقط مقادیر اولیه را تنظیم می‌کند
Parser::Parser() : current_region(CodeRegion::REACHABLE) {}

// متد کمکی برای تبدیل جدول نمادها به یک رشته قابل نمایش برای UI
std::string Parser::getFormattedSymbolTable() const {
    std::stringstream ss;
    ss << "Label\t\tAddress\n";
    ss << "-----------------------\n";

    // استفاده از تابعی که به SymbolTable اضافه کردیم
    const auto& symbols = symbol_table.get_all_symbols();

    if (symbols.empty()) {
        ss << "(No symbols defined)";
    } else {
        for (const auto& pair : symbols) {
            // \t برای ایجاد فاصله مناسب بین ستون‌ها
            ss << pair.first << "\t\t0x" << std::hex << pair.second << std::dec << "\n";
        }
    }
    return ss.str();
}

// این تابع توسط UI فراخوانی می‌شود تا Pass 1 را اجرا کند
FirstPassResult Parser::runFirstPass(const std::string& assembly_code) {
    // ریست کردن وضعیت برای هر بار اجرای جدید
    symbol_table.clear();
    warnings.clear();
    current_region = CodeRegion::REACHABLE;

    FirstPassResult result;
    std::stringstream input_stream(assembly_code);

    try {
        first_pass(input_stream); // اجرای pass 1 روی کد ورودی از UI
        result.success = true;
        result.symbol_table_output = getFormattedSymbolTable();
        result.warnings = this->warnings; // کپی کردن هشدارهای پیدا شده
    } catch (const std::runtime_error& e) {
        result.success = false;
        // قرار دادن پیام خطای مرگبار در لیست هشدارها برای نمایش در UI
        result.warnings.push_back("FATAL ERROR: " + std::string(e.what()));
    }
    
    return result;
}

// این تابع توسط UI فراخوانی می‌شود تا Pass 2 را اجرا کند
SecondPassResult Parser::runSecondPass(const std::string& assembly_code) {
    SecondPassResult result;
    std::stringstream input_stream(assembly_code);
    std::stringstream binary_output_stream(std::ios::out | std::ios::in | std::ios::binary);

    // توجه: pass دوم به جدول نمادهایی که در pass اول ساخته شده، نیاز دارد.
    // به همین دلیل است که symbol_table یک عضو از کلاس Parser است.

    try {
        second_pass(input_stream, binary_output_stream);
        result.success = true;
        
        // تبدیل خروجی باینری به رشته هگز برای نمایش در UI
        std::string binary_str = binary_output_stream.str();
        std::stringstream hex_ss;
        hex_ss << std::hex << std::setfill('0');

        for (size_t i = 0; i < binary_str.length(); ++i) {
            // نمایش هر 4 بایت (یک دستور) در یک خط جدید
            if (i > 0 && i % 4 == 0) {
                hex_ss << "\n";
            }
            hex_ss << "0x" << std::setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(binary_str[i])) << " ";
        }
        result.binary_hex_output = hex_ss.str();

        // ذخیره داده باینری خام برای استفاده‌های احتمالی دیگر (مثلاً ذخیره در فایل)
        result.binary_data.assign(binary_str.begin(), binary_str.end());

    } catch (const std::runtime_error& e) {
        result.success = false;
        result.binary_hex_output = "FATAL ERROR: " + std::string(e.what());
    }

    return result;
}

// ====================================================================================
// بخش خصوصی: پیاده‌سازی منطق اصلی اسمبلر
// ====================================================================================

// پیاده‌سازی کامل first_pass با منطق هشدار
// در parser.cpp، این تابع را جایگزین کنید

void Parser::first_pass(std::istream& input_stream) {
    const uint32_t START_ADDRESS = 0x1000; // آدرس شروع اجرای کد

    // ریست کردن وضعیت‌ها
    symbol_table.clear();
    warnings.clear();
    current_region = CodeRegion::UNREACHABLE; // به طور پیش‌فرض همه چیز غیرقابل دسترس است

    uint32_t current_address = 0;
    std::string line;
    int line_number = 0;

    while (getline(input_stream, line)) {
        line_number++;

        size_t comment_pos = line.find('#');
        if (comment_pos != std::string::npos) {
            line = line.substr(0, comment_pos);
        }
        line = Utils::trim(line);
        if (line.empty()) continue;

        // اگر آدرس فعلی از آدرس شروع کد عبور کرده، منطقه را قابل دسترس می‌کنیم
        // این کار باعث می‌شود داده‌های قبل از 0x1000 نادیده گرفته شوند
        if (current_address >= START_ADDRESS && current_region == CodeRegion::UNREACHABLE) {
            current_region = CodeRegion::REACHABLE;
        }

        size_t label_pos = line.find(':');
        if (label_pos != std::string::npos) {
            std::string label = Utils::trim(line.substr(0, label_pos));
            symbol_table.add_label(label, current_address);
            line = Utils::trim(line.substr(label_pos + 1));
            // یک لیبل فقط در صورتی منطقه را قابل دسترس می‌کند که در بخش کد باشد
            if (current_address >= START_ADDRESS) {
                current_region = CodeRegion::REACHABLE;
            }
        }

        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string token;
        ss >> token;

        std::string operand_part;
        std::getline(ss, operand_part);
        std::vector<std::string> operands = Utils::split(operand_part, ',');

        if (token[0] == '.') { // Directive
            if (token == ".word" || token == ".byte" || token == ".half") {
                if (current_region == CodeRegion::REACHABLE) {
                    warnings.push_back("Data directive '" + token + "' at line " + std::to_string(line_number) +
                                       " is in a reachable code block. PC might try to execute this data.");
                }
            }
            handle_directive(token, operands, current_address, nullptr, line_number);
        } else { // Instruction
            // اگر یک دستورالعمل در بخش کد باشد، آن منطقه قطعاً قابل دسترس است
            if (current_address >= START_ADDRESS) {
                current_region = CodeRegion::REACHABLE;
            }

            std::string instruction_lower = token;
            std::transform(instruction_lower.begin(), instruction_lower.end(), instruction_lower.begin(), ::tolower);

            // فقط پرش‌های غیرشرطی و صریح جریان را قطع می‌کنند
            if (instruction_lower == "j" || instruction_lower == "jal" || instruction_lower == "ret") {
                current_region = CodeRegion::UNREACHABLE;
            }

            if (pseudo_handler.is_pseudo(token)) {
                current_address += pseudo_handler.get_instruction_size(token, operands);
            } else {
                current_address += 4;
            }
        }
    }
}
// پیاده‌سازی کامل second_pass
void Parser::second_pass(std::istream& input_stream, std::ostream& output_stream) {
    uint32_t current_address = 0;
    std::string line;
    int line_number = 0;

    while (getline(input_stream, line)) {
        line_number++;
        size_t comment_pos = line.find('#');
        if (comment_pos != std::string::npos) {
            line = line.substr(0, comment_pos);
        }
        line = Utils::trim(line);
        if (line.empty()) continue;

        size_t label_pos = line.find(':');
        if (label_pos != std::string::npos) {
            line = Utils::trim(line.substr(label_pos + 1));
        }
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string instruction;
        ss >> instruction;

        std::string operand_part;
        std::getline(ss, operand_part);
        std::vector<std::string> operands = Utils::split(operand_part, ',');

        if (instruction[0] == '.') {
            handle_directive(instruction, operands, current_address, &output_stream, line_number);
        } else if (pseudo_handler.is_pseudo(instruction)) {
            auto expanded_instructions = pseudo_handler.expand(instruction, operands);
            for (const auto& real_instr_str : expanded_instructions) {
                std::stringstream expanded_ss(real_instr_str);
                std::string real_instr;
                expanded_ss >> real_instr;
                std::string real_op_part;
                std::getline(expanded_ss, real_op_part);
                auto real_operands = Utils::split(real_op_part, ',');

                uint32_t machine_code = encoder.encode(real_instr, real_operands, current_address, symbol_table);
                output_stream.write(reinterpret_cast<const char*>(&machine_code), sizeof(machine_code));
                current_address += 4;
            }
        } else {
            uint32_t machine_code = encoder.encode(instruction, operands, current_address, symbol_table);
            output_stream.write(reinterpret_cast<const char*>(&machine_code), sizeof(machine_code));
            current_address += 4;
        }
    }
}
/*
// پیاده‌سازی کامل handle_directive
void Parser::handle_directive(const std::string& directive, const std::vector<std::string>& args,
                              uint32_t& address, std::ostream* out_stream, int line_number) {
    if (directive == ".org") {
        address = Utils::string_to_int(args[0]);
    } else if (directive == ".word") {
        uint32_t value = Utils::string_to_int(args[0]);
        if (out_stream) out_stream->write(reinterpret_cast<const char*>(&value), 4);
        address += 4;
    } else if (directive == ".half") {
        uint16_t value = Utils::string_to_int(args[0]);
        if (out_stream) out_stream->write(reinterpret_cast<const char*>(&value), 2);
        address += 2;
    } else if (directive == ".byte") {
        uint8_t value = Utils::string_to_int(args[0]);
        if (out_stream) out_stream->write(reinterpret_cast<const char*>(&value), 1);
        address += 1;
    } else if (directive == ".align") {
        uint32_t alignment = 1 << Utils::string_to_int(args[0]);
        uint32_t padding = (alignment - (address % alignment)) % alignment;
        if (out_stream) {
            for (uint32_t i = 0; i < padding; ++i) {
                char zero = 0;
                out_stream->write(&zero, 1);
            }
        }
        address += padding;
    }
}
*/
void Parser::handle_directive(const std::string& directive, const std::vector<std::string>& args,
                              uint32_t& address, std::ostream* out_stream, int line_number) {
    if (directive == ".org") {
        if (args.empty() || args[0].empty()) {
            throw std::runtime_error("Error at line " + std::to_string(line_number) + ": .org directive requires an address argument.");
        }
        uint32_t target_address = Utils::string_to_int(args[0]);

        if (target_address < address) {
            // --- بخش اصلاح شده برای ساخت پیام خطا ---
            std::stringstream error_ss;
            error_ss << "Error at line " << line_number
                     << ": .org directive cannot move address backward from 0x"
                     << std::hex << address << " to 0x" << target_address;
            throw std::runtime_error(error_ss.str());
            // --- پایان بخش اصلاح شده ---
        }

        // فقط در Pass 2 (زمانی که خروجی واقعی می‌نویسیم) پدینگ را اضافه کن
        if (out_stream) {
            uint32_t padding_bytes = target_address - address;
            if (padding_bytes > 0) {
                char zero_byte = 0;
                for (uint32_t i = 0; i < padding_bytes; ++i) {
                    out_stream->write(&zero_byte, 1);
                }
            }
        }

        // در هر دو Pass، شمارنده آدرس را به مکان جدید منتقل کن
        address = target_address;

    } else if (directive == ".word") {
        if (args.empty() || args[0].empty()) throw std::runtime_error("Error at line " + std::to_string(line_number) + ": .word needs a value.");
        uint32_t value = Utils::string_to_int(args[0]);
        if (out_stream) out_stream->write(reinterpret_cast<const char*>(&value), 4);
        address += 4;
    } else if (directive == ".half") {
        if (args.empty() || args[0].empty()) throw std::runtime_error("Error at line " + std::to_string(line_number) + ": .half needs a value.");
        uint16_t value = Utils::string_to_int(args[0]);
        if (out_stream) out_stream->write(reinterpret_cast<const char*>(&value), 2);
        address += 2;
    } else if (directive == ".byte") {
        if (args.empty() || args[0].empty()) throw std::runtime_error("Error at line " + std::to_string(line_number) + ": .byte needs a value.");
        uint8_t value = Utils::string_to_int(args[0]);
        if (out_stream) out_stream->write(reinterpret_cast<const char*>(&value), 1);
        address += 1;
    } else if (directive == ".align") {
        if (args.empty() || args[0].empty()) throw std::runtime_error("Error at line " + std::to_string(line_number) + ": .align needs a value.");
        uint32_t alignment = 1 << Utils::string_to_int(args[0]);
        uint32_t padding = (alignment - (address % alignment)) % alignment;
        if (out_stream) {
            char zero = 0;
            for (uint32_t i = 0; i < padding; ++i) {
                out_stream->write(&zero, 1);
            }
        }
        address += padding;
    }
}
