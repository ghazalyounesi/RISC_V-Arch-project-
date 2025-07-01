//
// Created by ghazal on 6/19/25.
//


// cpu.cpp

#include "cpu.h"
#include <sstream>
#include <iomanip>
#include <cstdint>

// آدرس شروع برنامه
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
    // در شبیه‌ساز ما، برنامه از آدرس 0x0000 بارگذاری می‌شود اما اجرا از 0x1000 شروع می‌شود.
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
    rtl << "T0: IR <- M[0x" << std::hex << std::setfill('0') << std::setw(4) << pc << "]";

    current_stage = PipelineStage::DECODE;

    CycleChanges changes;
    changes.rtl_description = rtl.str();
    changes.stage = PipelineStage::FETCH;
    changes.current_pc = pc;
    changes.instruction_finished = false;
    return changes;
}

CycleChanges CPU::do_decode() {
    uint32_t pc_before_inc = pc;
    current_instruction.decode(fetched_instruction_raw);

    if (current_instruction.type == InstrType::UNKNOWN) {
        halted = true;
        std::stringstream err;
        err << "Unknown instruction code: 0x" << std::hex << fetched_instruction_raw << ". Halting.";
        return { err.str(), PipelineStage::DECODE, {}, {}, {}, pc, true};
    }

    // خواندن رجیسترها حتی اگر استفاده نشوند
    reg_A_val = reg_file.read(current_instruction.rs1);
    reg_B_val = reg_file.read(current_instruction.rs2);

    std::stringstream rtl;

    // منطق به‌روزرسانی PC بر اساس نوع دستور
    // برای دستورات انشعابی (Branch) و JAL/JALR، مقدار PC در مرحله Execute مشخص می‌شود.
    if (current_instruction.type != InstrType::B && current_instruction.type != InstrType::J && current_instruction.mnemonic != "jalr") {
        pc += 4;
        rtl << "T1: PC <- PC + 4 (0x" << std::hex << pc << ")";
    } else {
        rtl << "T1: Decode " << current_instruction.mnemonic << ", PC update deferred";
    }

    current_stage = PipelineStage::EXECUTE;

    CycleChanges changes;
    changes.rtl_description = rtl.str();
    changes.stage = PipelineStage::DECODE;
    changes.current_pc = pc_before_inc;
    changes.instruction_finished = false;
    return changes;
}

// [اصلاح اساسی] این تابع اکنون تمام دستورات را پوشش می‌دهد
CycleChanges CPU::do_execute() {
    auto& instr = current_instruction;
    std::stringstream rtl;
    // PC در زمان fetch این دستور. برای branch/jump ها PC هنوز آپدیت نشده
    uint32_t pc_at_fetch = (instr.type == InstrType::B || instr.type == InstrType::J || instr.mnemonic == "jalr") ? pc : pc - 4;

    CycleChanges changes;
    changes.stage = PipelineStage::EXECUTE;
    changes.instruction_finished = false;
    bool write_needed = false;

    // توابع کمکی برای فرمت‌دهی خواناتر RTL
    auto format_rtl_reg = [&](const std::string& op, uint32_t valA, uint32_t valB) {
        rtl << "x" << (int)instr.rd << " <- x" << (int)instr.rs1 << " " << op << " x" << (int)instr.rs2
            << "  (0x" << std::hex << alu_output << " <- 0x" << valA << " " << op << " 0x" << valB << ")";
    };
    auto format_rtl_imm = [&](const std::string& op) {
        rtl << "x" << (int)instr.rd << " <- x" << (int)instr.rs1 << " " << op << " imm"
            << "  (0x" << std::hex << alu_output << " <- 0x" << reg_A_val << " " << op << " 0x" << instr.imm << ")";
    };

    // دستورات محاسباتی، منطقی و پرش‌های غیرشرطی
    if (instr.type == InstrType::R || instr.type == InstrType::I_compute || instr.type == InstrType::U || instr.type == InstrType::J) {
        rtl << "T2: ";
        write_needed = true; // اکثر این دستورات در رجیستر می‌نویسند

        // --- U-Type ---
        if (instr.mnemonic == "lui") { alu_output = instr.imm; rtl << "x" << (int)instr.rd << " <- imm (0x" << std::hex << alu_output << ")"; }
        else if (instr.mnemonic == "auipc") { alu_output = pc_at_fetch + instr.imm; rtl << "x" << (int)instr.rd << " <- PC + imm (0x" << std::hex << pc_at_fetch << " + 0x" << instr.imm << " = 0x" << alu_output << ")";}
            // --- J-Type ---
        else if (instr.mnemonic == "jal") { alu_output = pc_at_fetch + 4; pc = pc_at_fetch + instr.imm; rtl << "Reg[rd] <- PC+4, PC <- PC+imm (x" << (int)instr.rd << "=0x" << std::hex << alu_output << ", PC=0x" << pc << ")"; }
            // --- R-Type ---
        else if (instr.mnemonic == "add") { alu_output = reg_A_val + reg_B_val; format_rtl_reg("+", reg_A_val, reg_B_val); }
        else if (instr.mnemonic == "sub") { alu_output = reg_A_val - reg_B_val; format_rtl_reg("-", reg_A_val, reg_B_val); }
        else if (instr.mnemonic == "sll") { alu_output = reg_A_val << (reg_B_val & 0x1F); format_rtl_reg("<<", reg_A_val, reg_B_val); }
        else if (instr.mnemonic == "slt") { alu_output = (static_cast<int32_t>(reg_A_val) < static_cast<int32_t>(reg_B_val)) ? 1 : 0; format_rtl_reg("< (s)", reg_A_val, reg_B_val); }
        else if (instr.mnemonic == "sltu") { alu_output = (reg_A_val < reg_B_val) ? 1 : 0; format_rtl_reg("< (u)", reg_A_val, reg_B_val); }
        else if (instr.mnemonic == "xor") { alu_output = reg_A_val ^ reg_B_val; format_rtl_reg("^", reg_A_val, reg_B_val); }
        else if (instr.mnemonic == "srl") { alu_output = reg_A_val >> (reg_B_val & 0x1F); format_rtl_reg(">> (l)", reg_A_val, reg_B_val); }
        else if (instr.mnemonic == "sra") { alu_output = static_cast<int32_t>(reg_A_val) >> (reg_B_val & 0x1F); format_rtl_reg(">> (a)", reg_A_val, reg_B_val); }
        else if (instr.mnemonic == "or") { alu_output = reg_A_val | reg_B_val; format_rtl_reg("|", reg_A_val, reg_B_val); }
        else if (instr.mnemonic == "and") { alu_output = reg_A_val & reg_B_val; format_rtl_reg("&", reg_A_val, reg_B_val); }
            // --- M-Extension (R-Type) ---
        else if (instr.mnemonic == "mul") { alu_output = reg_A_val * reg_B_val; format_rtl_reg("*", reg_A_val, reg_B_val); }
        else if (instr.mnemonic == "mulh") { int64_t res = static_cast<int64_t>(static_cast<int32_t>(reg_A_val)) * static_cast<int64_t>(static_cast<int32_t>(reg_B_val)); alu_output = res >> 32; format_rtl_reg("*h (s)", reg_A_val, reg_B_val); }
        else if (instr.mnemonic == "mulhsu") { int64_t res = static_cast<int64_t>(static_cast<int32_t>(reg_A_val)) * static_cast<uint64_t>(reg_B_val); alu_output = res >> 32; format_rtl_reg("*h (su)", reg_A_val, reg_B_val); }
        else if (instr.mnemonic == "mulhu") { uint64_t res = static_cast<uint64_t>(reg_A_val) * static_cast<uint64_t>(reg_B_val); alu_output = res >> 32; format_rtl_reg("*h (u)", reg_A_val, reg_B_val); }
        else if (instr.mnemonic == "div") { if (reg_B_val == 0) alu_output = -1; else if (reg_A_val == 0x80000000 && static_cast<int32_t>(reg_B_val) == -1) alu_output = 0x80000000; else alu_output = static_cast<int32_t>(reg_A_val) / static_cast<int32_t>(reg_B_val); format_rtl_reg("/ (s)", reg_A_val, reg_B_val); }
        else if (instr.mnemonic == "divu") { if (reg_B_val == 0) alu_output = 0xFFFFFFFF; else alu_output = reg_A_val / reg_B_val; format_rtl_reg("/ (u)", reg_A_val, reg_B_val); }
        else if (instr.mnemonic == "rem") { if (reg_B_val == 0) alu_output = reg_A_val; else if (reg_A_val == 0x80000000 && static_cast<int32_t>(reg_B_val) == -1) alu_output = 0; else alu_output = static_cast<int32_t>(reg_A_val) % static_cast<int32_t>(reg_B_val); format_rtl_reg("% (s)", reg_A_val, reg_B_val); }
        else if (instr.mnemonic == "remu") { if (reg_B_val == 0) alu_output = reg_A_val; else alu_output = reg_A_val % reg_B_val; format_rtl_reg("% (u)", reg_A_val, reg_B_val); }
            // --- I-Type ---
        else if (instr.mnemonic == "addi") { alu_output = reg_A_val + instr.imm; format_rtl_imm("+"); }
        else if (instr.mnemonic == "slti") { alu_output = (static_cast<int32_t>(reg_A_val) < static_cast<int32_t>(instr.imm)) ? 1 : 0; format_rtl_imm("< (s)"); }
        else if (instr.mnemonic == "sltiu") { alu_output = (reg_A_val < static_cast<uint32_t>(instr.imm)) ? 1 : 0; format_rtl_imm("< (u)"); }
        else if (instr.mnemonic == "xori") { alu_output = reg_A_val ^ instr.imm; format_rtl_imm("^"); }
        else if (instr.mnemonic == "ori") { alu_output = reg_A_val | instr.imm; format_rtl_imm("|"); }
        else if (instr.mnemonic == "andi") { alu_output = reg_A_val & instr.imm; format_rtl_imm("&"); }
        else if (instr.mnemonic == "slli") { alu_output = reg_A_val << (instr.imm & 0x1F); format_rtl_imm("<<"); }
        else if (instr.mnemonic == "srli") { alu_output = reg_A_val >> (instr.imm & 0x1F); format_rtl_imm(">> (l)"); }
        else if (instr.mnemonic == "srai") { alu_output = static_cast<int32_t>(reg_A_val) >> (instr.imm & 0x1F); format_rtl_imm(">> (a)"); }
        else { rtl << "UNKNOWN ALU/JUMP INSTRUCTION: " << instr.mnemonic; write_needed = false; }

        current_stage = PipelineStage::FETCH; // این دستورات در 3 کلاک تمام می‌شوند
        changes.instruction_finished = true;
    }
        // دستورات دسترسی به حافظه (Load)
    else if (instr.type == InstrType::I_load) {
        rtl << "T2: MAR <- x" << (int)instr.rs1 << " + imm";
        alu_output = reg_A_val + instr.imm;
        rtl << "  (0x" << std::hex << alu_output << " <- 0x" << reg_A_val << " + 0x" << instr.imm << ")";
        current_stage = PipelineStage::MEMORY; // نیاز به مرحله حافظه دارد
    }
        // دستورات ذخیره‌سازی (Store)
    else if (instr.type == InstrType::S) {
        rtl << "T2: MAR <- x" << (int)instr.rs1 << " + imm";
        alu_output = reg_A_val + instr.imm;
        rtl << "  (0x" << std::hex << alu_output << " <- 0x" << reg_A_val << " + 0x" << instr.imm << ")";
        current_stage = PipelineStage::MEMORY; // نیاز به مرحله حافظه دارد
    }
        // دستورات انشعاب (Branch)
    else if (instr.type == InstrType::B) {
        rtl << "T2: ";
        bool condition = false;
        if      (instr.mnemonic == "beq")  { condition = (reg_A_val == reg_B_val); rtl << "if (x" << (int)instr.rs1 << " == x" << (int)instr.rs2 << ")";}
        else if (instr.mnemonic == "bne")  { condition = (reg_A_val != reg_B_val); rtl << "if (x" << (int)instr.rs1 << " != x" << (int)instr.rs2 << ")";}
        else if (instr.mnemonic == "blt")  { condition = (static_cast<int32_t>(reg_A_val) < static_cast<int32_t>(reg_B_val)); rtl << "if (x" << (int)instr.rs1 << " < x" << (int)instr.rs2 << " signed)";}
        else if (instr.mnemonic == "bge")  { condition = (static_cast<int32_t>(reg_A_val) >= static_cast<int32_t>(reg_B_val)); rtl << "if (x" << (int)instr.rs1 << " >= x" << (int)instr.rs2 << " signed)";}
        else if (instr.mnemonic == "bltu") { condition = (reg_A_val < reg_B_val); rtl << "if (x" << (int)instr.rs1 << " < x" << (int)instr.rs2 << " unsigned)";}
        else if (instr.mnemonic == "bgeu") { condition = (reg_A_val >= reg_B_val); rtl << "if (x" << (int)instr.rs1 << " >= x" << (int)instr.rs2 << " unsigned)";}

        rtl << " (0x" << std::hex << reg_A_val << " vs 0x" << reg_B_val << ")";

        if (condition) {
            pc = pc_at_fetch + instr.imm;
            rtl << " -> Taken. PC <- PC_fetch + imm (0x" << std::hex << pc << ")";
        } else {
            pc = pc_at_fetch + 4;
            rtl << " -> Not Taken. PC <- PC_fetch + 4 (0x" << std::hex << pc << ")";
        }

        current_stage = PipelineStage::FETCH;
        changes.instruction_finished = true;
    }
        // پرش غیرمستقیم (JALR)
    else if (instr.mnemonic == "jalr") {
        alu_output = pc_at_fetch + 4; // آدرس بازگشت
        pc = (reg_A_val + instr.imm) & ~1U; // آدرس پرش (بیت آخر صفر می‌شود)
        rtl << "T2: Temp <- PC+4; Reg[rd] <- Temp, PC <- (Reg[rs1]+imm)&~1"
            << " (x" << (int)instr.rd << "=0x" << std::hex << alu_output << ", PC=0x" << pc << ")";
        write_needed = true;
        current_stage = PipelineStage::FETCH;
        changes.instruction_finished = true;
    }

    // عمل نوشتن در رجیستر برای تمام دستوراتی که نیاز دارند
    if (write_needed && instr.rd != 0) {
        reg_file.write(instr.rd, alu_output);
        changes.register_write = {{instr.rd, alu_output}};
    } else if (write_needed && instr.rd == 0) {
        rtl << " (write to x0 ignored)";
    }

    changes.rtl_description = rtl.str();
    changes.current_pc = pc_at_fetch;
    return changes;
}

CycleChanges CPU::do_memory() {
    auto& instr = current_instruction;
    std::stringstream rtl;

    CycleChanges changes;
    changes.stage = PipelineStage::MEMORY;
    changes.current_pc = pc - 4; // PC در زمان fetch
    changes.instruction_finished = false;

    // دستورات ذخیره‌سازی (Store)
    if (instr.type == InstrType::S) {
        rtl << "T3: ";
        if (instr.mnemonic == "sb") { memory.write_byte(alu_output, reg_B_val); rtl << "M[0x" << std::hex << alu_output << "] <- x" << (int)instr.rs2 << "[7:0] (val=0x" << (reg_B_val & 0xFF) << ")"; }
        else if (instr.mnemonic == "sh") { memory.write_half(alu_output, reg_B_val); rtl << "M[0x" << std::hex << alu_output << "] <- x" << (int)instr.rs2 << "[15:0] (val=0x" << (reg_B_val & 0xFFFF) << ")"; }
        else if (instr.mnemonic == "sw") { memory.write_word(alu_output, reg_B_val); rtl << "M[0x" << std::hex << alu_output << "] <- x" << (int)instr.rs2 << "[31:0] (val=0x" << reg_B_val << ")"; }

        changes.memory_write = {{alu_output, reg_B_val}};
        current_stage = PipelineStage::FETCH;
        changes.instruction_finished = true; // دستور تمام شد (4 کلاک)
    }
        // دستورات خواندن (Load)
    else if (instr.type == InstrType::I_load) {
        rtl << "T3: MDR <- M[MAR] (M[0x" << std::hex << alu_output << "])";
        memory_data = memory.read_word(alu_output); // خواندن کلمه کامل برای سادگی
        changes.memory_read_addr = alu_output;
        current_stage = PipelineStage::WRITE_BACK; // برای load ها به WB می‌رویم
    }
    changes.rtl_description = rtl.str();
    return changes;
}

CycleChanges CPU::do_write_back() {
    auto& instr = current_instruction;
    uint32_t data_to_write = 0;
    std::stringstream rtl;

    rtl << "T4: ";
    if (instr.mnemonic == "lb") { data_to_write = static_cast<int32_t>(static_cast<int8_t>(memory_data)); rtl << "x" << (int)instr.rd << " <- sign_extend(MDR[7:0])"; }
    else if (instr.mnemonic == "lh") { data_to_write = static_cast<int32_t>(static_cast<int16_t>(memory_data)); rtl << "x" << (int)instr.rd << " <- sign_extend(MDR[15:0])"; }
    else if (instr.mnemonic == "lw") { data_to_write = memory_data; rtl << "x" << (int)instr.rd << " <- MDR[31:0]"; }
    else if (instr.mnemonic == "lbu") { data_to_write = memory_data & 0xFF; rtl << "x" << (int)instr.rd << " <- zero_extend(MDR[7:0])"; }
    else if (instr.mnemonic == "lhu") { data_to_write = memory_data & 0xFFFF; rtl << "x" << (int)instr.rd << " <- zero_extend(MDR[15:0])"; }

    rtl << "  (val=0x" << std::hex << data_to_write << ")";

    CycleChanges changes;
    changes.stage = PipelineStage::WRITE_BACK;
    changes.current_pc = pc - 4; // PC در زمان fetch
    changes.instruction_finished = true; // دستور تمام شد (5 کلاک)

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
