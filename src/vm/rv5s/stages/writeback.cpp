#include "vm/rv5s/rv5s_vm.h"

using instruction_set::Instruction;
using instruction_set::get_instr_encoding;

void RV5SVM::WriteBack() {
	if (this->wb_instruction->opcode==0b1110011) { // CSR opcode
		WriteBackCsr(false);
		return;
	}

	if(!this->wb_instruction->reg_write) return;

	uint64_t& write_data = this->wb_instruction->mem_to_reg ?
		this->wb_instruction->mem_out : 
		this->wb_instruction->alu_out;
		
	if(this->wb_instruction->reg_write_to_fpr){
		this->registers_.WriteFpr(this->wb_instruction->rd, write_data);
	}
	else{
		this->registers_.WriteGpr(this->wb_instruction->rd, write_data);
	}
}


void RV5SVM::DebugWriteBack(){
	if (this->wb_instruction->opcode==0b1110011) { // CSR opcode
		DebugWriteBackCsr();
		return;
	}

	if(!this->wb_instruction->reg_write) return;

	uint64_t& write_data = this->wb_instruction->mem_to_reg ?
		this->wb_instruction->mem_out : 
		this->wb_instruction->alu_out;
		
	uint64_t old_reg_value;

	unsigned int reg_index = this->wb_instruction->rd;
	unsigned int reg_type;	// 0 for gpr, 1 for csr, 2 for fpr

	if(this->wb_instruction->reg_write_to_fpr){
		old_reg_value = this->registers_.ReadFpr(this->wb_instruction->rd);
		reg_type = 2;
		this->registers_.WriteFpr(this->wb_instruction->rd, write_data);
	}
	else{
		old_reg_value = this->registers_.ReadGpr(this->wb_instruction->rd);
		reg_type = 1;
		this->registers_.WriteGpr(this->wb_instruction->rd, write_data);
	}

	if(old_reg_value!=write_data){
		this->current_delta.register_changes.push_back({
			this->wb_instruction->rd, 
			reg_type, 
			old_reg_value, 
			write_data
		});
	}
}


void RV5SVM::WriteBackCsr(bool debug_mode) {
	uint8_t& rd = this->wb_instruction->rd;
	uint8_t& funct3 = this->wb_instruction->funct3;

	uint16_t& csr_target_address_ = this->wb_instruction->csr_rd;
	uint64_t& csr_old_value_ = this->wb_instruction->csr_value;
	uint64_t& csr_write_val_ = this->wb_instruction->csr_write_val;
	uint8_t& csr_uimm_ = this->wb_instruction->csr_uimm;

	uint64_t csr_old_value_debug = csr_old_value_;
	uint64_t gpr_old_value_debug;
	if(debug_mode){
		gpr_old_value_debug = this->registers_.ReadGpr(rd);
	}

	switch (funct3) {
		case get_instr_encoding(Instruction::kcsrrw).funct3: { // CSRRW
			registers_.WriteGpr(rd, csr_old_value_);
			registers_.WriteCsr(csr_target_address_, csr_write_val_);
			break;
		}
		case get_instr_encoding(Instruction::kcsrrs).funct3: { // CSRRS
			registers_.WriteGpr(rd, csr_old_value_);
			if (csr_write_val_!=0) {
				registers_.WriteCsr(csr_target_address_, csr_old_value_ | csr_write_val_);
			}
			break;
		}
		case get_instr_encoding(Instruction::kcsrrc).funct3: { // CSRRC
			registers_.WriteGpr(rd, csr_old_value_);
			if (csr_write_val_!=0) {
				registers_.WriteCsr(csr_target_address_, csr_old_value_ & ~csr_write_val_);
			}
			break;
		}
		case get_instr_encoding(Instruction::kcsrrwi).funct3: { // CSRRWI
			registers_.WriteGpr(rd, csr_old_value_);
			registers_.WriteCsr(csr_target_address_, csr_uimm_);
			break;
		}
		case get_instr_encoding(Instruction::kcsrrsi).funct3: { // CSRRSI
			registers_.WriteGpr(rd, csr_old_value_);
			if (csr_uimm_!=0) {
				registers_.WriteCsr(csr_target_address_, csr_old_value_ | csr_uimm_);
			}
			break;
		}
		case get_instr_encoding(Instruction::kcsrrci).funct3: { // CSRRCI
			registers_.WriteGpr(rd, csr_old_value_);
			if (csr_uimm_!=0) {
				registers_.WriteCsr(csr_target_address_, csr_old_value_ & ~csr_uimm_);
			}
			break;
		}
	}

	if(debug_mode){
		uint64_t csr_new_value = this->registers_.ReadCsr(csr_target_address_);
		uint64_t gpr_new_value = this->registers_.ReadGpr(rd);

		if(csr_old_value_debug != csr_new_value){
			current_delta.register_changes.push_back({
				csr_target_address_,
				1, // 1 for csr
				csr_old_value_debug,
				csr_new_value
			});
		}

		if(gpr_old_value_debug != gpr_new_value){
			current_delta.register_changes.push_back({
				rd,
				0, // 0 for gpr
				gpr_old_value_debug,
				gpr_new_value
			});
		}
	}

}

void RV5SVM::DebugWriteBackCsr(){
	WriteBackCsr(true);
}