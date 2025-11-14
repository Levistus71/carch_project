#include "vm/triple_issue/stages/stages.h"
#include "vm/dual_issue/stages/stages.h"


namespace triple_issue
{

void TripleIssueStages::WriteBack(dual_issue::DualIssueInstrContext& wb_instruction, TripleIssueCore& vm_core){
    dual_issue::DualIssueStages::WriteBack(wb_instruction, vm_core);
}
    
} // namespace triple_issue
