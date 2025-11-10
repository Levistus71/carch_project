#include "vm/rv5s/pipelined/stages/stages.h"
#include "common/instructions.h"

namespace rv5s{

void PipelinedStages::Execute(PipelinedCore& vm_core) {
    PipelinedInstrContext& ex_instruction = vm_core.GetExInstruction();
	if(ex_instruction.nopped)
		return;

	if (instruction_set::isFInstruction(ex_instruction.instruction)) { // RV64 F
		ExecuteFloat(vm_core);
		return;
	}
	else if (instruction_set::isDInstruction(ex_instruction.instruction)) {
		ExecuteDouble(vm_core);
		return;
	}
	else {
		ExecuteBasic(vm_core);
	}

	// branch cheking
	if (ex_instruction.branch)
		ResolveBranch(vm_core);
}

void PipelinedStages::ResolveBranch(PipelinedCore& vm_core){
    using instruction_set::Instruction;
    using instruction_set::get_instr_encoding;

	vm_core.core_stats_.branch_instrs++;

	PipelinedInstrContext& ex_instruction = vm_core.GetExInstruction();
	uint8_t& opcode = ex_instruction.opcode;
	uint8_t& funct3 = ex_instruction.funct3;

	if (opcode==get_instr_encoding(Instruction::kjalr).opcode || 
			opcode==get_instr_encoding(Instruction::kjal).opcode) {

		// updating branch_taken status
		ex_instruction.branch_taken = true;
		
		// if branch was already taken, we skip updating the pc
		if(ex_instruction.branch_predicted_taken)
			return;

		vm_core.core_stats_.branch_mispredicts++;
		
		if (opcode==get_instr_encoding(Instruction::kjalr).opcode) { 
			vm_core.SetProgramCounter(ex_instruction.alu_out);
		}
		else if (opcode==get_instr_encoding(Instruction::kjal).opcode) {
			vm_core.SetProgramCounter(ex_instruction.pc + ex_instruction.immediate);
		}

		// storing the current value of pc for returning (storing it in rd)
		ex_instruction.alu_out = ex_instruction.pc + 4;

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

			// if branch was predicted taken, we skip updating pc
			if(ex_instruction.branch_predicted_taken)
				return;
			
			vm_core.core_stats_.branch_mispredicts++;

			// // Subtracting 4 from pc (updated in Fetch())
			// vm_core.AddToProgramCounter(-4);
			// vm_core.AddToProgramCounter(ex_instruction.immediate);

			vm_core.SetProgramCounter(ex_instruction.pc + ex_instruction.immediate);
		}
		else{
			// branch was incorrectly predicted, need to set the pc to pc+4
			if(ex_instruction.branch_predicted_taken){
				vm_core.core_stats_.branch_mispredicts++;
				vm_core.SetProgramCounter(ex_instruction.pc + 4);
			}
		}
	}
}


void PipelinedStages::ExecuteBasic(PipelinedCore& vm_core){
    PipelinedInstrContext& ex_instruction = vm_core.GetExInstruction();
	// register values might change (if imm_to_alu is true etc), so not by reference
	uint64_t reg1_value = ex_instruction.rs1_value;
	uint64_t reg2_value = ex_instruction.rs2_value;
	
	if (ex_instruction.imm_to_alu) {
		int32_t imm = ex_instruction.immediate;
    	reg2_value = static_cast<uint64_t>(static_cast<int64_t>(imm));
  	}

	if(ex_instruction.auipc){
		reg1_value = ex_instruction.pc;
	}

  	// std::tie(ex_instruction.alu_out, ex_instruction.alu_overflow) = alu_.execute(ex_instruction.alu_op, reg1_value, reg2_value);
	auto [alu_out_temp, alu_overflow_temp] = vm_core.alu_.execute(ex_instruction.alu_op, reg1_value, reg2_value);
	ex_instruction.alu_out = alu_out_temp;
	ex_instruction.alu_overflow = alu_overflow_temp;
}


void PipelinedStages::ExecuteFloat(PipelinedCore& vm_core){
    PipelinedInstrContext& ex_instruction = vm_core.GetExInstruction();
	uint8_t& funct3 = ex_instruction.funct3;
	uint8_t rm = funct3;

	uint8_t fcsr_status = 0;

	int32_t imm = ex_instruction.immediate;

	// FIXME?: what is this
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

	// WHAT?: what is this
	ex_instruction.fcsr_update = true;
	ex_instruction.fcsr_status = fcsr_status;
}


void PipelinedStages::ExecuteDouble(PipelinedCore& vm_core){
    PipelinedInstrContext& ex_instruction = vm_core.GetExInstruction();
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

} // namespace rv5s