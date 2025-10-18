#include "vm/rv5s/rv5s_vm.h"

using instruction_set::Instruction;
using instruction_set::get_instr_encoding;

void RV5SVM::WriteBack() {
	if (this->wb_instruction->opcode==0b1110011) { // CSR opcode
		WriteBackCsr();
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


// WHAT?: what is this
void RV5SVM::WriteBackCsr() {
	uint8_t& rd = this->wb_instruction->rd;
	uint8_t& funct3 = this->wb_instruction->funct3;

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
}

// WHAT?: what is this?
void RV5SVM::DebugWriteBackCsr(){
	WriteBackCsr();
}