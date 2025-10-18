#include "vm/rv5s/rv5s_vm.h"

void RV5SVM::MemoryAccess() {
	if(!this->mem_instruction->mem_read && !this->mem_instruction->mem_write) return;

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

	if (this->mem_instruction->mem_read) {
		uint64_t& mem_out = this->mem_instruction->mem_out;
		uint64_t& address = this->mem_instruction->alu_out;
		mem_out = 0;

		for(size_t i=0;i<this->mem_instruction->mem_access_bytes;i++){
			mem_out += memory_controller_.ReadByte(address+i) << (i*8);
		}
		
		if(this->mem_instruction->sign_extend){
			mem_out = sign_extend(mem_out, this->mem_instruction->mem_access_bytes*8);
		}

		return;
	}

	if (this->mem_instruction->mem_write) {
		uint64_t& address = this->mem_instruction->alu_out;

		uint64_t write_data = (this->mem_instruction->mem_write_data_from_gpr) ? 
			this->mem_instruction->rs2_value :
			this->mem_instruction->frs2_value;			

		for(size_t i=0;i<this->mem_instruction->mem_access_bytes;i++){
			memory_controller_.WriteByte(address + i, 0xFF & (write_data >> (i*8)));
		}
	}
}



void RV5SVM::DebugMemoryAccess(){
	if(!this->mem_instruction->mem_read && !this->mem_instruction->mem_write) return;

	if(this->mem_instruction->mem_read){
		MemoryAccess();
		return;
	}

	if(this->mem_instruction->mem_write){
		uint64_t& address = this->mem_instruction->alu_out;

		std::vector<uint8_t> old_bytes;
		std::vector<uint8_t> new_bytes;

		uint64_t write_data = (this->mem_instruction->mem_write_data_from_gpr) ? 
			this->mem_instruction->rs2_value :
			this->mem_instruction->frs2_value;

		for(size_t i=0;i<this->mem_instruction->mem_access_bytes;i++){
			old_bytes.push_back(memory_controller_.ReadByte(address + i));
		}

		for(size_t i=0;i<this->mem_instruction->mem_access_bytes;i++){
			memory_controller_.WriteByte(address + i, 0xFF & (write_data >> (i*8)));
			new_bytes.push_back(0xFF & (write_data >> (i*8)));
		}

		  if (old_bytes != new_bytes) {
			this->current_delta.memory_changes.push_back({
				address,
				old_bytes,
				new_bytes
			});
		}
	}
}
