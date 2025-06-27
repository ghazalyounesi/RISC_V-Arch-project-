//
// Created by ghazal on 6/19/25.
//


// cpu.cpp
/*
#include "cpu.h"
#include <chrono>
#include <thread>
const uint32_t PROGRAM_START_ADDR = 0x1000;

CPU::CPU() : pc(PROGRAM_START_ADDR), is_running(false), fetched_instruction(0) {
    // طبق مستندات پروژه، PC در ابتدا روی 0x1000 تنظیم می‌شود.
}

bool CPU::load_program(const std::string& filename) {
    // فراخوانی تابع جدید حافظه با آدرس شروع صحیح
    return memory.load_binary(filename, 0x0000);
}


void CPU::run() {
    is_running = true;
    while (is_running) {
        step();
        // یک تاخیر کوچک برای اینکه UI فرصت به‌روزرسانی داشته باشد
        // سرعت را می‌توانید از UI کنترل کنید
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void CPU::step() {
    if (pc >= MEMORY_SIZE) {
        is_running = false;
        return;
    }
    fetch();
    decode();
    execute();
}

void CPU::stop() {
    is_running = false;
}

CPUState CPU::get_state() const {
    return {pc, fetched_instruction, current_instruction.mnemonic, is_running};
}

void CPU::fetch() {
    fetched_instruction = memory.read_word(pc);
    pc += 4; // PC را برای دستور بعدی آماده می‌کنیم
}

void CPU::decode() {
    current_instruction.decode(fetched_instruction);
}

void CPU::execute() {
    // برای سادگی، از یک نام مستعار استفاده می‌کنیم
    auto& instr = current_instruction;
    uint32_t val1 = reg_file.read(instr.rs1);
    uint32_t val2 = reg_file.read(instr.rs2);

    // اجرای دستور بر اساس نام آن
    // دستورات هایلایت شده در عکس پیاده‌سازی شده‌اند.
    if (instr.mnemonic == "lui") {
        reg_file.write(instr.rd, instr.imm);
    } else if (instr.mnemonic == "auipc") {
        reg_file.write(instr.rd, (pc - 4) + instr.imm);
    } else if (instr.mnemonic == "jal") {
        reg_file.write(instr.rd, pc); // آدرس بازگشت (pc+4) در rd ذخیره می‌شود
        pc = (pc - 4) + instr.imm; // پرش به آدرس جدید
    } else if (instr.mnemonic == "jalr") {
        uint32_t next_pc = pc;
        pc = (val1 + instr.imm) & ~1U; // پرش به آدرس جدید (مطمئن می‌شویم زوج است)
        reg_file.write(instr.rd, next_pc);
    }
        // Branch Instructions
    else if (instr.mnemonic == "beq" && val1 == val2) { pc = (pc - 4) + instr.imm; }
    else if (instr.mnemonic == "bne" && val1 != val2) { pc = (pc - 4) + instr.imm; }
    else if (instr.mnemonic == "blt" && static_cast<int32_t>(val1) < static_cast<int32_t>(val2)) { pc = (pc - 4) + instr.imm; }
    else if (instr.mnemonic == "bge" && static_cast<int32_t>(val1) >= static_cast<int32_t>(val2)) { pc = (pc - 4) + instr.imm; }
    else if (instr.mnemonic == "bltu" && val1 < val2) { pc = (pc - 4) + instr.imm; }
    else if (instr.mnemonic == "bgeu" && val1 >= val2) { pc = (pc - 4) + instr.imm; }
        // Load Instructions
    else if (instr.mnemonic == "lb") { reg_file.write(instr.rd, static_cast<int8_t>(memory.read_byte(val1 + instr.imm))); }
    else if (instr.mnemonic == "lhu") { reg_file.write(instr.rd, memory.read_half(val1 + instr.imm)); }
    else if (instr.mnemonic == "lbu") { reg_file.write(instr.rd, memory.read_byte(val1 + instr.imm)); }
    else if (instr.mnemonic == "lw") { reg_file.write(instr.rd, memory.read_word(val1 + instr.imm)); }
        // Store Instructions
    else if (instr.mnemonic == "sb") { memory.write_byte(val1 + instr.imm, reg_file.read(instr.rs2)); }
    else if (instr.mnemonic == "sh") { memory.write_half(val1 + instr.imm, reg_file.read(instr.rs2)); }
    else if (instr.mnemonic == "sw") { memory.write_word(val1 + instr.imm, reg_file.read(instr.rs2)); }
        // Arithmetic-Immediate Instructions
    else if (instr.mnemonic == "addi") { reg_file.write(instr.rd, val1 + instr.imm); }
    else if (instr.mnemonic == "slti") { reg_file.write(instr.rd, (static_cast<int32_t>(val1) < instr.imm) ? 1 : 0); }
    else if (instr.mnemonic == "sltiu") { reg_file.write(instr.rd, (val1 < static_cast<uint32_t>(instr.imm)) ? 1 : 0); }
    else if (instr.mnemonic == "xori") { reg_file.write(instr.rd, val1 ^ instr.imm); }
    else if (instr.mnemonic == "ori") { reg_file.write(instr.rd, val1 | instr.imm); }
    else if (instr.mnemonic == "andi") { reg_file.write(instr.rd, val1 & instr.imm); }
    else if (instr.mnemonic == "slli") { reg_file.write(instr.rd, val1 << (instr.imm & 0x1F)); }
    else if (instr.mnemonic == "srli_srai") {
        if (instr.funct7 == 0b0100000) { // SRAI
            reg_file.write(instr.rd, static_cast<int32_t>(val1) >> (instr.imm & 0x1F));
        } else { // SRLI
            reg_file.write(instr.rd, val1 >> (instr.imm & 0x1F));
        }
    }
        // R-Type Instructions
    else if (instr.mnemonic == "add") { reg_file.write(instr.rd, val1 + val2); }
    else if (instr.mnemonic == "sub") { reg_file.write(instr.rd, val1 - val2); }
    else if (instr.mnemonic == "sll") { reg_file.write(instr.rd, val1 << (val2 & 0x1F)); }
    else if (instr.mnemonic == "slt") { reg_file.write(instr.rd, (static_cast<int32_t>(val1) < static_cast<int32_t>(val2)) ? 1 : 0); }
    else if (instr.mnemonic == "sltu") { reg_file.write(instr.rd, (val1 < val2) ? 1 : 0); }
    else if (instr.mnemonic == "xor") { reg_file.write(instr.rd, val1 ^ val2); }
    else if (instr.mnemonic == "srl") { reg_file.write(instr.rd, val1 >> (val2 & 0x1F)); }
    else if (instr.mnemonic == "sra") { reg_file.write(instr.rd, static_cast<int32_t>(val1) >> (val2 & 0x1F)); }
    else if (instr.mnemonic == "or") { reg_file.write(instr.rd, val1 | val2); }
    else if (instr.mnemonic == "and") { reg_file.write(instr.rd, val1 & val2); }
    else {
        // دستور ناشناخته یا پیاده‌سازی نشده
        is_running = false;
        // می‌توانید یک exception پرتاب کنید
    }
}
*/

#include "cpu.h"
#include <sstream> // for std::stringstream
#include <iomanip> // for std::hex

const uint32_t PROGRAM_START_ADDR = 0x1000;

CPU::CPU() {
    reset();
}

void CPU::reset() {
    pc = PROGRAM_START_ADDR;
    halted = false;
    current_stage = PipelineStage::IDLE;
    reg_file = RegisterFile();
    memory = Memory();
    fetched_instruction_raw = 0;
    reg_A_val = 0;
    reg_B_val = 0;
    alu_output = 0;
    memory_data = 0;
    current_instruction = Instruction();
}

bool CPU::load_program(const std::string& filename) {
    reset();
    bool success = memory.load_binary(filename, 0x0000);
    if (success) {
        current_stage = PipelineStage::FETCH;
    } else {
        halted = true;
    }
    return success;
}

bool CPU::is_halted() const {
    return halted;
}

CycleChanges CPU::clock_tick() {
    if (halted) {
        return { "CPU Halted", PipelineStage::IDLE, {}, {}, {}, pc, true };
    }

    if (pc >= MEMORY_SIZE) {
        halted = true;
        return { "PC out of bounds. Halting.", PipelineStage::IDLE, {}, {}, {}, pc, true };
    }
    
    if (current_stage == PipelineStage::IDLE) {
        current_stage = PipelineStage::FETCH;
    }

    switch (current_stage) {
        case PipelineStage::FETCH:      return do_fetch();
        case PipelineStage::DECODE:     return do_decode();
        case PipelineStage::EXECUTE:    return do_execute();
        case PipelineStage::MEMORY:     return do_memory();
        case PipelineStage::WRITE_BACK: return do_write_back();
        default:
            halted = true;
            return { "Invalid stage. Halting.", PipelineStage::IDLE, {}, {}, {}, pc, true };
    }
}

CycleChanges CPU::do_fetch() {
    fetched_instruction_raw = memory.read_word(pc);
    
    std::stringstream rtl;
    rtl << "IR <- M[0x" << std::hex << pc << "]";
    
    current_stage = PipelineStage::DECODE;

    CycleChanges changes;
    changes.rtl_description = rtl.str();
    changes.stage = PipelineStage::FETCH;
    changes.current_pc = pc;
    changes.instruction_finished = false;
    return changes;
}

CycleChanges CPU::do_decode() {
    current_instruction.decode(fetched_instruction_raw);
    
    if (current_instruction.type == InstrType::UNKNOWN) {
        halted = true;
        return { "Unknown instruction. Halting.", PipelineStage::DECODE, {}, {}, {}, pc, true};
    }

    reg_A_val = reg_file.read(current_instruction.rs1);
    reg_B_val = reg_file.read(current_instruction.rs2);
    
    pc += 4;
    
    std::stringstream rtl;
    rtl << "Decode: " << current_instruction.mnemonic << "; A<-Reg[" << (int)current_instruction.rs1 
        << "], B<-Reg[" << (int)current_instruction.rs2 << "]; PC<-PC+4";

    current_stage = PipelineStage::EXECUTE;

    CycleChanges changes;
    changes.rtl_description = rtl.str();
    changes.stage = PipelineStage::DECODE;
    changes.current_pc = pc; // مقدار PC بعد از افزایش است
    changes.instruction_finished = false;
    return changes;
}

CycleChanges CPU::do_execute() {
    auto& instr = current_instruction;
    std::stringstream rtl;
    rtl << "Execute: ";

    CycleChanges changes;
    changes.stage = PipelineStage::EXECUTE;
    changes.instruction_finished = false;

    switch (instr.type) {
        case InstrType::R:
            if (instr.mnemonic == "add") { alu_output = reg_A_val + reg_B_val; rtl << "ALU_out <- A + B"; }
            else if (instr.mnemonic == "sub") { alu_output = reg_A_val - reg_B_val; rtl << "ALU_out <- A - B"; }
                // ... (سایر دستورات R-type)
            else { alu_output = 0; rtl << "Unhandled R-type"; }
            current_stage = PipelineStage::WRITE_BACK;
            break;

        case InstrType::I:
            if (instr.mnemonic == "addi") { alu_output = reg_A_val + instr.imm; rtl << "ALU_out <- A + Imm"; }
            else if (instr.mnemonic == "lw" || instr.mnemonic == "lb" || instr.mnemonic == "lh" || instr.mnemonic == "lbu" || instr.mnemonic == "lhu") {
                alu_output = reg_A_val + instr.imm;
                rtl << "ALU_out <- A + Imm (Address Calc)";
                current_stage = PipelineStage::MEMORY;
            }
                // ... (سایر دستورات I-type)
            else { alu_output = 0; rtl << "Unhandled I-type"; }
            if (current_stage != PipelineStage::MEMORY) current_stage = PipelineStage::WRITE_BACK;
            break;

        case InstrType::S:
            alu_output = reg_A_val + instr.imm;
            rtl << "ALU_out <- A + Imm (Address Calc)";
            current_stage = PipelineStage::MEMORY;
            break;

        case InstrType::B:
        { // <-- شروع بلوک برای محدود کردن حوزه متغیر
            bool condition = false; // حالا 'condition' فقط در این بلوک وجود دارد
            if (instr.mnemonic == "beq" && reg_A_val == reg_B_val) condition = true;
            else if (instr.mnemonic == "bne" && reg_A_val != reg_B_val) condition = true;
            else if (instr.mnemonic == "blt" && static_cast<int32_t>(reg_A_val) < static_cast<int32_t>(reg_B_val)) condition = true;
            else if (instr.mnemonic == "bge" && static_cast<int32_t>(reg_A_val) >= static_cast<int32_t>(reg_B_val)) condition = true;
            else if (instr.mnemonic == "bltu" && reg_A_val < reg_B_val) condition = true;
            else if (instr.mnemonic == "bgeu" && reg_A_val >= reg_B_val) condition = true;

            rtl << "IF (condition) then branch";
            if (condition) {
                pc = (pc - 4) + instr.imm; // PC قبلا +4 شده، پس برای محاسبه از مبدا درست، آن را برمیگردانیم
            }
            rtl << "; New PC = 0x" << std::hex << pc;

            current_stage = PipelineStage::FETCH;
            changes.instruction_finished = true;
            changes.rtl_description = rtl.str();
            changes.current_pc = pc;
            return changes; // این case مستقیما برمی‌گردد
        } // <-- پایان بلوک

        case InstrType::U:
            if (instr.mnemonic == "lui") { alu_output = instr.imm; rtl << "ALU_out <- imm"; }
            else if (instr.mnemonic == "auipc") { alu_output = (pc - 4) + instr.imm; rtl << "ALU_out <- PC_fetch + imm"; }
            current_stage = PipelineStage::WRITE_BACK;
            break;

        case InstrType::J: // JAL
            alu_output = pc; // آدرس بازگشت (PC در این لحظه PC+4 است)
            pc = (pc - 4) + instr.imm;
            rtl << "Reg[rd] <- PC+4; PC <- PC_fetch + imm. New PC = 0x" << std::hex << pc;
            current_stage = PipelineStage::WRITE_BACK;
            break;

        default:
            halted = true;
            changes.rtl_description = "Unknown instruction type in Execute. Halting.";
            changes.instruction_finished = true;
            changes.current_pc = pc;
            return changes;
    }

    changes.rtl_description = rtl.str();
    changes.current_pc = pc;
    return changes;
}
CycleChanges CPU::do_memory() {
    auto& instr = current_instruction;
    std::stringstream rtl;

    CycleChanges changes;
    changes.stage = PipelineStage::MEMORY;
    changes.current_pc = pc;

    if (instr.type == InstrType::S) { // Store
        rtl << "M[0x" << std::hex << alu_output << "] <- B (value: 0x" << reg_B_val << ")";
        memory.write_word(alu_output, reg_B_val); // برای سادگی فقط sw
        changes.memory_write = {{alu_output, reg_B_val}};
        current_stage = PipelineStage::FETCH;
        changes.instruction_finished = true;
    } 
    else if (instr.type == InstrType::I) { // Load
        memory_data = memory.read_word(alu_output); // برای سادگی فقط lw
        rtl << "MDR <- M[0x" << std::hex << alu_output << "] (value: 0x" << memory_data << ")";
        changes.memory_read_addr = alu_output;
        current_stage = PipelineStage::WRITE_BACK;
        changes.instruction_finished = false;
    }
    changes.rtl_description = rtl.str();
    return changes;
}

CycleChanges CPU::do_write_back() {
    auto& instr = current_instruction;
    uint32_t data_to_write = 0;
    std::stringstream rtl;
    
    if (instr.type == InstrType::R || instr.mnemonic == "addi" || instr.type == InstrType::U) {
        data_to_write = alu_output;
        rtl << "Reg[" << (int)instr.rd << "] <- ALU_out (0x" << std::hex << data_to_write << ")";
    } else if (instr.type == InstrType::I && instr.mnemonic != "addi") { // Load
        data_to_write = memory_data;
        rtl << "Reg[" << (int)instr.rd << "] <- MDR (0x" << std::hex << data_to_write << ")";
    } else if (instr.type == InstrType::J) { // jal
        data_to_write = alu_output;
        rtl << "Reg[" << (int)instr.rd << "] <- PC+4 (0x" << std::hex << data_to_write << ")";
    }

    CycleChanges changes;
    changes.stage = PipelineStage::WRITE_BACK;
    changes.current_pc = pc;
    changes.instruction_finished = true; // این آخرین مرحله است
    
    if (instr.rd != 0) {
        reg_file.write(instr.rd, data_to_write);
        changes.register_write = {{instr.rd, data_to_write}};
    } else {
        rtl << " (write to x0 ignored)";
    }
    
    changes.rtl_description = rtl.str();
    current_stage = PipelineStage::FETCH;
    return changes;
}