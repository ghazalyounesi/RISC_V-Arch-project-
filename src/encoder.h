#ifndef ENCODER_H
#define ENCODER_H


// encoder.h
#pragma once
#include "common.h"
#include "symbol_table.h"

class Encoder {
public:
    Encoder();
    uint32_t encode(const std::string& instruction, const std::vector<std::string>& operands,
                    uint32_t current_address, const SymbolTable& symbol_table);

private:
    std::unordered_map<std::string, InstructionInfo> instruction_map;

    uint32_t encode_r_type(const InstructionInfo& info, const std::vector<std::string>& operands);
    uint32_t encode_i_type(const InstructionInfo& info, const std::vector<std::string>& operands);
    uint32_t encode_s_type(const InstructionInfo& info, const std::vector<std::string>& operands);
    uint32_t encode_b_type(const InstructionInfo& info, const std::vector<std::string>& operands,
                           uint32_t current_address, const SymbolTable& symbol_table);
    uint32_t encode_u_type(const InstructionInfo& info, const std::vector<std::string>& operands);
    uint32_t encode_j_type(const InstructionInfo& info, const std::vector<std::string>& operands,
                           uint32_t current_address, const SymbolTable& symbol_table);
};


#endif //ENCODER_H
