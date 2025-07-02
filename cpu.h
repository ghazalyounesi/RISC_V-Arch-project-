
#ifndef CPU_H
#define CPU_H



#pragma once
#include "comm.h"
#include "register_file.h"
#include "memory.h"
#include "instruction.h"
#include <optional> 


enum class PipelineStage {
    IDLE,
    FETCH,
    DECODE,
    EXECUTE,
    MEMORY,
    WRITE_BACK
};


struct CycleChanges {
    std::string rtl_description;
    PipelineStage stage;

    std::optional<std::pair<uint8_t, uint32_t>> register_write; 

    std::optional<std::pair<uint32_t, uint32_t>> memory_write;   
    std::optional<uint32_t> memory_read_addr;

    uint32_t current_pc;
    bool instruction_finished; 
};

class CPU {
public:
    CPU();

    bool load_program(const std::string& filename);

    CycleChanges clock_tick();

    void reset();

    bool is_halted() const;

    const RegisterFile& get_registers() const { return reg_file; }
    const Memory& get_memory() const { return memory; }
    const Instruction& get_current_instruction() const { return current_instruction; }
    uint32_t get_pc() const { return pc; }
    PipelineStage get_current_stage() const { return current_stage; }


private:
    
    CycleChanges do_fetch();
    CycleChanges do_decode();
    CycleChanges do_execute();
    CycleChanges do_memory();
    CycleChanges do_write_back();

    uint32_t pc;
    bool halted;

    RegisterFile reg_file;
    Memory memory;

    PipelineStage current_stage;

    
    uint32_t fetched_instruction_raw;
    Instruction current_instruction;
    uint32_t reg_A_val, reg_B_val; 
    uint32_t alu_output;
    uint32_t memory_data;
};
#endif //CPU_H
