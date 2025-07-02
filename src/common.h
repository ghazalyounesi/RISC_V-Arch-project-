#ifndef COMMON_H
#define COMMON_H


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


struct InstructionInfo {
    std::string opcode;
    std::string funct3;
    std::string funct7;
    char type; // R, I, S, B, U, J
};


namespace Utils {

inline std::string trim(const std::string& s) {
    size_t first = s.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) return "";
    size_t last = s.find_last_not_of(" \t\n\r");
    return s.substr(first, (last - first + 1));
}


inline std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(trim(token));
    }
    return tokens;
}


int register_to_int(const std::string& reg);

int32_t string_to_int(const std::string& s);
}


#endif //COMMON_H
