#include "vm/rv5s/rv5s_vm.h"

using instruction_set::Instruction;
using instruction_set::get_instr_encoding;

void RV5SVM::Execute() {
	InstrContext& ex_instruction = GetExInstruction();
	uint8_t& opcode = ex_instruction.opcode;
	uint8_t& funct3 = ex_instruction.funct3;

	// TODO: modify this. alu shouldn't check anything
	if (instruction_set::isFInstruction(ex_instruction.instruction)) { // RV64 F
		ExecuteFloat();
		return;
	}
	else if (instruction_set::isDInstruction(ex_instruction.instruction)) {
		ExecuteDouble();
		return;
	}
	else {
		ExecuteBasic();
	}

	// branch cheking
	if (ex_instruction.branch)
		ResolveBranch();
}

void RV5SVM::ResolveBranch(){
	InstrContext& ex_instruction = GetExInstruction();
	uint8_t& opcode = ex_instruction.opcode;
	uint8_t& funct3 = ex_instruction.funct3;

	if (opcode==get_instr_encoding(Instruction::kjalr).opcode || 
			opcode==get_instr_encoding(Instruction::kjal).opcode) {

		// updating branch_taken status
		ex_instruction.branch_taken = true;
		
		// storing the current value of pc for returning (storing it in rd)
		ex_instruction.alu_out = this->program_counter_;

		// subtracting 4 from pc (updated in Fetch())
		AddToProgramCounter(-4);
		
		if (opcode==get_instr_encoding(Instruction::kjalr).opcode) { 
			SetProgramCounter(ex_instruction.alu_out);
		}
		else if (opcode==get_instr_encoding(Instruction::kjal).opcode) {
			AddToProgramCounter(ex_instruction.immediate);
		}
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
				branch_flag = (ex_instruction.alu_out==0);
				break;
			}
			case 0b001: {// BNE
				branch_flag = (ex_instruction.alu_out!=0);
				break;
			}
			case 0b100: {// BLT
				branch_flag = (ex_instruction.alu_out==1);
				break;
			}
			case 0b101: {// BGE
				branch_flag = (ex_instruction.alu_out==0);
				break;
			}
			case 0b110: {// BLTU
				branch_flag = (ex_instruction.alu_out==1);
				break;
			}
			case 0b111: {// BGEU
				branch_flag = (ex_instruction.alu_out==0);
				break;
			}
		}

		if (branch_flag) {
			// Updating branch_taken status
			ex_instruction.branch_taken = true;

			// Subtracting 4 from pc (updated in Fetch())
			AddToProgramCounter(-4);
			AddToProgramCounter(ex_instruction.immediate);
		}
	}
}



void RV5SVM::ExecuteBasic(){
	InstrContext& ex_instruction = GetExInstruction();
	uint64_t& reg1_value = ex_instruction.rs1_value;
	uint64_t& reg2_value = ex_instruction.rs2_value;
	
	if (ex_instruction.imm_to_alu) {
		int32_t& imm = ex_instruction.immediate;
    	reg2_value = static_cast<uint64_t>(static_cast<int64_t>(imm));
  	}

	if(ex_instruction.auipc){
		reg1_value = ex_instruction.pc;
	}

  	// std::tie(ex_instruction.alu_out, ex_instruction.alu_overflow) = alu_.execute(ex_instruction.alu_op, reg1_value, reg2_value);
	auto [alu_out_temp, alu_overflow_temp] = alu_.execute(ex_instruction.alu_op, reg1_value, reg2_value);
	ex_instruction.alu_out = alu_out_temp;
	ex_instruction.alu_overflow = alu_overflow_temp;
}

void RV5SVM::ExecuteFloat() {
	InstrContext& ex_instruction = GetExInstruction();
	uint8_t& opcode = ex_instruction.opcode;
	uint8_t& funct3 = ex_instruction.funct3;
	uint8_t& funct7 = ex_instruction.funct7;
	uint8_t rm = funct3;

	uint8_t fcsr_status = 0;

	int32_t& imm = ex_instruction.immediate;

	// FIXME?: what is this
	if (rm==0b111) {
		rm = registers_.ReadCsr(0x002);
	}

	uint64_t& reg1_value = (ex_instruction.rs1_from_fprf) ? ex_instruction.frs1_value : ex_instruction.rs1_value;
	uint64_t& reg2_value = ex_instruction.frs2_value;
	uint64_t& reg3_value = ex_instruction.frs3_value;


	if (ex_instruction.imm_to_alu) {
		reg2_value = static_cast<uint64_t>(static_cast<int64_t>(imm));
	}

	alu::AluOp aluOperation = ex_instruction.alu_op;
	// std::tie(execution_result_, fcsr_status) = alu::Alu::fpexecute(aluOperation, reg1_value, reg2_value, reg3_value, rm);
	auto [alu_out_temp, fcsr_status_temp] = alu::Alu::fpexecute(aluOperation, reg1_value, reg2_value, reg3_value, rm);
	ex_instruction.alu_out = alu_out_temp;
	fcsr_status = fcsr_status_temp;

	// WHAT?: what is this
	ex_instruction.fcsr_update = true;
	ex_instruction.fcsr_status = fcsr_status;
}

void RV5SVM::ExecuteDouble() {
	InstrContext& ex_instruction = GetExInstruction();
	uint8_t& opcode = ex_instruction.opcode;
	uint8_t& funct3 = ex_instruction.funct3;
	uint8_t& funct7 = ex_instruction.funct7;
	uint8_t rm = funct3;

	// WHAT? : this fcsr_status is not used? Same as ExecuteFloat() i suppose
	uint8_t fcsr_status = 0;

	int32_t imm = ex_instruction.immediate;

	uint64_t reg1_value = ex_instruction.frs1_value;
	uint64_t reg2_value = ex_instruction.frs1_value;
	uint64_t reg3_value = ex_instruction.frs1_value;

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