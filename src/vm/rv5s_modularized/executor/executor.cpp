#include "vm/rv5s_modularized/executor/executor.h"

namespace rv5s
{

void Executor::Run(Core& vm_core){
    vm_core.debug_mode_ = false;
    vm_core.is_stop_requested_ = false;

    if(vm_core.pipelining_enabled_){
        RunPipelined(vm_core);
    }
    else{
        RunSingleCycle(vm_core);
    }
}


void Executor::Step(Core& vm_core){
    vm_core.debug_mode_ = true;
    vm_core.stop_requested_ = false;

    if(vm_core.pipelining_enabled_){
        StepPipelined(vm_core);
    }
    else{
        StepSingleCycle(vm_core, true);
    }
}


void Executor::DebugRun(Core& vm_core){
    vm_core.debug_mode_ = true;
    vm_core.stop_requested_ = false;

    if(vm_core.pipelining_enabled_){
        DebugRunPipelined(vm_core);
    }
    else{
        DebugRunSingleCycle(vm_core);
    }
}

void Executor::Undo(Core& vm_core){
    vm_core.debug_mode_ = true;
    vm_core.stop_requested_ = false;

    if(vm_core.pipelining_enabled_){
        UndoPipelined(vm_core);
    }
    else{
        UndoSingleCycle(vm_core);
    }
}



} // namespace rv5s