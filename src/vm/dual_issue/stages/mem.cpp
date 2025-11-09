#include "vm/dual_issue/stages/stages.h"

namespace dual_issue{

void exec_mini_alu(DualIssueInstrContext& mem_instruction, DualIssueCore& vm_core){
	uint64_t reg1_value = mem_instruction.rs1_value;
	uint64_t reg2_value = mem_instruction.rs2_value;
	
	if (mem_instruction.imm_to_alu) {
		int32_t imm = mem_instruction.immediate;
    	reg2_value = static_cast<uint64_t>(static_cast<int64_t>(imm));
  	}

	if(mem_instruction.auipc){
		reg1_value = mem_instruction.pc;
	}

  	// std::tie(mem_instruction.alu_out, mem_instruction.alu_overflow) = alu_.execute(mem_instruction.alu_op, reg1_value, reg2_value);
	auto [alu_out_temp, alu_overflow_temp] = vm_core.alu_.execute(mem_instruction.alu_op, reg1_value, reg2_value);
	mem_instruction.alu_out = alu_out_temp;
	mem_instruction.alu_overflow = alu_overflow_temp;
}


void DualIssueStages::MemoryAccess(DualIssueCore& vm_core){
    DualIssueInstrContext& mem_instruction = vm_core.pipeline_reg_instrs_.rsrvstn_lsu;
	if(mem_instruction.illegal)
		return;

	if(!mem_instruction.mem_read && !mem_instruction.mem_write) return;


	exec_mini_alu(mem_instruction, vm_core);

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

	if (mem_instruction.mem_read) {
		uint64_t& mem_out = mem_instruction.mem_out;
		uint64_t& address = mem_instruction.alu_out;
		mem_out = 0;

		for(size_t i=0;i<mem_instruction.mem_access_bytes;i++){
			mem_out += vm_core.memory_controller_.ReadByte(address+i) << (i*8);
		}
		
		if(mem_instruction.sign_extend){
			mem_out = sign_extend(mem_out, mem_instruction.mem_access_bytes*8);
		}

		return;
	}

	if (mem_instruction.mem_write) {
		uint64_t& address = mem_instruction.alu_out;

		uint64_t write_data = (mem_instruction.mem_write_data_from_gpr) ? 
			mem_instruction.rs2_value :
			mem_instruction.frs2_value;
					
		if(vm_core.debug_mode_){
			for(size_t i=0;i<mem_instruction.mem_access_bytes;i++){
				mem_instruction.mem_overwritten.push_back(vm_core.memory_controller_.ReadByte(address+i));
			}
		}

		for(size_t i=0;i<mem_instruction.mem_access_bytes;i++){
			vm_core.memory_controller_.WriteByte(address + i, 0xFF & (write_data >> (i*8)));
		}
	}
}

} // namespace rv5s