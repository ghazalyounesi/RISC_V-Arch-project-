//
// Created by ghazal on 6/17/25.
//

#ifndef ARCH_PROJECT_PARSER_H
#define ARCH_PROJECT_PARSER_H

/*
// parser.h
#pragma once
#include "common.h"
#include "symbol_table.h"
#include "encoder.h"
#include "pseudo.h"
#include <fstream>

class Parser {
public:
    Parser(const std::string& input_filename, const std::string& output_filename);
    void assemble();

private:
    void first_pass();
    void second_pass();

    void handle_directive(const std::string& directive, const std::vector<std::string>& args,
                          uint32_t& address, std::ofstream* out_file);

    std::string input_filename;
    std::string output_filename;
    SymbolTable symbol_table;
    Encoder encoder;
    Pseudo pseudo_handler;
};
*/


// parser.h
#pragma once
#include "common.h"
#include "symbol_table.h"
#include "pseudo.h"
#include "encoder.h"
// ... سایر include ها

#include <iomanip> // برای فرمت هگز

// ساختار برای نگهداری نتایج Pass 1
struct FirstPassResult {
    bool success = true;
    std::string symbol_table_output;
    std::vector<std::string> warnings;
};

// ساختار برای نگهداری نتایج Pass 2
struct SecondPassResult {
    bool success = true;
    std::string binary_hex_output;
    std::vector<uint8_t> binary_data; // داده باینری خام
};


class Parser {
public:
    Parser(); // سازنده ساده‌تر می‌شود

    // توابع جدید برای فراخوانی از UI
    FirstPassResult runFirstPass(const std::string& assembly_code);
    SecondPassResult runSecondPass(const std::string& assembly_code);

private:
    // توابع داخلی بدون تغییر باقی می‌مانند
    void first_pass(std::istream& input_stream);
    void second_pass(std::istream& input_stream, std::ostream& output_stream);
    void handle_directive(const std::string& directive, const std::vector<std::string>& args,
                          uint32_t& address, std::ostream* out_stream, int line_number);
                          
    // Helper برای فرمت کردن جدول نمادها
    std::string getFormattedSymbolTable() const;


    // وضعیت‌ها بین فراخوانی‌ها حفظ می‌شوند
    SymbolTable symbol_table;
    Encoder encoder;
    Pseudo pseudo_handler;
    
    enum class CodeRegion { REACHABLE, UNREACHABLE };
    CodeRegion current_region;
    std::vector<std::string> warnings;
};

#endif //ARCH_PROJECT_PARSER_H
