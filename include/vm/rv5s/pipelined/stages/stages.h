#pragma once
#include "../core/core.h"

namespace rv5s{

class PipelinedStages{
public:
    static void Fetch(PipelinedCore& vm_core);

    static void Decode(PipelinedCore& vm_core);

    static void Execute(PipelinedCore& vm_core);

    static void MemoryAccess(PipelinedCore& vm_core);

    static void WriteBack(PipelinedCore& vm_core);


private:
    // FIXME: this doesn't belong here
    static void HandleSyscall(PipelinedCore& vm_core);

    static void ResolveBranch(PipelinedCore& vm_core);
    static void ExecuteBasic(PipelinedCore& vm_core);
    static void ExecuteFloat(PipelinedCore& vm_core);
    static void ExecuteDouble(PipelinedCore& vm_core);

    static void WriteBackCsr(PipelinedCore& vm_core);
};


} // namespace rv5s