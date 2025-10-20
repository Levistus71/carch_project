#include "vm/rv5s/rv5s_vm.h"

void RV5SVM::MemoryAccess(bool debug_mode) {
	InstrContext& mem_instruction = GetMemInstruction();
	if(!mem_instruction.mem_read && !mem_instruction.mem_write) return;

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
			mem_out += memory_controller_.ReadByte(address+i) << (i*8);
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
					
		if(debug_mode){
			for(size_t i=0;i<mem_instruction.mem_access_bytes;i++){
				mem_instruction.mem_overwritten.push_back(memory_controller_.ReadByte(address+i));
			}
		}

		for(size_t i=0;i<mem_instruction.mem_access_bytes;i++){
			memory_controller_.WriteByte(address + i, 0xFF & (write_data >> (i*8)));
		}
	}
}