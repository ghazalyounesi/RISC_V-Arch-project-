
#ifndef MEMORY_H
#define MEMORY_H


// memory.h
#pragma once
#include "comm.h"
#include "register_file.h"
class Memory {
public:
    
    Memory();

    bool load_binary(const std::string& filename, uint32_t start_address);

   
    uint8_t read_byte(uint32_t address) const;
    void write_byte(uint32_t address, uint8_t value);

    uint16_t read_half(uint32_t address) const;
    void write_half(uint32_t address, uint16_t value);

    uint32_t read_word(uint32_t address) const;
    void write_word(uint32_t address, uint32_t value);

   
    void dump(uint32_t start_address, uint32_t length) const;

    const std::vector<uint8_t>& get_all_data() const;


private:
    
    std::vector<uint8_t> data;

};


#endif //MEMORY_H
