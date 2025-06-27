//
// Created by ghazal on 6/19/25.
//


// instruction.cpp
#include "instruction.h"
#include <map>

void Instruction::decode(uint32_t raw_instruction) {
    raw = raw_instruction;
    opcode = raw & 0x7F; // استخراج ۷ بیت opcode

    // استخراج فیلدهای مشترک
    rd = (raw >> 7) & 0x1F;
    funct3 = (raw >> 12) & 0x7;
    rs1 = (raw >> 15) & 0x1F;
    rs2 = (raw >> 20) & 0x1F;

    // دیکود بر اساس opcode
    switch (opcode) {
        case 0b0110011: // R-type
            type = InstrType::R;
            funct7 = (raw >> 25) & 0x7F;
            if (funct7 == 0b0000000) {
                if (funct3 == 0b000) mnemonic = "add";
                else if (funct3 == 0b111) mnemonic = "and";
                else if (funct3 == 0b110) mnemonic = "or";
                else if (funct3 == 0b100) mnemonic = "xor";
                else if (funct3 == 0b001) mnemonic = "sll";
                else if (funct3 == 0b101) mnemonic = "srl";
                else if (funct3 == 0b010) mnemonic = "slt";
                else if (funct3 == 0b011) mnemonic = "sltu";
            } else if (funct7 == 0b0100000) {
                if (funct3 == 0b000) mnemonic = "sub";
                else if (funct3 == 0b101) mnemonic = "sra";
            }
            break;

        case 0b0010011: // I-type (immediate arithmetics)
        case 0b0000011: // I-type (loads)
        case 0b1100111: // I-type (jalr)
            type = InstrType::I;
            imm = static_cast<int32_t>(raw) >> 20; // Sign-extend
            if (opcode == 0b0010011) {
                if (funct3 == 0b000) mnemonic = "addi";
                else if (funct3 == 0b111) mnemonic = "andi";
                else if (funct3 == 0b110) mnemonic = "ori";
                else if (funct3 == 0b100) mnemonic = "xori";
                else if (funct3 == 0b010) mnemonic = "slti";
                else if (funct3 == 0b011) mnemonic = "sltiu";
                else if (funct3 == 0b001) { mnemonic = "slli"; funct7 = (raw >> 25) & 0x7F;} // special case
                else if (funct3 == 0b101) { mnemonic = "srli_srai"; funct7 = (raw >> 25) & 0x7F;} // special case
            } else if (opcode == 0b0000011) {
                if (funct3 == 0b010) mnemonic = "lw";
                else if (funct3 == 0b000) mnemonic = "lb";
                else if (funct3 == 0b100) mnemonic = "lbu";
                else if (funct3 == 0b101) mnemonic = "lhu";
            } else { // jalr
                mnemonic = "jalr";
            }
            break;

        case 0b0100011: // S-type
            type = InstrType::S;
            imm = ((static_cast<int32_t>(raw) >> 25) << 5) | ((raw >> 7) & 0x1F);
            if (funct3 == 0b010) mnemonic = "sw";
            else if (funct3 == 0b000) mnemonic = "sb";
            else if (funct3 == 0b001) mnemonic = "sh";
            break;

        case 0b1100011: // B-type
            type = InstrType::B;
            imm = (((static_cast<int32_t>(raw) >> 31) & 1) << 12) |
                  (((raw >> 7) & 1) << 11) |
                  (((raw >> 25) & 0x3F) << 5) |
                  (((raw >> 8) & 0xF) << 1);
            imm = (imm << 19) >> 19; // Sign-extend from 13 bits
            if (funct3 == 0b000) mnemonic = "beq";
            else if (funct3 == 0b001) mnemonic = "bne";
            else if (funct3 == 0b100) mnemonic = "blt";
            else if (funct3 == 0b101) mnemonic = "bge";
            else if (funct3 == 0b110) mnemonic = "bltu";
            else if (funct3 == 0b111) mnemonic = "bgeu";
            break;

        case 0b0110111: // U-type (lui)
            type = InstrType::U;
            mnemonic = "lui";
            imm = static_cast<int32_t>(raw & 0xFFFFF000);
            break;

        case 0b0010111: // U-type (auipc)
            type = InstrType::U;
            mnemonic = "auipc";
            imm = static_cast<int32_t>(raw & 0xFFFFF000);
            break;

        case 0b1101111: // J-type
            type = InstrType::J;
            mnemonic = "jal";
            imm = (((static_cast<int32_t>(raw) >> 31) & 1) << 20) |
                  ((raw >> 12) & 0xFF) << 12 |
                  ((raw >> 20) & 1) << 11 |
                  ((raw >> 21) & 0x3FF) << 1;
            imm = (imm << 11) >> 11; // Sign-extend from 21 bits
            break;

        default:
            type = InstrType::UNKNOWN;
            mnemonic = "unknown";
            break;
    }
}