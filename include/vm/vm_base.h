/**
 * @file vm_base.h
 * @brief File containing the base class for the virtual machine
 * @author Vishank Singh, https://VishankSingh
 */
#ifndef VM_BASE_H
#define VM_BASE_H


#include "registers.h"
#include "memory_controller.h"
#include "alu.h"
#include "globals.h"

#include "vm_asm_mw.h"

#include <vector>
#include <string>
#include <filesystem>
#include <cstdint>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

enum SyscallCode {
    SYSCALL_PRINT_INT = 1,
    SYSCALL_PRINT_FLOAT = 2,
    SYSCALL_PRINT_DOUBLE = 3,
    SYSCALL_PRINT_STRING = 4,
    SYSCALL_EXIT = 10,
    SYSCALL_READ = 63,
    SYSCALL_WRITE = 64,
};


class VmBase {
public:
    VmBase() = default;
    ~VmBase() = default;

    AssembledProgram program_;

    std::vector<uint64_t> breakpoints_;

    uint64_t program_counter_{};
    

    std::string output_status_;

    MemoryController memory_controller_;
    RegisterFile registers_;
    
    alu::Alu alu_;

    void SetProgram(const AssembledProgram& program){
        this->program_ = program;
    }

    virtual void LoadVM() = 0;
    uint64_t program_size_ = 0;

    uint64_t GetProgramCounter() const;
    void AddToProgramCounter(int64_t value);
    void SetProgramCounter(uint64_t value);
    

    void AddBreakpoint(uint64_t val, bool is_line = true);
    void RemoveBreakpoint(uint64_t val, bool is_line = true);
    bool CheckBreakpoint(uint64_t address);

    void PrintString(uint64_t address);

    virtual void Run() = 0;
    virtual void DebugRun() = 0;
    virtual void Step() = 0;
    virtual void Undo() = 0;
    // REDO IS DEPRECATED. why was it there in the first place? just step over bruh
    // virtual void Redo() = 0;
    virtual void Reset() = 0;
    void DumpState(const std::filesystem::path &filename);

    void ModifyRegister(const std::string &reg_name, uint64_t value);

};

#endif // VM_BASE_H
