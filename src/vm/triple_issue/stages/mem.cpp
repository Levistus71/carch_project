#include "vm/triple_issue/stages/stages.h"
#include "vm/dual_issue/stages/stages.h"

namespace triple_issue
{

void TripleIssueStages::MemoryAccess(TripleIssueCore& vm_core){
    dual_issue::DualIssueStages::MemoryAccess(vm_core);
}

} // namespace triple_issue
