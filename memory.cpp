#include "memory.h"
#include <iostream>
#include <iomanip>

Memory::Memory() {
   
    data.resize(MEMORY_SIZE, 0);
}

bool Memory::load_binary(const std::string& filename, uint32_t start_address) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Could not open binary file '" << filename << "'." << std::endl;
        return false;
    }

    
    if (start_address >= MEMORY_SIZE) {
        std::cerr << "Error: Load address is out of memory bounds." << std::endl;
        return false;
    }

    
    char byte;
    uint32_t address = start_address; 
    while (file.get(byte) && address < MEMORY_SIZE) {
        data[address] = static_cast<uint8_t>(byte);
        address++;
    }

    file.close();
    std::cout << "Loaded " << (address - start_address) << " bytes from '" << filename
              << "' into memory starting at 0x" << std::hex << start_address << std::dec << "." << std::endl;
    return true;
}



uint8_t Memory::read_byte(uint32_t address) const {
    if (address >= MEMORY_SIZE) {
        throw std::out_of_range("Memory read out of bounds.");
    }
    return data[address];
}

uint16_t Memory::read_half(uint32_t address) const {
    if (address + 1 >= MEMORY_SIZE) {
        throw std::out_of_range("Memory read out of bounds.");
    }
    
    uint16_t value = static_cast<uint16_t>(data[address+1]) << 8 |
                     static_cast<uint16_t>(data[address]);
    return value;
}

uint32_t Memory::read_word(uint32_t address) const {
    if (address + 3 >= MEMORY_SIZE) {
        throw std::out_of_range("Memory read out of bounds.");
    }
    
    uint32_t value = static_cast<uint32_t>(data[address+3]) << 24 |
                     static_cast<uint32_t>(data[address+2]) << 16 |
                     static_cast<uint32_t>(data[address+1]) << 8  |
                     static_cast<uint32_t>(data[address]);
    return value;
}




void Memory::write_byte(uint32_t address, uint8_t value) {
    if (address >= MEMORY_SIZE) {
        throw std::out_of_range("Memory write out of bounds.");
    }
    data[address] = value;
}

void Memory::write_half(uint32_t address, uint16_t value) {
    if (address + 1 >= MEMORY_SIZE) {
        throw std::out_of_range("Memory write out of bounds.");
    }
    
    data[address]   = value & 0xFF;
    data[address+1] = (value >> 8) & 0xFF;
}

void Memory::write_word(uint32_t address, uint32_t value) {
    if (address + 3 >= MEMORY_SIZE) {
        throw std::out_of_range("Memory write out of bounds.");
    }
    
    data[address]   = value & 0xFF;
    data[address+1] = (value >> 8) & 0xFF;
    data[address+2] = (value >> 16) & 0xFF;
    data[address+3] = (value >> 24) & 0xFF;
}

void Memory::dump(uint32_t start_address, uint32_t length) const {
    std::cout << "===== Memory Dump from 0x" << std::hex << start_address << " =====\n";
    for (uint32_t i = 0; i < length; i += 16) {
        
        std::cout << "0x" << std::right << std::setfill('0') << std::setw(8) << std::hex << (start_address + i) << ": ";

        for (uint32_t j = 0; j < 16; ++j) {
            if (start_address + i + j < MEMORY_SIZE) {
                std::cout << std::setw(2) << static_cast<unsigned>(data[start_address + i + j]) << " ";
            } else {
                std::cout << "   ";
            }
        }
        std::cout << "\n";
    }
    std::cout << std::dec << std::setfill(' ') << "======================================\n";
}

const std::vector<uint8_t>& Memory::get_all_data() const {
    return data;
}
