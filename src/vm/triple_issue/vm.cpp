#include "vm/triple_issue/vm.h"


namespace triple_issue
{

TripleIssueVM::TripleIssueVM(){
    Reset();
}


void TripleIssueVM::Reset(){
    vm_core_.Reset();
}


void TripleIssueVM::LoadVM(){
    vm_core_.Load();
}

void TripleIssueVM::LoadVM(AssembledProgram program){
    program_ = program;
    vm_core_.Load(program);
}


void TripleIssueVM::Run(){
    TripleIssueExecutor::RunTripleIssue(vm_core_);
}

void TripleIssueVM::Step(){
    TripleIssueExecutor::StepTripleIssue(vm_core_);
}

void TripleIssueVM::DebugRun(){
    TripleIssueExecutor::DebugRunTripleIssue(vm_core_);
}

void TripleIssueVM::Undo(){
    TripleIssueExecutor::UndoTripleIssue(vm_core_);
}


uint64_t TripleIssueVM::ReadMemDoubleWord(uint64_t address){
    return vm_core_.memory_controller_.ReadDoubleWord(address);
}

const std::array<uint64_t, 32>& TripleIssueVM::GetGprValues(){
    return vm_core_.register_file_.GetGprValues();
}
const std::array<uint64_t, 32>& TripleIssueVM::GetFprValues(){
    return vm_core_.register_file_.GetFprValues();
}


std::vector<uint64_t> TripleIssueVM::GetInstructionPCs(){
    return {0};
}
VmBase::InstrView TripleIssueVM::GetInstructions(){
    std::vector<std::unique_ptr<const InstrContext>> pipeline;
    pipeline.push_back(std::make_unique<const TripleIssueInstrContext>(vm_core_.pipeline_reg_instrs_.if_id_1));
    pipeline.push_back(std::make_unique<const TripleIssueInstrContext>(vm_core_.pipeline_reg_instrs_.if_id_2));
    pipeline.push_back(std::make_unique<const TripleIssueInstrContext>(vm_core_.pipeline_reg_instrs_.if_id_3));
    pipeline.push_back(std::make_unique<const TripleIssueInstrContext>(vm_core_.pipeline_reg_instrs_.id_issue_1));
    pipeline.push_back(std::make_unique<const TripleIssueInstrContext>(vm_core_.pipeline_reg_instrs_.id_issue_2));
    pipeline.push_back(std::make_unique<const dual_issue::DualIssueInstrContext>(vm_core_.pipeline_reg_instrs_.id_issue_3));
    pipeline.push_back(std::make_unique<const dual_issue::DualIssueInstrContext>(vm_core_.pipeline_reg_instrs_.rsrvstn_alu));
    pipeline.push_back(std::make_unique<const dual_issue::DualIssueInstrContext>(vm_core_.pipeline_reg_instrs_.rsrvstn_falu));
    pipeline.push_back(std::make_unique<const dual_issue::DualIssueInstrContext>(vm_core_.pipeline_reg_instrs_.rsrvstn_lsu));
    pipeline.push_back(std::make_unique<const dual_issue::DualIssueInstrContext>(vm_core_.pipeline_reg_instrs_.alu_commit));
    pipeline.push_back(std::make_unique<const dual_issue::DualIssueInstrContext>(vm_core_.pipeline_reg_instrs_.falu_commit));
    pipeline.push_back(std::make_unique<const dual_issue::DualIssueInstrContext>(vm_core_.pipeline_reg_instrs_.lsu_commit));

    InstrView ret;
    ret.pipeline = std::move(pipeline);
    ret.reservation_station_alu = vm_core_.alu_que_.GetQue();
    ret.reservation_station_lsu = vm_core_.lsu_que_.GetQue();
    ret.reservation_station_falu = vm_core_.falu_que_.GetQue();
    ret.reorder_buffer = vm_core_.commit_buffer_.GetInstrs();
    ret.rob_status = vm_core_.commit_buffer_.GetStatus();
    ret.rob_head_tail = vm_core_.commit_buffer_.GetHeadTail();
    
    return ret;
}


VmBase::Stats& TripleIssueVM::GetStats(){
    return vm_core_.core_stats_;
}
    
} // namespace dual_issue
