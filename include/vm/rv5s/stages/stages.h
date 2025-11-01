#pragma once
#include "../core/core.h"

namespace rv5s{

class Stages{
public:
    static void Fetch(Core& vm_core);

    static void Decode(Core& vm_core);

    static void Execute(Core& vm_core);

    static void MemoryAccess(Core& vm_core);

    static void WriteBack(Core& vm_core);


private:
    // FIXME: this doesn't belong here
    static void HandleSyscall(Core& vm_core);

    static void ResolveBranch(Core& vm_core);
    static void ExecuteBasic(Core& vm_core);
    static void ExecuteFloat(Core& vm_core);
    static void ExecuteDouble(Core& vm_core);

    static void WriteBackCsr(Core& vm_core);
};


} // namespace rv5s