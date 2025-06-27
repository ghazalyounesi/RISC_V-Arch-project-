//
// Created by ghazal on 6/17/25.
//


// symbol_table.cpp
#include "symbol_table.h"

void SymbolTable::add_label(const std::string& label, uint32_t address) {
    if (has_label(label)) {
        throw std::runtime_error("Error: Duplicate label definition '" + label + "'");
    }
    table[label] = address;
}

bool SymbolTable::has_label(const std::string& label) const {
    return table.find(label) != table.end();
}

uint32_t SymbolTable::get_address(const std::string& label) const {
    if (!has_label(label)) {
        throw std::runtime_error("Error: Undefined label '" + label + "'");
    }
    return table.at(label);
}

void SymbolTable::clear() {
    table.clear();
}
//for ui
const std::unordered_map<std::string, uint32_t>& SymbolTable::get_all_symbols() const {
    return table;
}