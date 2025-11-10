#include "vm/rv5s/pipelined/vm.h"

namespace rv5s{

PipelinedVM::PipelinedVM(){
    LoadVM();
}

void PipelinedVM::Reset(){
    vm_core_.Reset();
    program_ = AssembledProgram{};
}

void PipelinedVM::LoadVM(AssembledProgram program){
    program_ = program;
    vm_core_.Load(program_);
}

void PipelinedVM::LoadVM(){
    vm_core_.Load();
}

void PipelinedVM::Run(){
    vm_core_.debug_mode_ = false;
    vm_core_.is_stop_requested_ = false;

    PipelinedExecutor::RunPipelined(vm_core_);
}

void PipelinedVM::DebugRun(){
    vm_core_.debug_mode_ = true;
    vm_core_.is_stop_requested_ = false;

    PipelinedExecutor::DebugRunPipelined(vm_core_);
}

void PipelinedVM::Step(){
    vm_core_.debug_mode_ = true;
    vm_core_.is_stop_requested_ = false;

    PipelinedExecutor::StepPipelined(vm_core_);
    vm_core_.core_stats_.cycles++;
}

void PipelinedVM::Undo(){
    vm_core_.debug_mode_ = true;
    vm_core_.is_stop_requested_ = false;

    PipelinedExecutor::UndoPipelined(vm_core_);
}

uint64_t PipelinedVM::ReadMemDoubleWord(uint64_t address){
    return vm_core_.memory_controller_.ReadDoubleWord(address);
}

const std::array<uint64_t, 32>& PipelinedVM::GetGprValues(){
    return vm_core_.register_file_.GetGprValues();
}
const std::array<uint64_t, 32>& PipelinedVM::GetFprValues(){
    return vm_core_.register_file_.GetFprValues();
}

std::vector<uint64_t> PipelinedVM::GetInstructionPCs(){
    return {vm_core_.instruction_deque_[0].pc, vm_core_.instruction_deque_[1].pc, vm_core_.instruction_deque_[2].pc, vm_core_.instruction_deque_[3].pc, vm_core_.instruction_deque_[4].pc};
}


VmBase::InstrView PipelinedVM::GetInstructions() {
    std::vector<std::unique_ptr<const InstrContext>> result;
    result.push_back(std::make_unique<const PipelinedInstrContext>(vm_core_.GetIfInstruction()));
    result.push_back(std::make_unique<const PipelinedInstrContext>(vm_core_.GetIdInstruction()));
    result.push_back(std::make_unique<const PipelinedInstrContext>(vm_core_.GetExInstruction()));
    result.push_back(std::make_unique<const PipelinedInstrContext>(vm_core_.GetMemInstruction()));
    result.push_back(std::make_unique<const PipelinedInstrContext>(vm_core_.GetWbInstruction()));

    InstrView ret;
    ret.pipeline = std::move(result);
    return ret;
}

bool PipelinedVM::ForwardingEnabled(){
    return vm_core_.data_forwarding_enabled_;
}

bool PipelinedVM::HazardEnabled(){
    return vm_core_.hazard_detection_enabled_;
}

VmBase::Stats& PipelinedVM::GetStats(){
    return vm_core_.GetStats();
}

} // namespace rv5s