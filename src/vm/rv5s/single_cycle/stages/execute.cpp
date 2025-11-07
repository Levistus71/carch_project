#include "vm/rv5s/single_cycle/stages/stages.h"
#include "common/instructions.h"

namespace rv5s{

void SingleCycleStages::Execute(SingleCycleCore& vm_core) {
	if (instruction_set::isFInstruction(vm_core.instr.instruction)) { // RV64 F
		ExecuteFloat(vm_core);
		return;
	}
	else if (instruction_set::isDInstruction(vm_core.instr.instruction)) {
		ExecuteDouble(vm_core);
		return;
	}
	else {
		ExecuteBasic(vm_core);
	}

	// branch cheking
	if (vm_core.instr.branch)
		ResolveBranch(vm_core);
}

void SingleCycleStages::ResolveBranch(SingleCycleCore& vm_core){
    using instruction_set::Instruction;
    using instruction_set::get_instr_encoding;

	uint8_t& opcode = vm_core.instr.opcode;
	uint8_t& funct3 = vm_core.instr.funct3;

	if (opcode==get_instr_encoding(Instruction::kjalr).opcode || 
			opcode==get_instr_encoding(Instruction::kjal).opcode) {
		
		if (opcode==get_instr_encoding(Instruction::kjalr).opcode) { 
			vm_core.SetProgramCounter(vm_core.instr.alu_out);
		}
		else if (opcode==get_instr_encoding(Instruction::kjal).opcode) {
			vm_core.SetProgramCounter(vm_core.instr.pc + vm_core.instr.immediate);
		}

		// storing the current value of pc for returning (storing it in rd)
		vm_core.instr.alu_out = vm_core.instr.pc + 4;

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
				branch_flag = (vm_core.instr.alu_out==0);
				break;
			}
			case 0b001: {// BNE
				branch_flag = (vm_core.instr.alu_out!=0);
				break;
			}
			case 0b100: {// BLT
				branch_flag = (vm_core.instr.alu_out==1);
				break;
			}
			case 0b101: {// BGE
				branch_flag = (vm_core.instr.alu_out==0);
				break;
			}
			case 0b110: {// BLTU
				branch_flag = (vm_core.instr.alu_out==1);
				break;
			}
			case 0b111: {// BGEU
				branch_flag = (vm_core.instr.alu_out==0);
				break;
			}
		}

		if (branch_flag) {
			// // Subtracting 4 from pc (updated in Fetch())
			// vm_core.AddToProgramCounter(-4);
			// vm_core.AddToProgramCounter(vm_core.instr.immediate);

			vm_core.SetProgramCounter(vm_core.instr.pc + vm_core.instr.immediate);
		}
	}
}


void SingleCycleStages::ExecuteBasic(SingleCycleCore& vm_core){
	// register values might change (if imm_to_alu is true etc), so not by reference
	uint64_t reg1_value = vm_core.instr.rs1_value;
	uint64_t reg2_value = vm_core.instr.rs2_value;
	
	if (vm_core.instr.imm_to_alu) {
		int32_t imm = vm_core.instr.immediate;
    	reg2_value = static_cast<uint64_t>(static_cast<int64_t>(imm));
  	}

	if(vm_core.instr.auipc){
		reg1_value = vm_core.instr.pc;
	}

  	// std::tie(vm_core.instr.alu_out, vm_core.instr.alu_overflow) = alu_.execute(vm_core.instr.alu_op, reg1_value, reg2_value);
	auto [alu_out_temp, alu_overflow_temp] = vm_core.alu_.execute(vm_core.instr.alu_op, reg1_value, reg2_value);
	vm_core.instr.alu_out = alu_out_temp;
	vm_core.instr.alu_overflow = alu_overflow_temp;
}


void SingleCycleStages::ExecuteFloat(SingleCycleCore& vm_core){
	uint8_t& funct3 = vm_core.instr.funct3;
	uint8_t rm = funct3;

	uint8_t fcsr_status = 0;

	int32_t imm = vm_core.instr.immediate;

	// FIXME?: what is this
	if (rm==0b111) {
		rm = vm_core.register_file_.ReadCsr(0x002);
	}

	uint64_t reg1_value = (vm_core.instr.rs1_from_fprf) ? vm_core.instr.frs1_value : vm_core.instr.rs1_value;
	uint64_t reg2_value = vm_core.instr.frs2_value;
	uint64_t reg3_value = vm_core.instr.frs3_value;


	if (vm_core.instr.imm_to_alu) {
		reg2_value = static_cast<uint64_t>(static_cast<int64_t>(imm));
	}

	alu::AluOp aluOperation = vm_core.instr.alu_op;
	// std::tie(execution_result_, fcsr_status) = alu::Alu::fpexecute(aluOperation, reg1_value, reg2_value, reg3_value, rm);
	auto [alu_out_temp, fcsr_status_temp] = alu::Alu::fpexecute(aluOperation, reg1_value, reg2_value, reg3_value, rm);
	vm_core.instr.alu_out = alu_out_temp;
	fcsr_status = fcsr_status_temp;

	// WHAT?: what is this
	vm_core.instr.fcsr_update = true;
	vm_core.instr.fcsr_status = fcsr_status;
}


void SingleCycleStages::ExecuteDouble(SingleCycleCore& vm_core){
	uint8_t& opcode = vm_core.instr.opcode;
	uint8_t& funct3 = vm_core.instr.funct3;
	uint8_t& funct7 = vm_core.instr.funct7;
	uint8_t rm = funct3;

	// WHAT? : this fcsr_status is not used? Same as ExecuteFloat() i suppose
	uint8_t fcsr_status = 0;

	int32_t imm = vm_core.instr.immediate;

	uint64_t reg1_value = vm_core.instr.frs1_value;
	uint64_t reg2_value = vm_core.instr.frs2_value;
	uint64_t reg3_value = vm_core.instr.frs3_value;

	if (funct7==0b1101001 || funct7==0b1111001 || opcode==0b0000111 || opcode==0b0100111) {
		reg1_value = vm_core.instr.rs1_value;
	}

	if (vm_core.instr.imm_to_alu) {
		reg2_value = static_cast<uint64_t>(static_cast<int64_t>(imm));
	}

	alu::AluOp aluOperation = vm_core.instr.alu_op;
	// std::tie(execution_result_, fcsr_status) = alu::Alu::dfpexecute(aluOperation, reg1_value, reg2_value, reg3_value, rm);
	auto [alu_out_temp, fcsr_status_temp] = alu::Alu::dfpexecute(aluOperation, reg1_value, reg2_value, reg3_value, rm);
	vm_core.instr.alu_out = alu_out_temp;
	fcsr_status = fcsr_status_temp;

	// WHAT?: what is this
	vm_core.instr.fcsr_update = true;
	vm_core.instr.fcsr_status = fcsr_status;
}

} // namespace rv5s