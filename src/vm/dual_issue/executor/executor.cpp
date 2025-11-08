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

    // Functional units
    vm_core.alu_que_.ListenToBroadCast(vm_core.broadcast_bus_);
    vm_core.lsu_que_.ListenToBroadCast(vm_core.broadcast_bus_);

    vm_core.pipeline_reg_instrs_.rsrvstn_alu = vm_core.alu_que_.GetReadyInstr(vm_core);
    vm_core.pipeline_reg_instrs_.rsrvstn_lsu = vm_core.lsu_que_.GetReadyInstr(vm_core);

    DualIssueStages::Execute(vm_core);
    DualIssueStages::MemoryAccess(vm_core);

    vm_core.pipeline_reg_instrs_.alu_commit = vm_core.pipeline_reg_instrs_.rsrvstn_alu;
    vm_core.pipeline_reg_instrs_.lsu_commit = vm_core.pipeline_reg_instrs_.rsrvstn_lsu;

    // Issue
    int num_fetch = DualIssueStages::Issue(vm_core);

    // Decode
    DualIssueStages::Decode(vm_core);

    if(num_fetch==1){
        DualIssueInstrContext if_id_1 = vm_core.pipeline_reg_instrs_.if_id_1;
        DualIssueInstrContext if_id_2 = vm_core.pipeline_reg_instrs_.if_id_2;

        vm_core.pipeline_reg_instrs_.id_issue_1 = if_id_2;
        vm_core.pipeline_reg_instrs_.if_id_2 = if_id_1;
    }
    
    // Fetch
    DualIssueStages::Fetch(vm_core, num_fetch);
}

void DualIssueExecutor::UndoDualIssue(DualIssueCore& vm_core){
    globals::vm_cout_file << "Running Dual Issue still under development." << std::endl;
}

} // namespace dual_issue
