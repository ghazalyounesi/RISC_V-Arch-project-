#include "pseudo.h"

Pseudo::Pseudo() {
    pseudo_map["nop"] = 1;
    pseudo_map["li"] = 1;
    pseudo_map["mv"] = 1;
    pseudo_map["not"] = 1;
    pseudo_map["neg"] = 1;
}

bool Pseudo::is_pseudo(const std::string& instruction) const {
    return pseudo_map.count(instruction);
}

int Pseudo::get_instruction_size(const std::string& instruction, const std::vector<std::string>& operands) {
    if (instruction == "li") {
        int32_t imm = Utils::string_to_int(operands[1]);
       
        if (imm >= -2048 && imm <= 2047) {
            return 4; 
        }
        return 8; 
    }
    
    return 4;
}

std::vector<std::string> Pseudo::expand(const std::string& instruction, const std::vector<std::string>& operands) {
    if (instruction == "nop") {
        return {"addi x0, x0, 0"};
    }
    if (instruction == "mv") {
        return {"addi " + operands[0] + ", " + operands[1] + ", 0"};
    }
    if (instruction == "not") {
        return {"xori " + operands[0] + ", " + operands[1] + ", -1"};
    }
    if (instruction == "neg") {
        return {"sub " + operands[0] + ", x0, " + operands[1]};
    }
    if (instruction == "li") {
        int32_t imm = Utils::string_to_int(operands[1]);
        if (imm >= -2048 && imm <= 2047) {
            return {"addi " + operands[0] + ", x0, " + std::to_string(imm)};
        } else {
            
            uint32_t u_imm = imm;
            uint32_t lower12 = u_imm & 0xFFF;
            uint32_t upper20 = u_imm >> 12;

            
            if ((lower12 >> 11) & 1) { 
                upper20 += 1; 
            }

            std::string rd = operands[0];
            return {
                "lui " + rd + ", " + std::to_string(upper20),
                "addi " + rd + ", " + rd + ", " + std::to_string(static_cast<int16_t>(lower12))
            };
        }
    }
    return {};
}
