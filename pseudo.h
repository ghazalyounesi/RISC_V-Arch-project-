//
// Created by ghazal on 6/17/25.
//

#ifndef ARCH_PROJECT_PSEUDO_H
#define ARCH_PROJECT_PSEUDO_H


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


#endif //ARCH_PROJECT_PSEUDO_H
