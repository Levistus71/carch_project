#include "../core/core.h"
#include "../stages/stages.h"
#include "utils.h"
#include <thread>
#include <chrono>

namespace dual_issue{

class DualIssueExecutor{
public:
    static void RunDualIssue(DualIssueCore& vm_core);

    static void DebugRunDualIssue(DualIssueCore& vm_core);

    static void StepDualIssue(DualIssueCore& vm_core);

    static void UndoDualIssue(DualIssueCore& vm_core);
};


} // namespace rv5s
