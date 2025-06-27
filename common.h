//
// Created by ghazal on 6/17/25.
//

#ifndef ARCH_PROJECT_COMMON_H
#define ARCH_PROJECT_COMMON_H


// common.h
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <algorithm>
#include <sstream>
#include <bitset>

// ساختاری برای نگهداری اطلاعات دستورالعمل‌ها
struct InstructionInfo {
    std::string opcode;
    std::string funct3;
    std::string funct7;
    char type; // R, I, S, B, U, J
};

// ابزارهای کمکی
namespace Utils {
    // حذف فضاهای خالی از ابتدا و انتهای رشته
    inline std::string trim(const std::string& s) {
        size_t first = s.find_first_not_of(" \t\n\r");
        if (std::string::npos == first) return "";
        size_t last = s.find_last_not_of(" \t\n\r");
        return s.substr(first, (last - first + 1));
    }

    // جدا کردن رشته بر اساس یک جداکننده
    inline std::vector<std::string> split(const std::string& s, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(trim(token));
        }
        return tokens;
    }

    // تبدیل نام رجیستر به عدد (مثال: x5 -> 5)
    int register_to_int(const std::string& reg);

    // تبدیل رشته عددی (شامل هگز و دسیمال) به عدد صحیح
    int32_t string_to_int(const std::string& s);
}


#endif //ARCH_PROJECT_COMMON_H
