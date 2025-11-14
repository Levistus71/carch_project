#pragma once
#include "../core/core.h"
#include "../stages/stages.h"
#include "utils.h"

namespace triple_issue{

class TripleIssueExecutor{
public:
    static void RunTripleIssue(TripleIssueCore& vm_core);

    static void DebugRunTripleIssue(TripleIssueCore& vm_core);

    static void StepTripleIssue(TripleIssueCore& vm_core);

    static void UndoTripleIssue(TripleIssueCore& vm_core);
};


} // namespace rv5s
