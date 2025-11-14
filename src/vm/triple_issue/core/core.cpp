#include "vm/triple_issue/core/core.h"

namespace triple_issue
{

void TripleIssueCore::PipelineRegInstrs::FlushPreIssueRegs(){
    dual_issue::DualIssueCore::PipelineRegInstrs::FlushPreIssueRegs();

    if_id_3.illegal = true;
    id_issue_3.illegal = true;
}

void TripleIssueCore::PipelineRegInstrs::Reset(){
    dual_issue::DualIssueCore::PipelineRegInstrs::Reset();

    if_id_1.illegal = true;
    if_id_2.illegal = true;
    if_id_3.illegal = true;
    
    id_issue_1.illegal = true;
    id_issue_2.illegal = true;
    id_issue_3.illegal = true;

    rsrvstn_falu.illegal = true;
    
    falu_commit.illegal = true;
}
    
void TripleIssueCore::FlushPreIssueRegs(){
    pipeline_reg_instrs_.FlushPreIssueRegs();
}


void TripleIssueCore::Reset(){
    dual_issue::DualIssueCore::Reset();

    pipeline_reg_instrs_.Reset();
    falu_que_.Reset();
    commit_buffer_.Reset();
}


} // namespace triple_issue
