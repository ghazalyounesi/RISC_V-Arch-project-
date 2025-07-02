#ifndef INSTRUCTION_H
#define INSTRUCTION_H

// instruction.h

#pragma once
#include "comm.h"
#include <string>


enum class InstrType {
    R,
    I_compute,      
    I_load,        
    S,
    B,
    U,
    J,
    UNKNOWN
   
};

class Instruction {
public:
    uint32_t raw; 
    InstrType type;
    uint8_t opcode;
    uint8_t rd, rs1, rs2;
    uint8_t funct3, funct7;
    int32_t imm; 

    std::string mnemonic;

    Instruction() : raw(0), type(InstrType::UNKNOWN) {}

    void decode(uint32_t raw_instruction);
};
#endif //INSTRUCTION_H
