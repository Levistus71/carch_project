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

    void RequestStop();

    void Run();

    void DebugRun();

    void Step();

    void Undo();

    std::vector<uint64_t> GetInstructionPCs(Core& vm_core);

private:
    Core vm_core_;
	AssembledProgram program_;
};


} // rv5s