#include "vm/rv5s/rv5s_vm.h"

using instruction_set::Instruction;
using instruction_set::get_instr_encoding;

void RV5SVM::WriteBack(bool debug_mode) {
	InstrContext& wb_instruction = GetWbInstruction();
	if (wb_instruction.opcode==0b1110011) { // CSR opcode
		WriteBackCsr(debug_mode);
		return;
	}
	// fcsr update
	if(wb_instruction.csr_overwritten){
		if(debug_mode){
			wb_instruction.csr_overwritten = this->registers_.ReadCsr(0x003);
		}
		this->registers_.WriteCsr(0x003, wb_instruction.fcsr_status);
	}

	if(!wb_instruction.reg_write) return;

	uint64_t& write_data = wb_instruction.mem_to_reg ?
		wb_instruction.mem_out : 
		wb_instruction.alu_out;
		
	if(wb_instruction.reg_write_to_fpr){
		if(debug_mode){
			wb_instruction.reg_overwritten = this->registers_.ReadFpr(wb_instruction.rd);
		}
		this->registers_.WriteFpr(wb_instruction.rd, write_data);
	}
	else{
		if(debug_mode){
			wb_instruction.reg_overwritten = this->registers_.ReadCsr(wb_instruction.rd);
		}
		this->registers_.WriteGpr(wb_instruction.rd, write_data);
	}
}



void RV5SVM::WriteBackCsr(bool debug_mode) {
	InstrContext& wb_instruction = GetWbInstruction();
	uint8_t& rd = wb_instruction.rd;
	uint8_t& funct3 = wb_instruction.funct3;

	uint16_t& csr_target_address_ = wb_instruction.csr_rd;
	uint64_t& csr_old_value_ = wb_instruction.csr_value;
	uint64_t& csr_write_val_ = wb_instruction.csr_write_val;
	uint8_t& csr_uimm_ = wb_instruction.csr_uimm;

	if(debug_mode){
		wb_instruction.is_csr_overwritten = true;
		wb_instruction.csr_overwritten = csr_old_value_;
		wb_instruction.reg_overwritten = this->registers_.ReadGpr(rd);
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
}
