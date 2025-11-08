#include "vm/dual_issue/core/core.h"


namespace dual_issue
{

uint64_t DualIssueCore::GetProgramCounter() const{
    return pc;
}
void DualIssueCore::AddToProgramCounter(int64_t value){
    pc = static_cast<uint64_t>(static_cast<int64_t>(pc) + value);
}
void DualIssueCore::SetProgramCounter(uint64_t value){
    pc = value;
}


    
} // namespace dual_issue
