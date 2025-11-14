#pragma once
#include "registers.h"
#include "memory_controller.h"
#include "alu.h"

#include "./instruction_context.h"

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
    struct InstrView{
        std::vector<std::unique_ptr<const InstrContext>> pipeline;
        std::vector<std::unique_ptr<const InstrContext>> reservation_station_alu;
        std::vector<std::unique_ptr<const InstrContext>> reservation_station_falu;
        std::vector<std::unique_ptr<const InstrContext>> reservation_station_lsu;
        std::vector<std::unique_ptr<const InstrContext>> reorder_buffer;
        std::vector<bool> rob_status;
        std::pair<size_t, size_t> rob_head_tail;
    };

    struct Stats{
        size_t cycles;
        size_t instrs_retired;
        size_t branch_instrs;
        size_t branch_mispredicts;
    };


    VmBase() = default;
    virtual ~VmBase() = default;

    virtual void Reset() = 0;

    virtual void LoadVM() = 0;
    virtual void LoadVM(AssembledProgram program) = 0;

    virtual void Run() = 0;
    virtual void DebugRun() = 0;
    virtual void Step() = 0;
    virtual void Undo() = 0;

    virtual uint64_t ReadMemDoubleWord(uint64_t address) = 0;

    virtual const std::array<uint64_t, 32>& GetGprValues() = 0;
    virtual const std::array<uint64_t, 32>& GetFprValues() = 0;

    virtual std::vector<uint64_t> GetInstructionPCs() = 0;
    virtual InstrView GetInstructions() = 0;

    virtual Stats& GetStats() = 0;
};
