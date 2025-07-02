
#include "instruction.h"
#include <cstdint>

void Instruction::decode(uint32_t raw_instruction) {
    raw = raw_instruction;
    opcode = raw & 0x7F;
    rd = (raw >> 7) & 0x1F;
    funct3 = (raw >> 12) & 0x7;
    rs1 = (raw >> 15) & 0x1F;
    rs2 = (raw >> 20) & 0x1F;
    funct7 = (raw >> 25) & 0x7F;

    switch (opcode) {
    // R-type: opcode=0110011
    case 0b0110011:
        type = InstrType::R;
        if (funct7 == 0b0000001) { // M-extension (funct7=0x01)
            switch (funct3) {
            case 0b000: mnemonic = "mul"; break;    // f31
            case 0b001: mnemonic = "mulh"; break;   // f32
            case 0b010: mnemonic = "mulhsu"; break; // f33
            case 0b011: mnemonic = "mulhu"; break;  // f34
            case 0b100: mnemonic = "div"; break;    // f35
            case 0b101: mnemonic = "divu"; break;   // f36
            case 0b110: mnemonic = "rem"; break;    // f37
            case 0b111: mnemonic = "remu"; break;   // f38
            default:    mnemonic = "unknown"; type = InstrType::UNKNOWN; break;
            }
        } else if (funct7 == 0b0000000) { // Base (funct7=0x00)
            switch (funct3) {
            case 0b000: mnemonic = "add"; break;    // f31
            case 0b001: mnemonic = "sll"; break;    // f32
            case 0b010: mnemonic = "slt"; break;    // f33
            case 0b011: mnemonic = "sltu"; break;   // f34
            case 0b100: mnemonic = "xor"; break;    // f35
            case 0b101: mnemonic = "srl"; break;    // f36
            case 0b110: mnemonic = "or"; break;     // f37
            case 0b111: mnemonic = "and"; break;    // f38
            default:    mnemonic = "unknown"; type = InstrType::UNKNOWN; break;
            }
        } else if (funct7 == 0b0100000) { // funct7=0x20
            switch (funct3) {
            case 0b000: mnemonic = "sub"; break;    // f31
            case 0b101: mnemonic = "sra"; break;    // f36
            default:    mnemonic = "unknown"; type = InstrType::UNKNOWN; break;
            }
        } else {
            mnemonic = "unknown";
            type = InstrType::UNKNOWN;
        }
        break;

        // I-type (compute): opcode=0010011 (I1)
    case 0b0010011:
        type = InstrType::I_compute;
        imm = static_cast<int32_t>(raw) >> 20; // Sign-extend
        switch (funct3) {
        case 0b000: mnemonic = "addi"; break; // f31
        case 0b110: mnemonic = "ori"; break;  // f37
        case 0b111: mnemonic = "andi"; break; // f38
        case 0b001: // slli
            mnemonic = "slli";
            imm = (raw >> 20) & 0x1F; 
            break;
        case 0b101: // srli/srai
            if ((raw >> 25) == 0b0100000) { mnemonic = "srai"; }
            else { mnemonic = "srli"; }
            imm = (raw >> 20) & 0x1F; 
            break;
            
        case 0b010: mnemonic = "slti"; break;
        case 0b011: mnemonic = "sltiu"; break;
        case 0b100: mnemonic = "xori"; break;
        default:    mnemonic = "unknown"; type = InstrType::UNKNOWN; break;
        }
        break;

        // I-type (load): opcode=0000011 (I2)
    case 0b0000011:
        type = InstrType::I_load;
        imm = static_cast<int32_t>(raw) >> 20; 
        switch (funct3) {
        case 0b000: mnemonic = "lb"; break;  // f31
        case 0b001: mnemonic = "lh"; break;  // f32
        case 0b010: mnemonic = "lw"; break;  // f33
        case 0b100: mnemonic = "lbu"; break; // f35
        case 0b101: mnemonic = "lhu"; break; // f36
        default:    mnemonic = "unknown"; type = InstrType::UNKNOWN; break;
        }
        break;

       
    case 0b1100111:
       
        type = InstrType::I_load;
        mnemonic = "jalr";
        imm = static_cast<int32_t>(raw) >> 20;
        break;

        
    case 0b0100011:
        type = InstrType::S;
        imm = ((static_cast<int32_t>(raw & 0xFE000000) >> 20) | ((raw >> 7) & 0x1F));
        switch (funct3) {
        case 0b000: mnemonic = "sb"; break; // f31
        case 0b001: mnemonic = "sh"; break; // f32
        case 0b010: mnemonic = "sw"; break; // f33
        default:    mnemonic = "unknown"; type = InstrType::UNKNOWN; break;
        }
        break;

        
    case 0b1100011:
        type = InstrType::B;
        imm = ((static_cast<int32_t>(raw & 0x80000000) >> 19) |
               ((raw & 0x80) << 4) |
               ((raw >> 20) & 0x7E0) |
               ((raw >> 7) & 0x1E));
        switch (funct3) {
        case 0b000: mnemonic = "beq"; break;  // f31
        case 0b001: mnemonic = "bne"; break;  // f32
        case 0b100: mnemonic = "blt"; break;  // f35
        case 0b101: mnemonic = "bge"; break;  // f36
        case 0b110: mnemonic = "bltu"; break; // f37
        case 0b111: mnemonic = "bgeu"; break; // f38
        default:    mnemonic = "unknown"; type = InstrType::UNKNOWN; break;
        }
        break;

        // U-type (lui): opcode=0110111 (U1)
    case 0b0110111:
        type = InstrType::U;
        mnemonic = "lui";
        imm = static_cast<int32_t>(raw & 0xFFFFF000);
        break;

        // U-type (auipc): opcode=0010111 (U2)
    case 0b0010111:
        type = InstrType::U;
        mnemonic = "auipc";
        imm = static_cast<int32_t>(raw & 0xFFFFF000);
        break;

        // J-type: opcode=1101111
    case 0b1101111:
        type = InstrType::J;
        mnemonic = "jal";
        imm = (static_cast<int32_t>(raw & 0x80000000) >> 11) |
              (raw & 0xFF000) |
              ((raw >> 9) & 0x800) |
              ((raw >> 20) & 0x7FE);
        break;

    default:
        type = InstrType::UNKNOWN;
        mnemonic = "unknown";
        break;
    }
}
