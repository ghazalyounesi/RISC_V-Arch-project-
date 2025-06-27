//
// Created by ghazal on 6/18/25.
//


// register_file.cpp
#include "register_file.h"
#include <iostream>
#include <iomanip>

RegisterFile::RegisterFile() {
    // مقداردهی اولیه تمام رجیسترها به صفر
    registers.fill(0);
}

void RegisterFile::write(uint8_t reg_index, uint32_t value) {
    // بررسی محدوده شماره رجیستر
    if (reg_index >= 32) {
        throw std::out_of_range("Register index out of bounds (must be 0-31).");
    }

    // بر اساس مستندات، نوشتن در رجیستر x0 بی‌اثر است.
    if (reg_index != 0) {
        registers[reg_index] = value;
    }
}

uint32_t RegisterFile::read(uint8_t reg_index) const {
    if (reg_index >= 32) {
        throw std::out_of_range("Register index out of bounds (must be 0-31).");
    }
    return registers[reg_index];
}

void RegisterFile::dump() const {
    std::cout << "===== Register File Dump =====\n";
    for (int i = 0; i < 32; ++i) {
        std::cout << "x" << std::left << std::setw(2) << i << ": 0x"
                  << std::right << std::setfill('0') << std::setw(8) << std::hex
                  << registers[i] << std::dec << std::setfill(' ');
        if ((i + 1) % 4 == 0) {
            std::cout << "\n";
        } else {
            std::cout << " | ";
        }
    }
    std::cout << "============================\n";
}