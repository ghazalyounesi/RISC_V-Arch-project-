//
// Created by ghazal on 6/19/25.
//

#ifndef ARCH_INSTRUCTION_H
#define ARCH_INSTRUCTION_H


// instruction.h
#pragma once
#include "comm.h"

// برای خوانایی بهتر
enum class InstrType { R, I, S, B, U, J, UNKNOWN };

class Instruction {
public:
    uint32_t raw; // دستور خام ۳۲ بیتی
    InstrType type;
    uint8_t opcode;
    uint8_t rd, rs1, rs2;
    uint8_t funct3, funct7;
    int32_t imm; // مقدار immediate پس از sign-extension

    // نام دستور برای استفاده در لاگ و UI
    std::string mnemonic;

    Instruction() : raw(0), type(InstrType::UNKNOWN) {}

    // تابع اصلی برای دیکود کردن
    void decode(uint32_t raw_instruction);
};


#endif //ARCH_INSTRUCTION_H
