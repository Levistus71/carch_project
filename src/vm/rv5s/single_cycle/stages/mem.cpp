#include "vm/rv5s/single_cycle/stages/stages.h"

namespace rv5s{

void SingleCycleStages::MemoryAccess(SingleCycleCore& vm_core){
	if(!vm_core.instr.mem_read && !vm_core.instr.mem_write) return;

	auto sign_extend = [](uint64_t value, unsigned int bits) -> uint64_t {
		if (bits >= 64) {
			return value;
		}
		
		unsigned int shift = 64 - bits;

		int64_t signed_value = static_cast<int64_t>(value);
		signed_value <<= shift;
		signed_value >>= shift;

		return static_cast<uint64_t>(signed_value);
	};

	if (vm_core.instr.mem_read) {
		uint64_t& mem_out = vm_core.instr.mem_out;
		uint64_t& address = vm_core.instr.alu_out;
		mem_out = 0;

		for(size_t i=0;i<vm_core.instr.mem_access_bytes;i++){
			mem_out += vm_core.memory_controller_.ReadByte(address+i) << (i*8);
		}
		
		if(vm_core.instr.sign_extend){
			mem_out = sign_extend(mem_out, vm_core.instr.mem_access_bytes*8);
		}

		return;
	}

	if (vm_core.instr.mem_write) {
		uint64_t& address = vm_core.instr.alu_out;

		uint64_t write_data = (vm_core.instr.mem_write_data_from_gpr) ? 
			vm_core.instr.rs2_value :
			vm_core.instr.frs2_value;
					
		if(vm_core.debug_mode_){
			for(size_t i=0;i<vm_core.instr.mem_access_bytes;i++){
				vm_core.instr.mem_overwritten.push_back(vm_core.memory_controller_.ReadByte(address+i));
			}
		}

		for(size_t i=0;i<vm_core.instr.mem_access_bytes;i++){
			vm_core.memory_controller_.WriteByte(address + i, 0xFF & (write_data >> (i*8)));
		}
	}
}

} // namespace rv5s