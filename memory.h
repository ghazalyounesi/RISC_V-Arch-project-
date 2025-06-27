//
// Created by ghazal on 6/19/25.
//

#ifndef ARCH_MEMORY_H
#define ARCH_MEMORY_H


// memory.h
#pragma once
#include "comm.h"
#include "register_file.h"
class Memory {
public:
    /**
     * @brief سازنده کلاس که حافظه را ایجاد و با صفر پر می‌کند.
     */
    Memory();

    /**
     * @brief یک فایل باینری را در حافظه بارگذاری می‌کند.
     * @param filename مسیر فایل .bin ورودی.
     * فرآیند شبیه‌ساز با بارگذاری فایل باینری در حافظه آغاز می‌شود.
     */
    bool load_binary(const std::string& filename, uint32_t start_address);

    // توابع خواندن و نوشتن در اندازه‌های مختلف (بایت، نیم‌کلمه، کلمه)
    // این توابع برای اجرای دستوراتی مثل lw, lb, sw, sb ضروری خواهند بود.

    uint8_t read_byte(uint32_t address) const;
    void write_byte(uint32_t address, uint8_t value);

    uint16_t read_half(uint32_t address) const;
    void write_half(uint32_t address, uint16_t value);

    uint32_t read_word(uint32_t address) const;
    void write_word(uint32_t address, uint32_t value);

    /**
     * @brief تابعی کمکی برای نمایش بخشی از حافظه (برای دیباگ).
     */
    void dump(uint32_t start_address, uint32_t length) const;


private:
    // حافظه به صورت یک بردار از بایت‌ها شبیه‌سازی می‌شود.
    std::vector<uint8_t> data;

};


#endif //ARCH_MEMORY_H
