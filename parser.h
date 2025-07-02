
#ifndef PARSER_H
#define PARSER_H




// parser.h
#pragma once
#include "common.h"
#include "symbol_table.h"
#include "pseudo.h"
#include "encoder.h"


#include <iomanip> 

struct FirstPassResult {
    bool success = true;
    std::string symbol_table_output;
    std::vector<std::string> warnings;
};

struct SecondPassResult {
    bool success = true;
    std::string binary_hex_output;
    std::vector<uint8_t> binary_data; 
};


class Parser {
public:
    Parser(); 

    FirstPassResult runFirstPass(const std::string& assembly_code);
    SecondPassResult runSecondPass(const std::string& assembly_code);

private:
   
    void first_pass(std::istream& input_stream);
    void second_pass(std::istream& input_stream, std::ostream& output_stream);
    void handle_directive(const std::string& directive, const std::vector<std::string>& args,
                          uint32_t& address, std::ostream* out_stream, int line_number);

    std::string getFormattedSymbolTable() const;

    SymbolTable symbol_table;
    Encoder encoder;
    Pseudo pseudo_handler;

    enum class CodeRegion { REACHABLE, UNREACHABLE };
    CodeRegion current_region;
    std::vector<std::string> warnings;
};

#endif //PARSER_H
