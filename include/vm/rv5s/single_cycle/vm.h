#pragma once

#include "./core/core.h"
#include "./stages/stages.h"
#include "./executor/executor.h"
#include "vm/vm_base.h"

namespace rv5s{

class SingleCycleVM : public VmBase{
public:
    SingleCycleVM();

    void Reset() override;

    void LoadVM() override;
    void LoadVM(AssembledProgram program) override;

    void Run() override;

    void DebugRun() override;

    void Step() override;

    void Undo() override;

    uint64_t ReadMemDoubleWord(uint64_t address) override;

    const std::array<uint64_t, 32>& GetGprValues() override;
    const std::array<uint64_t, 32>& GetFprValues() override;

    std::vector<uint64_t> GetInstructionPCs() override;
    InstrView GetInstructions() override;

    VmBase::Stats& GetStats() override;

    AssembledProgram program_;

private:
    SingleCycleCore vm_core_;
};


} // rv5s