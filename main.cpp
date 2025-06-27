#include "parser.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <thread>   // for sleep
#include <chrono>   // for milliseconds
#include "cpu.h"


// یک تابع کمکی برای نمایش وضعیت فعلی
void print_cycle_state(const CycleChanges& changes, const CPU& cpu) {
    std::cout << "===========================================================\n"
              << "PC: 0x" << std::hex << std::setfill('0') << std::setw(8) << changes.current_pc
              << " | Stage: " << static_cast<int>(changes.stage) // نام استیج را می‌توانید با یک تابع کمکی زیبا کنید
              << " | Instruction: " << cpu.get_current_instruction().mnemonic << "\n"
              << "RTL: " << changes.rtl_description << "\n";

    if (changes.register_write) {
        std::cout << ">> REGISTER WRITE: x" << (int)changes.register_write->first 
                  << " = 0x" << std::hex << changes.register_write->second << std::dec << "\n";
    }
    if (changes.memory_write) {
        std::cout << ">> MEMORY WRITE: M[0x" << std::hex << changes.memory_write->first 
                  << "] = 0x" << changes.memory_write->second << std::dec << "\n";
    }
    if (changes.memory_read_addr) {
        std::cout << ">> MEMORY READ: Addr=0x" << std::hex << *changes.memory_read_addr << std::dec << "\n";
    }
    
    if (changes.instruction_finished) {
        std::cout << "--- Instruction Finished ---\n";
        cpu.get_registers().dump(); // نمایش ثبات‌ها در پایان هر دستور
    }
    std::cout << "===========================================================" << std::endl;
}

void show_menu() {
    std::cout << "\n===== RISC-V Clock-by-Clock Simulator =====\n"
              << "(c)lock: Execute one clock cycle\n"
              << "(r)un:   Run automatically with delay\n"
              << "(d)ump:  Dump register values now\n"
              << "(m)em:   Inspect a memory region\n"
              << "(q)uit:  Exit the simulator\n"
              << "Enter command: ";
}

// تابع کمکی برای خواندن کل محتوای یک فایل به درون یک رشته
std::string read_file_into_string(const std::string& path) {
    std::ifstream input_file(path);
    if (!input_file.is_open()) {
        throw std::runtime_error("Could not open file: " + path);
    }
    std::stringstream buffer;
    buffer << input_file.rdbuf();
    return buffer.str();
}

int main() {
    // نام فایل اسمبلی ورودی خود را اینجا مشخص کنید
    std::string input_filename = "/home/ghazal/CLionProjects/Arch/input.s";
    std::string output_filename = "/home/ghazal/CLionProjects/Arch/output.bin";
    std::cout << "--- Starting Assembler Test ---" << std::endl;

    try {
        // 1. خواندن کل کد اسمبلی از فایل
        std::string assembly_code = read_file_into_string(input_filename);
        std::cout << "Successfully read '" << input_filename << "'." << std::endl << std::endl;

        // 2. ساختن یک نمونه از موتور اسمبلر
        Parser assembler;

        // 3. اجرای Pass 1
        std::cout << "--- Running First Pass ---" << std::endl;
        FirstPassResult first_result = assembler.runFirstPass(assembly_code);

        // 4. نمایش نتایج Pass 1
        std::cout << first_result.symbol_table_output << std::endl;

        if (!first_result.warnings.empty()) {
            std::cerr << "--- Warnings/Errors Found ---" << std::endl;
            for (const auto& warning : first_result.warnings) {
                // چاپ هشدارها در خروجی خطا (قرمز رنگ در بسیاری از ترمینال‌ها)
                std::cerr << "Warning: " << warning << std::endl;
            }
            std::cerr << "---------------------------\n" << std::endl;
        }

        // اگر خطای مرگباری در pass 1 رخ داده باشد، ادامه نده
        if (!first_result.success) {
            std::cerr << "Assembly process halted due to fatal errors in the first pass." << std::endl;
            return 1;
        }

        // 5. اجرای Pass 2
        std::cout << "--- Running Second Pass ---" << std::endl;
        SecondPassResult second_result = assembler.runSecondPass(assembly_code);
        
        // 6. نمایش نتایج Pass 2
        std::cout << "Binary output (Hex representation):" << std::endl;
        std::cout << second_result.binary_hex_output << std::endl;
        // ====================================================================
        // بخش جدید: ذخیره کردن خروجی باینری در فایل
        // ====================================================================
        std::ofstream output_file(output_filename, std::ios::binary);
        if (!output_file) {
            std::cerr << "Error: Could not open " << output_filename << " for writing!" << std::endl;
        } else {
            // نوشتن داده‌های باینری خام (که در result.binary_data ذخیره شده) در فایل
            output_file.write(reinterpret_cast<const char*>(second_result.binary_data.data()), second_result.binary_data.size());
            output_file.close();
            std::cout << "\nBinary output successfully saved to '" << output_filename << "'" << std::endl;
        }

        std::cout << "\n--- Test Finished Successfully ---" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return 1;
    }
    /*______________________________________________________________________________________________________________________*/
    std::string filename = "/home/ghazal/CLionProjects/Arch/output.bin"; // مسیر فایل باینری خودتان
    CPU cpu;

    if (!cpu.load_program(filename)) {
        std::cerr << "Failed to load program. Exiting." << std::endl;
        return 1;
    }

    std::cout << "Program loaded. Initial PC: 0x" << std::hex << cpu.get_pc() << std::dec << std::endl;

    char command;
    while (true) {
        show_menu();
        std::cin >> command;

        if (cpu.is_halted()) {
            std::cout << "CPU is halted. Cannot execute further." << std::endl;
            if (command != 'q') continue;
        }

        switch (command) {
            case 'c': { // Clock Tick
                CycleChanges changes = cpu.clock_tick();
                print_cycle_state(changes, cpu);
                break;
            }

            case 'r': { // Run
                std::cout << "\nRunning program automatically (500ms delay)... Press any key to stop.\n";
                while (!cpu.is_halted()) {
                    CycleChanges changes = cpu.clock_tick();
                    print_cycle_state(changes, cpu);
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    // این بخش برای توقف فوری است، در یک UI واقعی بهتر پیاده‌سازی می‌شود.
                }
                std::cout << "Run finished. CPU is halted." << std::endl;
                break;
            }
            
            case 'd': // Dump Registers
                std::cout << "\n" << std::endl;
                cpu.get_registers().dump();
                break;

            case 'm': { // Memory Inspect
                uint32_t address;
                uint32_t length;
                std::cout << "Enter start address (in hex): 0x";
                std::cin >> std::hex >> address;
                std::cout << "Enter length (in bytes): ";
                std::cin >> std::dec >> length;
                std::cout << "\n" << std::endl;
                cpu.get_memory().dump(address, length);
                break;
            }

            case 'q': // Quit
                std::cout << "Exiting simulator. Goodbye!" << std::endl;
                return 0;

            default:
                std::cout << "Unknown command. Please try again." << std::endl;
                break;
        }
    }

    return 0;

    
}