#include "vm/rv5s/vm.h"

namespace rv5s{

void VM::Reset(){
    vm_core_.Reset();
    program_ = AssembledProgram{};
}

void VM::LoadVM(AssembledProgram program){
    program_ = program;
    vm_core_.Load(program_);
}

void VM::RequestStop(){
    vm_core_.stop_requested_ = true;
}

void VM::Run(){
    Executor::Run(vm_core_);
}

void VM::DebugRun(){
    Executor::DebugRun(vm_core_);
}

void VM::Step(){
    Executor::Step(vm_core_);
}

void VM::Undo(){
    Executor::Undo(vm_core_);
}

bool VM::PipeliningEnabled(){
    return vm_core_.pipelining_enabled_;
}

const std::array<uint64_t, 32>& VM::GetGprValues(){
    return vm_core_.register_file_.GetGprValues();
}
const std::array<uint64_t, 32>& VM::GetFprValues(){
    return vm_core_.register_file_.GetFprValues();
}

std::vector<uint64_t> VM::GetInstructionPCs(){
    if(vm_core_.pipelining_enabled_)
        return {vm_core_.instruction_deque_[0].pc, vm_core_.instruction_deque_[1].pc, vm_core_.instruction_deque_[2].pc, vm_core_.instruction_deque_[3].pc, vm_core_.instruction_deque_[4].pc};
    else
        return {vm_core_.program_counter_};
}

} // namespace rv5s