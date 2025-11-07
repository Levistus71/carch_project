#include "vm/rv5s/single_cycle/stages/stages.h"

namespace rv5s {

void SingleCycleStages::Fetch(SingleCycleCore& vm_core){
	vm_core.instr.pc = vm_core.program_counter_;
  	vm_core.instr.instruction = vm_core.memory_controller_.ReadWord(vm_core.program_counter_);
	vm_core.AddToProgramCounter(4);
}

} // namespace rv5s