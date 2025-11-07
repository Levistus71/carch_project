#pragma once
#include "../core/core.h"

namespace dual_issue{

class DualIssueStages{
public:
    static void Fetch(DualIssueCore& vm_core);

    static void Decode(DualIssueCore& vm_core);

    static void Execute(DualIssueCore& vm_core);

    static void MemoryAccess(DualIssueCore& vm_core);

    static void WriteBack(DualIssueCore& vm_core);


private:
    // FIXME: this doesn't belong here
    static void HandleSyscall(DualIssueCore& vm_core);

    static void ResolveBranch(DualIssueCore& vm_core);
    static void ExecuteBasic(DualIssueCore& vm_core);
    static void ExecuteFloat(DualIssueCore& vm_core);
    static void ExecuteDouble(DualIssueCore& vm_core);

    static void WriteBackCsr(DualIssueCore& vm_core);
};


} // namespace rv5s