#include "vm/vm_main.h"
#include "vm/rv5s/pipelined/vm.h"
#include "vm/rv5s/single_cycle/vm.h"
#include "vm_asm_mw.h"

VM::VM(){
    if(vm_config::config.pipelining_enabled){
        vm_ = std::make_unique<rv5s::PipelinedVM>();
    }
    else{
        vm_ = std::make_unique<rv5s::SingleCycleVM>();
    }
}


void VM::Reset(){
    vm_->Reset();
}

void VM::LoadVM(){
    if(vm_config::config.pipelining_enabled){
        vm_ = std::make_unique<rv5s::PipelinedVM>();
    }
    else{
        vm_ = std::make_unique<rv5s::SingleCycleVM>();
    }
}
void VM::LoadVM(AssembledProgram program){
    LoadVM();
    program_ = program;
    vm_->LoadVM(program);
}

void VM::Run(){
    vm_->Run();
}
void VM::DebugRun(){
    vm_->DebugRun();
}

void VM::Step(){
    vm_->Step();
}

void VM::Undo(){
    vm_->Undo();
}

uint64_t VM::ReadMemDoubleWord(uint64_t address){
    return vm_->ReadMemDoubleWord(address);
}

const std::array<uint64_t, 32>& VM::GetGprValues(){
    return vm_->GetGprValues();
}

const std::array<uint64_t, 32>& VM::GetFprValues(){
    return vm_->GetFprValues();
}

std::vector<uint64_t> VM::GetInstructionPCs(){
    return vm_->GetInstructionPCs();
}

std::vector<std::unique_ptr<const InstrContext>> VM::GetInstructions(){
    return vm_->GetInstructions();
}

bool VM::PipeliningEnabled(){
    rv5s::PipelinedVM* test = dynamic_cast<rv5s::PipelinedVM*>(vm_.get());

    if(test==nullptr)
        return false;
    else
        return true;
}

bool VM::ForwardingEnabled(){
    rv5s::PipelinedVM* test = dynamic_cast<rv5s::PipelinedVM*>(vm_.get());

    if(test==nullptr)
        return false;
    else{
        return test->ForwardingEnabled();
    }
}

bool VM::HazardEnabled(){
    rv5s::PipelinedVM* test = dynamic_cast<rv5s::PipelinedVM*>(vm_.get());

    if(test==nullptr)
        return false;
    else{
        return test->HazardEnabled();
    }
}