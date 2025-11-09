#include "vm/dual_issue/vm.h"


namespace dual_issue
{

DualIssueVM::DualIssueVM(){
    Reset();
}


void DualIssueVM::Reset(){
    vm_core_.Reset();
}


void DualIssueVM::LoadVM(){
    vm_core_.Load();
}

void DualIssueVM::LoadVM(AssembledProgram program){
    program_ = program;
    vm_core_.Load(program);
}


void DualIssueVM::Run(){
    DualIssueExecutor::RunDualIssue(vm_core_);
}

void DualIssueVM::Step(){
    DualIssueExecutor::StepDualIssue(vm_core_);
}

void DualIssueVM::DebugRun(){
    DualIssueExecutor::DebugRunDualIssue(vm_core_);
}

void DualIssueVM::Undo(){
    DualIssueExecutor::UndoDualIssue(vm_core_);
}


uint64_t DualIssueVM::ReadMemDoubleWord(uint64_t address){
    return vm_core_.memory_controller_.ReadDoubleWord(address);
}

const std::array<uint64_t, 32>& DualIssueVM::GetGprValues(){
    return vm_core_.register_file_.GetGprValues();
}
const std::array<uint64_t, 32>& DualIssueVM::GetFprValues(){
    return vm_core_.register_file_.GetFprValues();
}


std::vector<uint64_t> DualIssueVM::GetInstructionPCs(){
    return {0};
}
VmBase::InstrView DualIssueVM::GetInstructions(){
    std::vector<std::unique_ptr<const InstrContext>> pipeline;
    pipeline.push_back(std::make_unique<const DualIssueInstrContext>(vm_core_.pipeline_reg_instrs_.if_id_1));
    pipeline.push_back(std::make_unique<const DualIssueInstrContext>(vm_core_.pipeline_reg_instrs_.if_id_2));
    pipeline.push_back(std::make_unique<const DualIssueInstrContext>(vm_core_.pipeline_reg_instrs_.id_issue_1));
    pipeline.push_back(std::make_unique<const DualIssueInstrContext>(vm_core_.pipeline_reg_instrs_.id_issue_2));
    pipeline.push_back(std::make_unique<const DualIssueInstrContext>(vm_core_.pipeline_reg_instrs_.alu_commit));
    pipeline.push_back(std::make_unique<const DualIssueInstrContext>(vm_core_.pipeline_reg_instrs_.lsu_commit));
    pipeline.push_back(std::make_unique<const DualIssueInstrContext>(vm_core_.pipeline_reg_instrs_.rsrvstn_alu));
    pipeline.push_back(std::make_unique<const DualIssueInstrContext>(vm_core_.pipeline_reg_instrs_.rsrvstn_lsu));

    InstrView ret;
    ret.pipeline = std::move(pipeline);
    ret.reservation_station_alu = vm_core_.alu_que_.GetQue();
    ret.reservation_station_lsu = vm_core_.lsu_que_.GetQue();
    ret.reorder_buffer = vm_core_.commit_buffer_.GetInstrs();
    ret.rob_status = vm_core_.commit_buffer_.GetStatus();
    ret.rob_head_tail = vm_core_.commit_buffer_.GetHeadTail();
    
    return ret;
}
    
} // namespace dual_issue
