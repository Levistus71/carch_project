#include "vm/rv5s/rv5s_vm.h"

using instruction_set::Instruction;
using instruction_set::get_instr_encoding;

void RV5SVM::Execute() {
	uint8_t& opcode = this->ex_instruction->opcode;
	uint8_t& funct3 = this->ex_instruction->funct3;

	if (instruction_set::isFInstruction(this->ex_instruction->instruction)) { // RV64 F
		ExecuteFloat();
		return;
	}
	else if (instruction_set::isDInstruction(this->ex_instruction->instruction)) {
		ExecuteDouble();
		return;
	}
	else if (opcode==0b1110011) {
		ExecuteCsr();
		return;
	}
	else {
		ExecuteBasic();
	}

	// branch cheking
	if (this->ex_instruction->branch)
		ResolveBranch();
}


void RV5SVM::DebugExecute(){
	Execute();
}

void RV5SVM::ResolveBranch(){
	uint8_t& opcode = this->ex_instruction->opcode;
	uint8_t& funct3 = this->ex_instruction->funct3;

	if (opcode==get_instr_encoding(Instruction::kjalr).opcode || 
			opcode==get_instr_encoding(Instruction::kjal).opcode) {
		
		// storing the current value of pc for returning (storing it in rd)
		this->ex_instruction->alu_out = this->program_counter_;

		// subtracting 4 from pc (updated in Fetch())
		UpdateProgramCounter(-4);
		
		if (opcode==get_instr_encoding(Instruction::kjalr).opcode) { 
			UpdateProgramCounter(this->ex_instruction->alu_out - this->program_counter_);
		}
		else if (opcode==get_instr_encoding(Instruction::kjal).opcode) {
			UpdateProgramCounter(this->ex_instruction->immediate);
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
				branch_flag = (this->ex_instruction->alu_out==0);
				break;
			}
			case 0b001: {// BNE
				branch_flag = (this->ex_instruction->alu_out!=0);
				break;
			}
			case 0b100: {// BLT
				branch_flag = (this->ex_instruction->alu_out==1);
				break;
			}
			case 0b101: {// BGE
				branch_flag = (this->ex_instruction->alu_out==0);
				break;
			}
			case 0b110: {// BLTU
				branch_flag = (this->ex_instruction->alu_out==1);
				break;
			}
			case 0b111: {// BGEU
				branch_flag = (this->ex_instruction->alu_out==0);
				break;
			}
		}

		if (branch_flag) {
			// Subtracting 4 from pc (updated in Fetch())
			UpdateProgramCounter(-4);
			UpdateProgramCounter(this->ex_instruction->immediate);
		}
	}
}



void RV5SVM::ExecuteBasic(){
	uint64_t& reg1_value = this->ex_instruction->rs1_value;
	uint64_t& reg2_value = this->ex_instruction->rs2_value;
	
	if (this->ex_instruction->imm_to_alu) {
		int32_t& imm = this->ex_instruction->immediate;
    	reg2_value = static_cast<uint64_t>(static_cast<int64_t>(imm));
  	}

	if(this->ex_instruction->auipc){
		reg1_value = this->ex_instruction->pc;
	}

  	// std::tie(this->ex_instruction->alu_out, this->ex_instruction->alu_overflow) = alu_.execute(this->ex_instruction->alu_op, reg1_value, reg2_value);
	auto [alu_out_temp, alu_overflow_temp] = alu_.execute(this->ex_instruction->alu_op, reg1_value, reg2_value);
	this->ex_instruction->alu_out = alu_out_temp;
	this->ex_instruction->alu_overflow = alu_overflow_temp;
}

void RV5SVM::ExecuteFloat() {
	uint8_t& opcode = this->ex_instruction->opcode;
	uint8_t& funct3 = this->ex_instruction->funct3;
	uint8_t& funct7 = this->ex_instruction->funct7;
	uint8_t rm = funct3;

	uint8_t fcsr_status = 0;

	int32_t& imm = this->ex_instruction->immediate;

	// WHAT?: what is this
	if (rm==0b111) {
		rm = registers_.ReadCsr(0x002);
	}

	uint64_t& reg1_value = this->ex_instruction->frs1_value;
	uint64_t& reg2_value = this->ex_instruction->frs2_value;
	uint64_t& reg3_value = this->ex_instruction->frs3_value;

	if (funct7==0b1101000 || funct7==0b1111000 || opcode==0b0000111 || opcode==0b0100111) {
		reg1_value = this->ex_instruction->rs1_value;
	}

	if (this->ex_instruction->imm_to_alu) {
		reg2_value = static_cast<uint64_t>(static_cast<int64_t>(imm));
	}

	alu::AluOp aluOperation = this->ex_instruction->alu_op;
	// std::tie(execution_result_, fcsr_status) = alu::Alu::fpexecute(aluOperation, reg1_value, reg2_value, reg3_value, rm);
	auto [alu_out_temp, fcsr_status_temp] = alu::Alu::fpexecute(aluOperation, reg1_value, reg2_value, reg3_value, rm);
	this->ex_instruction->alu_out = alu_out_temp;
	fcsr_status = fcsr_status_temp;

	// WHAT?: what is this
	registers_.WriteCsr(0x003, fcsr_status);
}

void RV5SVM::ExecuteDouble() {
	uint8_t& opcode = this->ex_instruction->opcode;
	uint8_t& funct3 = this->ex_instruction->funct3;
	uint8_t& funct7 = this->ex_instruction->funct7;
	uint8_t rm = funct3;

	// WHAT? : this fcsr_status is not used?
	uint8_t fcsr_status = 0;

	int32_t imm = this->ex_instruction->immediate;

	uint64_t reg1_value = this->ex_instruction->frs1_value;
	uint64_t reg2_value = this->ex_instruction->frs1_value;
	uint64_t reg3_value = this->ex_instruction->frs1_value;

	if (funct7==0b1101001 || funct7==0b1111001 || opcode==0b0000111 || opcode==0b0100111) {
		reg1_value = this->ex_instruction->rs1_value;
	}

	if (this->ex_instruction->imm_to_alu) {
		reg2_value = static_cast<uint64_t>(static_cast<int64_t>(imm));
	}

	alu::AluOp aluOperation = this->ex_instruction->alu_op;
	// std::tie(execution_result_, fcsr_status) = alu::Alu::dfpexecute(aluOperation, reg1_value, reg2_value, reg3_value, rm);
	auto [alu_out_temp, fcsr_status_temp] = alu::Alu::dfpexecute(aluOperation, reg1_value, reg2_value, reg3_value, rm);
	this->ex_instruction->alu_out = alu_out_temp;
}

// WHAT? what is this
void RV5SVM::ExecuteCsr() {
  uint8_t& rs1 = this->ex_instruction->rs1;
  uint16_t csr = (this->ex_instruction->instruction >> 20) & 0xFFF;
  uint64_t csr_val = registers_.ReadCsr(csr);

  csr_target_address_ = csr;
  csr_old_value_ = csr_val;
  csr_write_val_ = registers_.ReadGpr(rs1);
  csr_uimm_ = rs1;
}


// TODO: implement this
void RV5SVM::HandleSyscall() {
	std::cout << "Syscalls are not enabled" << std::endl;
}