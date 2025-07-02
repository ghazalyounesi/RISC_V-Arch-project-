#ifndef PSEUDO_H
#define PSEUDO_H


// pseudo.h
#pragma once
#include "common.h"

class Pseudo {
public:
    Pseudo();
    bool is_pseudo(const std::string& instruction) const;
    int get_instruction_size(const std::string& instruction, const std::vector<std::string>& operands);
    std::vector<std::string> expand(const std::string& instruction, const std::vector<std::string>& operands);

private:
    std::unordered_map<std::string, int> pseudo_map;
};


#endif //PSEUDO_H
