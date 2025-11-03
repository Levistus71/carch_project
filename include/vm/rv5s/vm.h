#pragma once

#include "./core/core.h"
#include "./stages/stages.h"
#include "./executor/executor.h"

namespace rv5s{

class VM{
public:
    VM();

    void Reset();

    void LoadVM(AssembledProgram program);
    void LoadVM();

    void RequestStop();

    void Run();

    void DebugRun();

    void Step();

    void Undo();

    bool PipeliningEnabled();
    bool HazardEnabled();
    bool ForwardingEnabled();

    uint64_t ReadMemDoubleWord(uint64_t address);

    const std::array<uint64_t, 32>& GetGprValues();
    const std::array<uint64_t, 32>& GetFprValues();

    std::vector<uint64_t> GetInstructionPCs();
    std::vector<std::reference_wrapper<const InstrContext>> GetInstructions();

    AssembledProgram program_;

private:
    Core vm_core_;
};


} // rv5s