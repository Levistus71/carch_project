#include "vm/rv5s/pipelined/stages/stages.h"
#include "common/instructions.h"

namespace rv5s{

void PipelinedStages::WriteBack(PipelinedCore& vm_core){
    PipelinedInstrContext& wb_instruction = vm_core.GetWbInstruction();
	if(wb_instruction.nopped)
		return;

	vm_core.core_stats_.instrs_retired++;

	if (wb_instruction.opcode==0b1110011) { // CSR opcode
		WriteBackCsr(vm_core);
		return;
	}

	// fcsr update
	if(wb_instruction.csr_overwritten){
		if(vm_core.debug_mode_){
			wb_instruction.csr_overwritten = vm_core.register_file_.ReadCsr(0x003);
		}
		vm_core.register_file_.WriteCsr(0x003, wb_instruction.fcsr_status);
	}

	if(!wb_instruction.reg_write) return;

	uint64_t& write_data = wb_instruction.mem_to_reg ?
		wb_instruction.mem_out : 
		wb_instruction.alu_out;
		
	if(wb_instruction.reg_write_to_fpr){
		if(vm_core.debug_mode_){
			wb_instruction.reg_overwritten = vm_core.register_file_.ReadFpr(wb_instruction.rd);
		}
		vm_core.register_file_.WriteFpr(wb_instruction.rd, write_data);
	}
	else{
		if(vm_core.debug_mode_){
			wb_instruction.reg_overwritten = vm_core.register_file_.ReadGpr(wb_instruction.rd);
		}
		vm_core.register_file_.WriteGpr(wb_instruction.rd, write_data);
	}
}


void PipelinedStages::WriteBackCsr(PipelinedCore& vm_core){
    using instruction_set::get_instr_encoding;
    using instruction_set::Instruction;

	PipelinedInstrContext& wb_instruction = vm_core.GetWbInstruction();
	uint8_t& rd = wb_instruction.rd;
	uint8_t& funct3 = wb_instruction.funct3;

	uint16_t& csr_target_address_ = wb_instruction.csr_rd;
	uint64_t& csr_old_value_ = wb_instruction.csr_value;
	uint64_t& csr_write_val_ = wb_instruction.csr_write_val;
	uint8_t& csr_uimm_ = wb_instruction.csr_uimm;

	if(vm_core.debug_mode_){
		wb_instruction.is_csr_overwritten = true;
		wb_instruction.csr_overwritten = csr_old_value_;
		wb_instruction.reg_overwritten = vm_core.register_file_.ReadGpr(rd);
	}

	switch (funct3) {
		case get_instr_encoding(Instruction::kcsrrw).funct3: { // CSRRW
			vm_core.register_file_.WriteGpr(rd, csr_old_value_);
			vm_core.register_file_.WriteCsr(csr_target_address_, csr_write_val_);
			break;
		}
		case get_instr_encoding(Instruction::kcsrrs).funct3: { // CSRRS
			vm_core.register_file_.WriteGpr(rd, csr_old_value_);
			if (csr_write_val_!=0) {
				vm_core.register_file_.WriteCsr(csr_target_address_, csr_old_value_ | csr_write_val_);
			}
			break;
		}
		case get_instr_encoding(Instruction::kcsrrc).funct3: { // CSRRC
			vm_core.register_file_.WriteGpr(rd, csr_old_value_);
			if (csr_write_val_!=0) {
				vm_core.register_file_.WriteCsr(csr_target_address_, csr_old_value_ & ~csr_write_val_);
			}
			break;
		}
		case get_instr_encoding(Instruction::kcsrrwi).funct3: { // CSRRWI
			vm_core.register_file_.WriteGpr(rd, csr_old_value_);
			vm_core.register_file_.WriteCsr(csr_target_address_, csr_uimm_);
			break;
		}
		case get_instr_encoding(Instruction::kcsrrsi).funct3: { // CSRRSI
			vm_core.register_file_.WriteGpr(rd, csr_old_value_);
			if (csr_uimm_!=0) {
				vm_core.register_file_.WriteCsr(csr_target_address_, csr_old_value_ | csr_uimm_);
			}
			break;
		}
		case get_instr_encoding(Instruction::kcsrrci).funct3: { // CSRRCI
			vm_core.register_file_.WriteGpr(rd, csr_old_value_);
			if (csr_uimm_!=0) {
				vm_core.register_file_.WriteCsr(csr_target_address_, csr_old_value_ & ~csr_uimm_);
			}
			break;
		}
	}
}

} // namespace rv5s