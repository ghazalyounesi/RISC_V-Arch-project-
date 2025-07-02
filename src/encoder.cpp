#include "encoder.h"
#include <stdexcept>
#include <bitset>

namespace Utils {
int register_to_int(const std::string& reg) {
    if (reg.empty()) {
        throw std::runtime_error("Invalid register name: empty string");
    }
    if (reg[0] == 'x') {
        return std::stoi(reg.substr(1));
    }
   
    if (reg == "zero") return 0; if (reg == "ra") return 1; if (reg == "sp") return 2;
    if (reg == "gp") return 3; if (reg == "tp") return 4; if (reg == "t0") return 5;
    if (reg == "t1") return 6; if (reg == "t2") return 7; if (reg == "s0" || reg == "fp") return 8;
    if (reg == "s1") return 9; if (reg == "a0") return 10; if (reg == "a1") return 11;
    if (reg == "a2") return 12; if (reg == "a3") return 13; if (reg == "a4") return 14;
    if (reg == "a5") return 15; if (reg == "a6") return 16; if (reg == "a7") return 17;
    if (reg == "s2") return 18; if (reg == "s3") return 19; if (reg == "s4") return 20;
    if (reg == "s5") return 21; if (reg == "s6") return 22; if (reg == "s7") return 23;
    if (reg == "s8") return 24; if (reg == "s9") return 25; if (reg == "s10") return 26;
    if (reg == "s11") return 27; if (reg == "t3") return 28;
    if (reg == "t4") return 29; if (reg == "t5") return 30;
    if (reg == "t6") return 31;

    throw std::runtime_error("Invalid register name: " + reg);
}

int32_t string_to_int(const std::string& s) {
    if (s.empty()) {
        throw std::runtime_error("Invalid immediate value: empty string");
    }
    try {
        if (s.rfind("0x", 0) == 0 || s.rfind("0X", 0) == 0) {
            return std::stoul(s, nullptr, 16);
        }
        return std::stol(s, nullptr, 10);
    } catch (const std::invalid_argument& e) {
        throw std::runtime_error("Invalid number format: " + s);
    } catch (const std::out_of_range& e) {
        throw std::runtime_error("Number out of range: " + s);
    }
}
}

Encoder::Encoder() {
    
    // R-Type
    instruction_map["add"]  = {"0110011", "000", "0000000", 'R'};
    instruction_map["sub"]  = {"0110011", "000", "0100000", 'R'};
    instruction_map["sll"]  = {"0110011", "001", "0000000", 'R'};
    instruction_map["slt"]  = {"0110011", "010", "0000000", 'R'};
    instruction_map["sltu"] = {"0110011", "011", "0000000", 'R'};
    instruction_map["xor"]  = {"0110011", "100", "0000000", 'R'};
    instruction_map["srl"]  = {"0110011", "101", "0000000", 'R'};
    instruction_map["sra"]  = {"0110011", "101", "0100000", 'R'};
    instruction_map["or"]   = {"0110011", "110", "0000000", 'R'};
    instruction_map["and"]  = {"0110011", "111", "0000000", 'R'};

    // I-Type
    instruction_map["addi"]  = {"0010011", "000", "", 'I'};
    instruction_map["slti"]  = {"0010011", "010", "", 'I'}; 
    instruction_map["sltiu"] = {"0010011", "011", "", 'I'}; 
    instruction_map["xori"]  = {"0010011", "100", "", 'I'};
    instruction_map["ori"]   = {"0010011", "110", "", 'I'};
    instruction_map["andi"]  = {"0010011", "111", "", 'I'};
    // I-Type (Shifts) - funct7 is required
    instruction_map["slli"]  = {"0010011", "001", "0000000", 'I'}; 
    instruction_map["srli"]  = {"0010011", "101", "0000000", 'I'}; 
    instruction_map["srai"]  = {"0010011", "101", "0100000", 'I'}; 

    // I-Type (Load)
    instruction_map["lb"]    = {"0000011", "000", "", 'I'};
    instruction_map["lh"]    = {"0000011", "001", "", 'I'};
    instruction_map["lw"]    = {"0000011", "010", "", 'I'};
    instruction_map["lbu"]   = {"0000011", "100", "", 'I'};
    instruction_map["lhu"]   = {"0000011", "101", "", 'I'};

    // I-Type (Jump)
    instruction_map["jalr"]  = {"1100111", "000", "", 'I'};

    // S-Type
    instruction_map["sb"]    = {"0100011", "000", "", 'S'};
    instruction_map["sh"]    = {"0100011", "001", "", 'S'};
    instruction_map["sw"]    = {"0100011", "010", "", 'S'};

    // B-Type
    instruction_map["beq"]   = {"1100011", "000", "", 'B'};
    instruction_map["bne"]   = {"1100011", "001", "", 'B'};
    instruction_map["blt"]   = {"1100011", "100", "", 'B'};
    instruction_map["bge"]   = {"1100011", "101", "", 'B'};
    instruction_map["bltu"]  = {"1100011", "110", "", 'B'};
    instruction_map["bgeu"]  = {"1100011", "111", "", 'B'};

    // U-Type
    instruction_map["lui"]   = {"0110111", "", "", 'U'};
    instruction_map["auipc"] = {"0010111", "", "", 'U'};

    // J-Type
    instruction_map["jal"]   = {"1101111", "", "", 'J'};

    // "M" Standard Extension
    // R-Type
    instruction_map["mul"]    = {"0110011", "000", "0000001", 'R'};
    instruction_map["mulh"]   = {"0110011", "001", "0000001", 'R'}; 
    instruction_map["mulhsu"] = {"0110011", "010", "0000001", 'R'}; 
    instruction_map["mulhu"]  = {"0110011", "011", "0000001", 'R'}; 
    instruction_map["div"]    = {"0110011", "100", "0000001", 'R'};
    instruction_map["divu"]   = {"0110011", "101", "0000001", 'R'}; 
    instruction_map["rem"]    = {"0110011", "110", "0000001", 'R'};
    instruction_map["remu"]   = {"0110011", "111", "0000001", 'R'}; 
}

uint32_t Encoder::encode(const std::string& instruction, const std::vector<std::string>& operands,
                         uint32_t current_address, const SymbolTable& symbol_table) {
    if (instruction_map.find(instruction) == instruction_map.end()) {
        throw std::runtime_error("Unknown instruction: " + instruction);
    }
    const auto& info = instruction_map.at(instruction);
    switch (info.type) {
    case 'R': return encode_r_type(info, operands);
    case 'I': return encode_i_type(info, operands);
    case 'S': return encode_s_type(info, operands);
    case 'B': return encode_b_type(info, operands, current_address, symbol_table);
    case 'U': return encode_u_type(info, operands);
    case 'J': return encode_j_type(info, operands, current_address, symbol_table);
    default: throw std::runtime_error("Unknown instruction type for " + instruction);
    }
}

uint32_t Encoder::encode_r_type(const InstructionInfo& info, const std::vector<std::string>& operands) {
    if (operands.size() != 3) throw std::runtime_error("R-type instruction requires 3 operands.");
    uint32_t rd = Utils::register_to_int(operands[0]);
    uint32_t rs1 = Utils::register_to_int(operands[1]);
    uint32_t rs2 = Utils::register_to_int(operands[2]);

    std::string binary_str = info.funct7 +
                             std::bitset<5>(rs2).to_string() +
                             std::bitset<5>(rs1).to_string() +
                             info.funct3 +
                             std::bitset<5>(rd).to_string() +
                             info.opcode;
    return std::stoul(binary_str, nullptr, 2);
}

uint32_t Encoder::encode_i_type(const InstructionInfo& info, const std::vector<std::string>& operands) {
    if (operands.size() < 2) throw std::runtime_error("I-type instruction requires at least 2 operands.");
    uint32_t rd = Utils::register_to_int(operands[0]);
    uint32_t rs1;
    std::string imm_binary_str;

    
    if (info.opcode == "0000011" || info.opcode == "1100111") { 
        size_t L_par = operands[1].find('(');
        size_t R_par = operands[1].find(')');
        if (L_par == std::string::npos || R_par == std::string::npos) {
            throw std::runtime_error("Invalid format for load/jalr instruction: " + operands[1]);
        }
        int32_t imm_val = Utils::string_to_int(operands[1].substr(0, L_par));
        rs1 = Utils::register_to_int(operands[1].substr(L_par + 1, R_par - L_par - 1));
        imm_binary_str = std::bitset<12>(imm_val).to_string();
    }
    
    else if (info.opcode == "0010011" && (info.funct3 == "001" || info.funct3 == "101")) { 
        if (operands.size() != 3) throw std::runtime_error("Shift-immediate instruction requires 3 operands.");
        rs1 = Utils::register_to_int(operands[1]);
        uint32_t shamt = Utils::string_to_int(operands[2]);
        if (shamt > 31) throw std::runtime_error("Shift amount (shamt) must be between 0-31.");
       
        imm_binary_str = info.funct7 + std::bitset<5>(shamt).to_string();
    }
    
    else {
        if (operands.size() != 3) throw std::runtime_error("Immediate instruction requires 3 operands.");
        rs1 = Utils::register_to_int(operands[1]);
        int32_t imm_val = Utils::string_to_int(operands[2]);
        imm_binary_str = std::bitset<12>(imm_val).to_string();
    }

    std::string binary_str = imm_binary_str +
                             std::bitset<5>(rs1).to_string() +
                             info.funct3 +
                             std::bitset<5>(rd).to_string() +
                             info.opcode;
    return std::stoul(binary_str, nullptr, 2);
}

uint32_t Encoder::encode_s_type(const InstructionInfo& info, const std::vector<std::string>& operands) {
    if (operands.size() != 2) throw std::runtime_error("S-type instruction requires 2 operands.");
    uint32_t rs2 = Utils::register_to_int(operands[0]);
    size_t L_par = operands[1].find('(');
    size_t R_par = operands[1].find(')');
    if (L_par == std::string::npos || R_par == std::string::npos) {
        throw std::runtime_error("Invalid format for store instruction: " + operands[1]);
    }
    int32_t imm = Utils::string_to_int(operands[1].substr(0, L_par));
    uint32_t rs1 = Utils::register_to_int(operands[1].substr(L_par + 1, R_par - L_par - 1));

    std::bitset<12> imm_bs(imm);
    std::string imm11_5 = imm_bs.to_string().substr(0, 7);
    std::string imm4_0 = imm_bs.to_string().substr(7, 5);

    std::string binary_str = imm11_5 +
                             std::bitset<5>(rs2).to_string() +
                             std::bitset<5>(rs1).to_string() +
                             info.funct3 +
                             imm4_0 +
                             info.opcode;
    return std::stoul(binary_str, nullptr, 2);
}

uint32_t Encoder::encode_b_type(const InstructionInfo& info, const std::vector<std::string>& operands,
                                uint32_t current_address, const SymbolTable& symbol_table) {
    if (operands.size() != 3) throw std::runtime_error("B-type instruction requires 3 operands.");
    uint32_t rs1 = Utils::register_to_int(operands[0]);
    uint32_t rs2 = Utils::register_to_int(operands[1]);
    uint32_t target_address = symbol_table.get_address(operands[2]);
    int32_t offset = target_address - current_address;

    if (offset % 2 != 0) {
        throw std::runtime_error("Branch offset must be a multiple of 2.");
    }

    std::bitset<13> imm_bs(offset);
    std::string imm_str = imm_bs.to_string();

    std::string imm12 = imm_str.substr(0, 1);
    std::string imm10_5 = imm_str.substr(2, 6);
    std::string imm4_1 = imm_str.substr(8, 4);
    std::string imm11 = imm_str.substr(1, 1);

    std::string binary_str = imm12 + imm10_5 +
                             std::bitset<5>(rs2).to_string() +
                             std::bitset<5>(rs1).to_string() +
                             info.funct3 +
                             imm4_1 + imm11 +
                             info.opcode;
    return std::stoul(binary_str, nullptr, 2);
}

uint32_t Encoder::encode_u_type(const InstructionInfo& info, const std::vector<std::string>& operands) {
    if (operands.size() != 2) throw std::runtime_error("U-type instruction requires 2 operands.");
    uint32_t rd = Utils::register_to_int(operands[0]);
    int32_t imm = Utils::string_to_int(operands[1]);

    std::string imm_str = std::bitset<20>(imm).to_string();

    std::string binary_str = imm_str +
                             std::bitset<5>(rd).to_string() +
                             info.opcode;
    return std::stoul(binary_str, nullptr, 2);
}

uint32_t Encoder::encode_j_type(const InstructionInfo& info, const std::vector<std::string>& operands,
                                uint32_t current_address, const SymbolTable& symbol_table) {
    if (operands.size() != 2) throw std::runtime_error("J-type instruction requires 2 operands.");
    uint32_t rd = Utils::register_to_int(operands[0]);
    uint32_t target_address = symbol_table.get_address(operands[1]);
    int32_t offset = target_address - current_address;

    if (offset % 2 != 0) {
        throw std::runtime_error("JAL offset must be a multiple of 2.");
    }

    std::bitset<21> imm_bs(offset);
    std::string imm_str = imm_bs.to_string();

    std::string imm20 = imm_str.substr(0, 1);
    std::string imm10_1 = imm_str.substr(10, 10);
    std::string imm11 = imm_str.substr(9, 1);
    std::string imm19_12 = imm_str.substr(1, 8);

    std::string binary_str = imm20 + imm10_1 + imm11 + imm19_12 +
                             std::bitset<5>(rd).to_string() +
                             info.opcode;
    return std::stoul(binary_str, nullptr, 2);
}
