#pragma once
#include "../core/core.h"

namespace rv5s{

class SingleCycleStages{
public:
    static void Fetch(SingleCycleCore& vm_core);

    static void Decode(SingleCycleCore& vm_core);

    static void Execute(SingleCycleCore& vm_core);

    static void MemoryAccess(SingleCycleCore& vm_core);

    static void WriteBack(SingleCycleCore& vm_core);


private:
    // FIXME: this doesn't belong here
    static void HandleSyscall(SingleCycleCore& vm_core);

    static void ResolveBranch(SingleCycleCore& vm_core);
    static void ExecuteBasic(SingleCycleCore& vm_core);
    static void ExecuteFloat(SingleCycleCore& vm_core);
    static void ExecuteDouble(SingleCycleCore& vm_core);

    static void WriteBackCsr(SingleCycleCore& vm_core);
};


} // namespace rv5s