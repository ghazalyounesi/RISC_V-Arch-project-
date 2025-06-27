//
// Created by ghazal on 6/19/25.
//

#ifndef ARCH_CPU_H
#define ARCH_CPU_H


// cpu.h
/*
#pragma once
#include "comm.h"
#include "register_file.h"
#include "memory.h"
#include "instruction.h"

// این ساختار برای ارسال وضعیت فعلی CPU به UI عالی است
struct CPUState {
    uint32_t pc;
    uint32_t instruction;
    std::string mnemonic;
    bool is_running;
    // می‌توانید فیلدهای دیگری مثل آخرین رجیستر تغییرکرده را هم اضافه کنید
};

class CPU {
public:
    CPU();

    // بارگذاری برنامه از فایل .bin
    bool load_program(const std::string& filename);

    // توابع کنترل اجرا که به دکمه‌های UI متصل می‌شوند
    void run();       // اجرای کامل و بدون توقف
    void step();      // اجرای فقط یک دستور
    void stop();      // متوقف کردن اجرای run()

    // توابع Getter برای دسترسی UI به وضعیت داخلی
    CPUState get_state() const;
    const RegisterFile& get_registers() const { return reg_file; }
    const Memory& get_memory() const { return memory; }

private:
    void fetch();
    void decode();
    void execute();

    uint32_t pc;
    bool is_running;

    RegisterFile reg_file;
    Memory memory;

    uint32_t fetched_instruction; // دستور خام واکشی شده
    Instruction current_instruction; // دستور دیکود شده
};
*/


#pragma once
#include "comm.h"
#include "register_file.h"
#include "memory.h"
#include "instruction.h"
#include <optional> // برای مقادیر آپشنال

// وضعیت‌های ممکن در پایپ‌لاین چند چرخه‌ای ما
enum class PipelineStage {
    IDLE,
    FETCH,
    DECODE,
    EXECUTE,
    MEMORY,
    WRITE_BACK
};

// این ساختار تمام اطلاعات لازم برای آپدیت UI در هر کلاک را حمل می‌کند
struct CycleChanges {
    std::string rtl_description;
    PipelineStage stage;

    // اطلاعات تغییرات ثبات
    std::optional<std::pair<uint8_t, uint32_t>> register_write; // <index, new_value>
    
    // اطلاعات تغییرات حافظه
    std::optional<std::pair<uint32_t, uint32_t>> memory_write;   // <address, new_value>
    std::optional<uint32_t> memory_read_addr;
    
    // وضعیت PC
    uint32_t current_pc;
    bool instruction_finished; // آیا این کلاک، آخرین کلاک یک دستور بود؟
};

class CPU {
public:
    CPU();

    // بارگذاری برنامه از فایل .bin
    bool load_program(const std::string& filename);

    // تابع اصلی جدید: اجرای یک پالس ساعت
    CycleChanges clock_tick();

    // ریست کردن وضعیت پردازنده
    void reset();

    // بررسی اینکه آیا برنامه به پایان رسیده
    bool is_halted() const;

    // توابع Getter برای دسترسی UI به وضعیت داخلی
    const RegisterFile& get_registers() const { return reg_file; }
    const Memory& get_memory() const { return memory; }
    const Instruction& get_current_instruction() const { return current_instruction; }
    uint32_t get_pc() const { return pc; }
    PipelineStage get_current_stage() const { return current_stage; }


private:
    // توابع داخلی برای هر مرحله
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

    // ثبات‌های داخلی برای نگهداری مقادیر بین مراحل
    uint32_t fetched_instruction_raw;
    Instruction current_instruction;
    uint32_t reg_A_val, reg_B_val; // مقادیر خوانده شده از rs1, rs2
    uint32_t alu_output;
    uint32_t memory_data;
};
#endif //ARCH_CPU_H
