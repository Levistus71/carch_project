#include "vm/rv5s/single_cycle/vm.h"

namespace rv5s{

SingleCycleVM::SingleCycleVM(){
    LoadVM();
}

void SingleCycleVM::Reset(){
    vm_core_.Reset();
    program_ = AssembledProgram{};
}

void SingleCycleVM::LoadVM(AssembledProgram program){
    program_ = program;
    vm_core_.Load(program_);
}

void SingleCycleVM::LoadVM(){
    vm_core_.Load();
}

void SingleCycleVM::Run(){
    vm_core_.stop_requested_ = false;
    vm_core_.debug_mode_ = false;

    SingleCycleExecutor::RunSingleCycle(vm_core_);
}

void SingleCycleVM::DebugRun(){
    vm_core_.stop_requested_ = false;
    vm_core_.debug_mode_ = true;

    SingleCycleExecutor::DebugRunSingleCycle(vm_core_);
}

void SingleCycleVM::Step(){
    vm_core_.stop_requested_ = false;
    vm_core_.debug_mode_ = true;

    SingleCycleExecutor::StepSingleCycle(vm_core_, true);
}

void SingleCycleVM::Undo(){
    vm_core_.stop_requested_ = false;
    vm_core_.debug_mode_ = true;

    SingleCycleExecutor::UndoSingleCycle(vm_core_);
}

uint64_t SingleCycleVM::ReadMemDoubleWord(uint64_t address){
    return vm_core_.memory_controller_.ReadDoubleWord(address);
}

const std::array<uint64_t, 32>& SingleCycleVM::GetGprValues(){
    return vm_core_.register_file_.GetGprValues();
}
const std::array<uint64_t, 32>& SingleCycleVM::GetFprValues(){
    return vm_core_.register_file_.GetFprValues();
}

std::vector<uint64_t> SingleCycleVM::GetInstructionPCs(){
    return {vm_core_.program_counter_};
}

VmBase::InstrView SingleCycleVM::GetInstructions(){
    std::vector<std::unique_ptr<const InstrContext>> result;
    result.push_back(std::make_unique<const SingleCycleInstrContext>(vm_core_.instr));

    InstrView ret;
    ret.pipeline = std::move(result);
    return ret;
}

} // namespace rv5s