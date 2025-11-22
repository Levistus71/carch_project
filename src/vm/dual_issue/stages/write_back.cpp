#include "vm/dual_issue/stages/stages.h"
#include "common/instructions.h"
#include "vm/triple_issue/core/core.h"

namespace dual_issue{

void DualIssueStages::WriteBack(DualIssueInstrContext& wb_instruction, DualIssueCore& vm_core){
	if(wb_instruction.illegal)
		return;

	vm_core.core_stats_.instrs_retired++;
	
	if (wb_instruction.opcode==0b1110011) { // CSR opcode
		WriteBackCsr(wb_instruction, vm_core);
		return;
	}

	// fcsr update
	if(wb_instruction.csr_overwritten){
		if(vm_core.debug_mode_){
			wb_instruction.csr_overwritten = vm_core.register_file_.ReadCsr(0x003);
		}
		vm_core.register_file_.WriteCsr(0x003, wb_instruction.fcsr_status);
	}

	if(wb_instruction.branch)
		ResolveBranch(wb_instruction, vm_core);

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

void DualIssueStages::ResolveBranch(DualIssueInstrContext& instr, DualIssueCore& vm_core){
	vm_core.core_stats_.branch_instrs++;

    using instruction_set::Instruction;
    using instruction_set::get_instr_encoding;

	uint8_t& opcode = instr.opcode;
	uint8_t& funct3 = instr.funct3;

	triple_issue::TripleIssueCore* upcasted_triple = dynamic_cast<triple_issue::TripleIssueCore*>(&vm_core);

	if (opcode==get_instr_encoding(Instruction::kjalr).opcode || 
			opcode==get_instr_encoding(Instruction::kjal).opcode) {
		
		// if branch was already taken, we skip updating the pc
		if(instr.branch_predicted_taken)
			return;
		
		vm_core.core_stats_.branch_mispredicts++;
		
		if(upcasted_triple)
			upcasted_triple->commit_buffer_.ResetTailTillIdx(instr.rob_idx, *upcasted_triple);
		else
        	vm_core.commit_buffer_.ResetTailTillIdx(instr.rob_idx, vm_core);
	
		vm_core.FlushPreIssueRegs();
		if (opcode==get_instr_encoding(Instruction::kjalr).opcode) { 
			vm_core.branch_predictor_.update_btb(instr.pc, true, instr.alu_out);
			vm_core.SetProgramCounter(instr.alu_out);
		}
		else if (opcode==get_instr_encoding(Instruction::kjal).opcode) {
			vm_core.branch_predictor_.update_btb(instr.pc, true, instr.pc + instr.immediate);
			vm_core.SetProgramCounter(instr.pc + instr.immediate);
		}

		// storing the current value of pc for returning (storing it in rd)
		instr.alu_out = instr.pc + 4;

		return;
	}
	else if (opcode==get_instr_encoding(Instruction::kbeq).opcode ||
				opcode==get_instr_encoding(Instruction::kbne).opcode ||
				opcode==get_instr_encoding(Instruction::kblt).opcode ||
				opcode==get_instr_encoding(Instruction::kbge).opcode ||
				opcode==get_instr_encoding(Instruction::kbltu).opcode ||
				opcode==get_instr_encoding(Instruction::kbgeu).opcode) {
		
		bool branch_flag = false;
		switch (funct3) {
			case 0b000: {// BEQ
				branch_flag = (instr.alu_out==0);
				break;
			}
			case 0b001: {// BNE
				branch_flag = (instr.alu_out!=0);
				break;
			}
			case 0b100: {// BLT
				branch_flag = (instr.alu_out==1);
				break;
			}
			case 0b101: {// BGE
				branch_flag = (instr.alu_out==0);
				break;
			}
			case 0b110: {// BLTU
				branch_flag = (instr.alu_out==1);
				break;
			}
			case 0b111: {// BGEU
				branch_flag = (instr.alu_out==0);
				break;
			}
		}

		if (branch_flag) {

			// if branch was predicted taken, we skip updating pc
			if(instr.branch_predicted_taken)
				return;

			vm_core.core_stats_.branch_mispredicts++;

			// // Subtracting 4 from pc (updated in Fetch())
			// vm_core.AddToProgramCounter(-4);
			// vm_core.AddToProgramCounter(instr.immediate);
			if(upcasted_triple)
				upcasted_triple->commit_buffer_.ResetTailTillIdx(instr.rob_idx, *upcasted_triple);
			else
            	vm_core.commit_buffer_.ResetTailTillIdx(instr.rob_idx, vm_core);

			vm_core.branch_predictor_.update_btb(instr.pc, true, instr.pc + instr.immediate);
			
			vm_core.FlushPreIssueRegs();
			vm_core.SetProgramCounter(instr.pc + instr.immediate);
		}
		else{
			// branch was incorrectly predicted, need to set the pc to pc+4
			if(instr.branch_predicted_taken){
				vm_core.core_stats_.branch_mispredicts++;
				
				if(upcasted_triple)
					upcasted_triple->commit_buffer_.ResetTailTillIdx(instr.rob_idx, *upcasted_triple);
				else
                	vm_core.commit_buffer_.ResetTailTillIdx(instr.rob_idx, vm_core);

				vm_core.branch_predictor_.update_btb(instr.pc, false, instr.pc + instr.immediate);
					
				vm_core.FlushPreIssueRegs();
				vm_core.SetProgramCounter(instr.pc + 4);
			}
		}
	}
}


void DualIssueStages::WriteBackCsr(DualIssueInstrContext& wb_instruction, DualIssueCore& vm_core){
    using instruction_set::get_instr_encoding;
    using instruction_set::Instruction;

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