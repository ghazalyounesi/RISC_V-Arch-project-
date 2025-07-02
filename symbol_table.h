
#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H


// symbol_table.h
#pragma once
#include "common.h"

class SymbolTable {
public:
    void add_label(const std::string& label, uint32_t address);
    bool has_label(const std::string& label) const;
    uint32_t get_address(const std::string& label) const;
    void clear();
    const std::unordered_map<std::string, uint32_t>& get_all_symbols() const;
private:
    std::unordered_map<std::string, uint32_t> table;
};


#endif //ARCH_PROJECT_SYMBOL_TABLE_H
#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

class symbol_table
{
public:
    symbol_table();
};

#endif // SYMBOL_TABLE_H
