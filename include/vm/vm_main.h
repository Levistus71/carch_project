#pragma once

#include "vm_base.h"

class VM{
public:
    VM();

    void Reset();

    void LoadVM();
    void LoadVM(AssembledProgram program);

    void Run();
    void DebugRun();

    void Step();

    void Undo();

    uint64_t ReadMemDoubleWord(uint64_t address);

    const std::array<uint64_t, 32>& GetGprValues();

    const std::array<uint64_t, 32>& GetFprValues();

    std::vector<uint64_t> GetInstructionPCs();

    std::vector<std::unique_ptr<const InstrContext>> GetInstructions();

    bool PipeliningEnabled();
    bool ForwardingEnabled();
    bool HazardEnabled();

    AssembledProgram program_;

private:
    std::unique_ptr<VmBase> vm_;
};