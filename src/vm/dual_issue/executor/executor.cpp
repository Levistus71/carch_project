#include "vm/dual_issue/executor/executor.h"

namespace dual_issue
{
    
void DualIssueExecutor::RunDualIssue(DualIssueCore& vm_core){
    globals::vm_cout_file << "Running Dual Issue still under development." << std::endl;
}

void DualIssueExecutor::DebugRunDualIssue(DualIssueCore& vm_core){
    globals::vm_cout_file << "Running Dual Issue still under development." << std::endl;
}

void DualIssueExecutor::StepDualIssue(DualIssueCore& vm_core){

    // Writeback
    vm_core.commit_buffer_.Pull(vm_core);
    vm_core.commit_buffer_.Commit(vm_core);

    vm_core.alu_que_.ListenToBroadCast(vm_core.broadcast_bus_);
    vm_core.lsu_que_.ListenToBroadCast(vm_core.broadcast_bus_);
    
    DualIssueStages::Execute(vm_core);
    DualIssueStages::MemoryAccess(vm_core);

    vm_core.pipeline_reg_instrs_.alu_commit = vm_core.pipeline_reg_instrs_.rsrvstn_alu;
    vm_core.pipeline_reg_instrs_.lsu_commit = vm_core.pipeline_reg_instrs_.rsrvstn_lsu;
    
    vm_core.pipeline_reg_instrs_.rsrvstn_alu = vm_core.alu_que_.GetReadyInstr();
    vm_core.pipeline_reg_instrs_.rsrvstn_lsu = vm_core.lsu_que_.GetReadyInstr();

    // Issue
    int num_issued = DualIssueStages::Issue(vm_core);

    // Decode
    DualIssueStages::Decode(vm_core);

    if(num_issued==2){
        vm_core.pipeline_reg_instrs_.id_issue_1 = vm_core.pipeline_reg_instrs_.if_id_1;
        vm_core.pipeline_reg_instrs_.id_issue_2 = vm_core.pipeline_reg_instrs_.if_id_2;
    }
    // id_issue_2 is free. if_id_1 and if_id_2 are both decoded.
    // we push if_id_1 into id_issue_2
    // then we push if_id_2 into if_id_1
    // then Fetch() stores the 1 fetch into if_id_2
    else if(num_issued==1){
        DualIssueInstrContext if_id_1 = vm_core.pipeline_reg_instrs_.if_id_1;
        DualIssueInstrContext if_id_2 = vm_core.pipeline_reg_instrs_.if_id_2;

        vm_core.pipeline_reg_instrs_.id_issue_2 = if_id_1;
        vm_core.pipeline_reg_instrs_.if_id_1 = if_id_2;
    }
    
    // Fetch
    DualIssueStages::Fetch(vm_core, num_issued);
}

void DualIssueExecutor::UndoDualIssue(DualIssueCore& vm_core){
    globals::vm_cout_file << "Running Dual Issue still under development." << std::endl;
}

} // namespace dual_issue
