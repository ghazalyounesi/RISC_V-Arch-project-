#include "parser.h"
#include <sstream>      
#include <stdexcept>
#include <iomanip>      
#include <algorithm>    

#include <iomanip>      
Parser::Parser() : current_region(CodeRegion::REACHABLE) {}

std::string Parser::getFormattedSymbolTable() const {
    std::stringstream ss;
    ss << "Label\t\tAddress\n";
    ss << "-----------------------\n";

    const auto& symbols = symbol_table.get_all_symbols();

    if (symbols.empty()) {
        ss << "(No symbols defined)";
    } else {
        for (const auto& pair : symbols) {
           
            ss << pair.first << "\t\t0x" << std::hex << pair.second << std::dec << "\n";
        }
    }
    return ss.str();
}

FirstPassResult Parser::runFirstPass(const std::string& assembly_code) {
   
    symbol_table.clear();
    warnings.clear();
    current_region = CodeRegion::REACHABLE;

    FirstPassResult result;
    std::stringstream input_stream(assembly_code);

    try {
        first_pass(input_stream); 
        result.success = true;
        result.symbol_table_output = getFormattedSymbolTable();
        result.warnings = this->warnings; 
    } catch (const std::runtime_error& e) {
        result.success = false;
        
        result.warnings.push_back("FATAL ERROR: " + std::string(e.what()));
    }

    return result;
}

SecondPassResult Parser::runSecondPass(const std::string& assembly_code) {
    SecondPassResult result;
    std::stringstream input_stream(assembly_code);
    std::stringstream binary_output_stream(std::ios::out | std::ios::in | std::ios::binary);

    try {
        second_pass(input_stream, binary_output_stream);
        result.success = true;

        std::string binary_str = binary_output_stream.str();
        std::stringstream hex_ss;
        hex_ss << std::hex << std::setfill('0');

        for (size_t i = 0; i < binary_str.length(); ++i) {
            
            if (i > 0 && i % 4 == 0) {
                hex_ss << "\n";
            }
            hex_ss << "0x" << std::setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(binary_str[i])) << " ";
        }
        result.binary_hex_output = hex_ss.str();

       
        result.binary_data.assign(binary_str.begin(), binary_str.end());

    } catch (const std::runtime_error& e) {
        result.success = false;
        result.binary_hex_output = "FATAL ERROR: " + std::string(e.what());
    }

    return result;
}

void Parser::first_pass(std::istream& input_stream) {
    const uint32_t START_ADDRESS = 0x1000; 
    
    symbol_table.clear();
    warnings.clear();
    current_region = CodeRegion::UNREACHABLE; 

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

        if (current_address >= START_ADDRESS && current_region == CodeRegion::UNREACHABLE) {
            current_region = CodeRegion::REACHABLE;
        }

        size_t label_pos = line.find(':');
        if (label_pos != std::string::npos) {
            std::string label = Utils::trim(line.substr(0, label_pos));
            symbol_table.add_label(label, current_address);
            line = Utils::trim(line.substr(label_pos + 1));
            
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

        if (token[0] == '.') { 
            if (token == ".word" || token == ".byte" || token == ".half") {
                if (current_region == CodeRegion::REACHABLE) {
                    warnings.push_back("Data directive '" + token + "' at line " + std::to_string(line_number) +
                                       " is in a reachable code block. PC might try to execute this data.");
                }
            }
            handle_directive(token, operands, current_address, nullptr, line_number);
        } else { 
           
            if (current_address >= START_ADDRESS) {
                current_region = CodeRegion::REACHABLE;
            }

            std::string instruction_lower = token;
            std::transform(instruction_lower.begin(), instruction_lower.end(), instruction_lower.begin(), ::tolower);

            
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

void Parser::handle_directive(const std::string& directive, const std::vector<std::string>& args,
                              uint32_t& address, std::ostream* out_stream, int line_number) {
    if (directive == ".org") {
        if (args.empty() || args[0].empty()) {
            throw std::runtime_error("Error at line " + std::to_string(line_number) + ": .org directive requires an address argument.");
        }
        uint32_t target_address = Utils::string_to_int(args[0]);

        if (target_address < address) {
           
            std::stringstream error_ss;
            error_ss << "Error at line " << line_number
                     << ": .org directive cannot move address backward from 0x"
                     << std::hex << address << " to 0x" << target_address;
            throw std::runtime_error(error_ss.str());
            
        }

       
        if (out_stream) {
            uint32_t padding_bytes = target_address - address;
            if (padding_bytes > 0) {
                char zero_byte = 0;
                for (uint32_t i = 0; i < padding_bytes; ++i) {
                    out_stream->write(&zero_byte, 1);
                }
            }
        }

        
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
