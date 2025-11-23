#include "vm/triple_issue/executor/executor.h"


namespace triple_issue
{
    
void TripleIssueExecutor::RunTripleIssue(TripleIssueCore& vm_core){
    globals::vm_cout_file << "Running Triple Issue still under development." << std::endl;
}

void TripleIssueExecutor::DebugRunTripleIssue(TripleIssueCore& vm_core){
    globals::vm_cout_file << "Running Triple Issue still under development." << std::endl;
}

void TripleIssueExecutor::StepTripleIssue(TripleIssueCore& vm_core){

    // Writeback
    vm_core.commit_buffer_.Commit(vm_core);
    vm_core.commit_buffer_.Pull(vm_core);

    // Driving the pipeline part 1
    dual_issue::DualIssueInstrContext ready_alu_fu_instr = vm_core.alu_que_.GetReadyInstr();
    dual_issue::DualIssueInstrContext ready_falu_fu_instr = vm_core.falu_que_.GetReadyInstr();
    dual_issue::DualIssueInstrContext ready_lsu_fu_instr = vm_core.lsu_que_.GetInorderInstr();

    // Issue
    int num_issued = TripleIssueStages::Issue(vm_core);

    // Decode
    TripleIssueStages::Decode(vm_core);
    
    // Driving the pipeline part 2
    if(num_issued==3){
        vm_core.pipeline_reg_instrs_.id_issue_1 = vm_core.pipeline_reg_instrs_.if_id_1;
        vm_core.pipeline_reg_instrs_.id_issue_2 = vm_core.pipeline_reg_instrs_.if_id_2;
        vm_core.pipeline_reg_instrs_.id_issue_3 = vm_core.pipeline_reg_instrs_.if_id_3;
    }
    // id_issue_3 and id_issue_2 are free. if_id_1, if_id_2, if_id_3 are all decoded.
    // if_id_2 and if_id_3 will be used by the fetch
    // we push if_id_1 into id_issue_2
    // then we push if_id_2 into id_issue_3
    // then we push if_id_3 into if_id_1
    // then Fetch() stores the 2 into if_id_2 and if_id_3
    else if(num_issued==2){
        TripleIssueInstrContext if_id_1 = vm_core.pipeline_reg_instrs_.if_id_1;
        TripleIssueInstrContext if_id_2 = vm_core.pipeline_reg_instrs_.if_id_2;
        TripleIssueInstrContext if_id_3 = vm_core.pipeline_reg_instrs_.if_id_3;

        vm_core.pipeline_reg_instrs_.id_issue_2 = if_id_1;
        vm_core.pipeline_reg_instrs_.id_issue_3 = if_id_2;

        vm_core.pipeline_reg_instrs_.if_id_1 = if_id_3;
    }
    // id_issue_3 is free. if_id_1, if_id_2, if_id_3 are all decoded.
    // if_id_3 will be used by the fetch
    // we push if_id_1 into id_issue_3
    // then push if_id_2 into if_id_1
    // then push if_id_3 into if_id_2
    else if(num_issued==1){
        TripleIssueInstrContext if_id_1 = vm_core.pipeline_reg_instrs_.if_id_1;
        TripleIssueInstrContext if_id_2 = vm_core.pipeline_reg_instrs_.if_id_2;
        TripleIssueInstrContext if_id_3 = vm_core.pipeline_reg_instrs_.if_id_3;

        vm_core.pipeline_reg_instrs_.id_issue_3 = if_id_1;

        vm_core.pipeline_reg_instrs_.if_id_1 = if_id_2;
        vm_core.pipeline_reg_instrs_.if_id_2 = if_id_3;
    }

    // Fetch
    TripleIssueStages::Fetch(vm_core, num_issued);

    // Exec
    TripleIssueStages::ExecuteAlu(vm_core);
    TripleIssueStages::ExecuteFalu(vm_core);
    TripleIssueStages::MemoryAccess(vm_core);

    vm_core.pipeline_reg_instrs_.alu_commit = vm_core.pipeline_reg_instrs_.rsrvstn_alu;
    vm_core.pipeline_reg_instrs_.falu_commit = vm_core.pipeline_reg_instrs_.rsrvstn_falu;
    vm_core.pipeline_reg_instrs_.lsu_commit = vm_core.pipeline_reg_instrs_.rsrvstn_lsu;
    
    vm_core.pipeline_reg_instrs_.rsrvstn_alu = ready_alu_fu_instr;
    vm_core.pipeline_reg_instrs_.rsrvstn_falu = ready_falu_fu_instr;
    vm_core.pipeline_reg_instrs_.rsrvstn_lsu = ready_lsu_fu_instr;

    vm_core.alu_que_.ListenToBroadCast(vm_core.broadcast_bus_);
    vm_core.falu_que_.ListenToBroadCast(vm_core.broadcast_bus_);
    vm_core.lsu_que_.ListenToBroadCast(vm_core.broadcast_bus_);
    // for(auto& msg : vm_core.broadcast_bus_.broadcast_msgs){
    //     std::cout << msg.rob_idx << std::endl;
    // }
    // std::cout << std::endl;
    vm_core.broadcast_bus_.Reset();

    vm_core.core_stats_.cycles++;
}

void TripleIssueExecutor::UndoTripleIssue(TripleIssueCore& vm_core){
    globals::vm_cout_file << "Running Triple Issue still under development." << std::endl;
}

} // namespace dual_issue
