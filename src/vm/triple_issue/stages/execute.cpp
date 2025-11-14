#include "vm/triple_issue/stages/stages.h"

namespace triple_issue
{

void TripleIssueStages::ExecuteAlu(TripleIssueCore& vm_core){
    dual_issue::DualIssueInstrContext& ex_instruction = vm_core.pipeline_reg_instrs_.rsrvstn_alu;
    if(ex_instruction.illegal){
        return;
    }
    ExecuteBasic(vm_core, ex_instruction);
}

void TripleIssueStages::ExecuteBasic(TripleIssueCore& vm_core, dual_issue::DualIssueInstrContext& instr){
	// register values might change (if imm_to_alu is true etc), so not by reference
	uint64_t reg1_value = instr.rs1_value;
	uint64_t reg2_value = instr.rs2_value;
	
	if (instr.imm_to_alu) {
		int32_t imm = instr.immediate;
    	reg2_value = static_cast<uint64_t>(static_cast<int64_t>(imm));
  	}

	if(instr.auipc){
		reg1_value = instr.pc;
	}

	auto [alu_out_temp, alu_overflow_temp] = vm_core.alu_.execute(instr.alu_op, reg1_value, reg2_value);
	instr.alu_out = alu_out_temp;
	instr.alu_overflow = alu_overflow_temp;
}


void TripleIssueStages::ExecuteFalu(TripleIssueCore& vm_core){
    dual_issue::DualIssueInstrContext& ex_instruction = vm_core.pipeline_reg_instrs_.rsrvstn_falu;
    if (instruction_set::isFInstruction(ex_instruction.instruction)) { // RV64 F
		ExecuteFloat(vm_core, ex_instruction);
		return;
	}
	else if (instruction_set::isDInstruction(ex_instruction.instruction)) {
		ExecuteDouble(vm_core, ex_instruction);
		return;
	}
}

void TripleIssueStages::ExecuteFloat(TripleIssueCore& vm_core, dual_issue::DualIssueInstrContext& ex_instruction){
	uint8_t& funct3 = ex_instruction.funct3;
	uint8_t rm = funct3;

	uint8_t fcsr_status = 0;

	int32_t imm = ex_instruction.immediate;

	if (rm==0b111) {
		rm = vm_core.register_file_.ReadCsr(0x002);
	}

	uint64_t reg1_value = (ex_instruction.rs1_from_fprf) ? ex_instruction.frs1_value : ex_instruction.rs1_value;
	uint64_t reg2_value = ex_instruction.frs2_value;
	uint64_t reg3_value = ex_instruction.frs3_value;


	if (ex_instruction.imm_to_alu) {
		reg2_value = static_cast<uint64_t>(static_cast<int64_t>(imm));
	}

	alu::AluOp aluOperation = ex_instruction.alu_op;
	// std::tie(execution_result_, fcsr_status) = alu::Alu::fpexecute(aluOperation, reg1_value, reg2_value, reg3_value, rm);
	auto [alu_out_temp, fcsr_status_temp] = alu::Alu::fpexecute(aluOperation, reg1_value, reg2_value, reg3_value, rm);
	ex_instruction.alu_out = alu_out_temp;
	fcsr_status = fcsr_status_temp;

	ex_instruction.fcsr_update = true;
	ex_instruction.fcsr_status = fcsr_status;
}



void TripleIssueStages::ExecuteDouble(TripleIssueCore& vm_core, dual_issue::DualIssueInstrContext& ex_instruction){
	uint8_t& opcode = ex_instruction.opcode;
	uint8_t& funct3 = ex_instruction.funct3;
	uint8_t& funct7 = ex_instruction.funct7;
	uint8_t rm = funct3;

	// WHAT? : this fcsr_status is not used? Same as ExecuteFloat() i suppose
	uint8_t fcsr_status = 0;

	int32_t imm = ex_instruction.immediate;

	uint64_t reg1_value = ex_instruction.frs1_value;
	uint64_t reg2_value = ex_instruction.frs2_value;
	uint64_t reg3_value = ex_instruction.frs3_value;

	if (funct7==0b1101001 || funct7==0b1111001 || opcode==0b0000111 || opcode==0b0100111) {
		reg1_value = ex_instruction.rs1_value;
	}

	if (ex_instruction.imm_to_alu) {
		reg2_value = static_cast<uint64_t>(static_cast<int64_t>(imm));
	}

	alu::AluOp aluOperation = ex_instruction.alu_op;
	// std::tie(execution_result_, fcsr_status) = alu::Alu::dfpexecute(aluOperation, reg1_value, reg2_value, reg3_value, rm);
	auto [alu_out_temp, fcsr_status_temp] = alu::Alu::dfpexecute(aluOperation, reg1_value, reg2_value, reg3_value, rm);
	ex_instruction.alu_out = alu_out_temp;
	fcsr_status = fcsr_status_temp;

	// WHAT?: what is this
	ex_instruction.fcsr_update = true;
	ex_instruction.fcsr_status = fcsr_status;
}


} // namespace triple_issue
