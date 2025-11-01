#pragma once

#include "./core/core.h"
#include "./stages/stages.h"
#include "./executor/executor.h"

namespace rv5s{

class VM{
public:
    VM() = default;

    void Reset();

    void LoadVM(AssembledProgram program);

    void RequestStop();

    void Run();

    void DebugRun();

    void Step();

    void Undo();

    bool PipeliningEnabled();

    const std::array<uint64_t, 32>& GetGprValues();
    const std::array<uint64_t, 32>& GetFprValues();

    std::vector<uint64_t> GetInstructionPCs();

    AssembledProgram program_;

private:
    Core vm_core_;
};


} // rv5s