#include "vm/rv5s/single_cycle/stages/stages.h"
#include "common/instructions.h"

namespace rv5s{

void SingleCycleStages::WriteBack(SingleCycleCore& vm_core){
	if (vm_core.instr.opcode==0b1110011) { // CSR opcode
		WriteBackCsr(vm_core);
		return;
	}

	// fcsr update
	if(vm_core.instr.csr_overwritten){
		if(vm_core.debug_mode_){
			vm_core.instr.csr_overwritten = vm_core.register_file_.ReadCsr(0x003);
		}
		vm_core.register_file_.WriteCsr(0x003, vm_core.instr.fcsr_status);
	}

	if(!vm_core.instr.reg_write) return;

	uint64_t& write_data = vm_core.instr.mem_to_reg ?
		vm_core.instr.mem_out : 
		vm_core.instr.alu_out;
		
	if(vm_core.instr.reg_write_to_fpr){
		if(vm_core.debug_mode_){
			vm_core.instr.reg_overwritten = vm_core.register_file_.ReadFpr(vm_core.instr.rd);
		}
		vm_core.register_file_.WriteFpr(vm_core.instr.rd, write_data);
	}
	else{
		if(vm_core.debug_mode_){
			vm_core.instr.reg_overwritten = vm_core.register_file_.ReadGpr(vm_core.instr.rd);
		}
		vm_core.register_file_.WriteGpr(vm_core.instr.rd, write_data);
	}
}


void SingleCycleStages::WriteBackCsr(SingleCycleCore& vm_core){
    using instruction_set::get_instr_encoding;
    using instruction_set::Instruction;

	uint8_t& rd = vm_core.instr.rd;
	uint8_t& funct3 = vm_core.instr.funct3;

	uint16_t& csr_target_address_ = vm_core.instr.csr_rd;
	uint64_t& csr_old_value_ = vm_core.instr.csr_value;
	uint64_t& csr_write_val_ = vm_core.instr.csr_write_val;
	uint8_t& csr_uimm_ = vm_core.instr.csr_uimm;

	if(vm_core.debug_mode_){
		vm_core.instr.is_csr_overwritten = true;
		vm_core.instr.csr_overwritten = csr_old_value_;
		vm_core.instr.reg_overwritten = vm_core.register_file_.ReadGpr(rd);
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