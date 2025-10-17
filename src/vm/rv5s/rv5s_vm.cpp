/**
 * @file rvss_vm.cpp
 * @brief RVSS VM implementation
 * @author Vishank Singh, https://github.com/VishankSingh
 */

#include "vm/rv5s/rv5s_vm.h"

#include "utils.h"
#include "globals.h"
#include "common/instructions.h"
#include "config.h"

#include <cctype>
#include <cstdint>
#include <iostream>
#include <tuple>
#include <stack>  
#include <algorithm>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

using instruction_set::Instruction;
using instruction_set::get_instr_encoding;


RV5SVM::RV5SVM() : VmBase() {
  DumpRegisters(globals::registers_dump_file_path, registers_);
  DumpState(globals::vm_state_dump_file_path);

  pipelining_enabled = vm_config::config.getPipeliningStatus();
  hazard_detection_enabled = vm_config::config.getHazardDetectionStatus();
  data_forwarding_enabled = vm_config::config.getDataFowardingStatus();
}

RV5SVM::~RV5SVM() = default;

void RV5SVM::Fetch() {
  	this->if_instruction->instruction = this->memory_controller_.ReadWord(program_counter_);
  	UpdateProgramCounter(4);
}

void RV5SVM::Decode() {
	decode_unit.DecodeInstruction(*(this->id_instruction), registers_);

	if (this->id_instruction->opcode == get_instr_encoding(Instruction::kecall).opcode && 
		this->id_instruction->funct3 == get_instr_encoding(Instruction::kecall).funct3) {
		// TODO: flush all instructions, start fresh at pc = pc+4
		HandleSyscall();
		return;
	}
}

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
				branch_flag = (execution_result_==0);
				break;
			}
			case 0b001: {// BNE
				branch_flag = (execution_result_!=0);
				break;
			}
			case 0b100: {// BLT
				branch_flag = (execution_result_==1);
				break;
			}
			case 0b101: {// BGE
				branch_flag = (execution_result_==0);
				break;
			}
			case 0b110: {// BLTU
				branch_flag = (execution_result_==1);
				break;
			}
			case 0b111: {// BGEU
				branch_flag = (execution_result_==0);
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
	auto [alu_out_temp, fcsr_status] = alu::Alu::fpexecute(aluOperation, reg1_value, reg2_value, reg3_value, rm);
	this->ex_instruction->alu_out = alu_out_temp;

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
	auto [alu_out_temp, fcsr_status] = alu::Alu::dfpexecute(aluOperation, reg1_value, reg2_value, reg3_value, rm);
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

// TODO: implement writeback for syscalls
void RV5SVM::HandleSyscall() {
  uint64_t syscall_number = registers_.ReadGpr(17);
  switch (syscall_number) {
    case SYSCALL_PRINT_INT: {
        if (!globals::vm_as_backend) {
            std::cout << "[Syscall output: ";
        } else {
          std::cout << "VM_STDOUT_START";
        }
        std::cout << static_cast<int64_t>(registers_.ReadGpr(10)); // Print signed integer
        if (!globals::vm_as_backend) {
            std::cout << "]" << std::endl;
        } else {
          std::cout << "VM_STDOUT_END" << std::endl;
        }
        break;
    }
    case SYSCALL_PRINT_FLOAT: { // print float
        if (!globals::vm_as_backend) {
            std::cout << "[Syscall output: ";
        } else {
          std::cout << "VM_STDOUT_START";
        }
        float float_value;
        uint64_t raw = registers_.ReadGpr(10);
        std::memcpy(&float_value, &raw, sizeof(float_value));
        std::cout << std::setprecision(std::numeric_limits<float>::max_digits10) << float_value;
        if (!globals::vm_as_backend) {
            std::cout << "]" << std::endl;
        } else {
          std::cout << "VM_STDOUT_END" << std::endl;
        }
        break;
    }
    case SYSCALL_PRINT_DOUBLE: { // print double
        if (!globals::vm_as_backend) {
            std::cout << "[Syscall output: ";
        } else {
          std::cout << "VM_STDOUT_START";
        }
        double double_value;
        uint64_t raw = registers_.ReadGpr(10);
        std::memcpy(&double_value, &raw, sizeof(double_value));
        std::cout << std::setprecision(std::numeric_limits<double>::max_digits10) << double_value;
        if (!globals::vm_as_backend) {
            std::cout << "]" << std::endl;
        } else {
          std::cout << "VM_STDOUT_END" << std::endl;
        }
        break;
    }
    case SYSCALL_PRINT_STRING: {
        if (!globals::vm_as_backend) {
            std::cout << "[Syscall output: ";
        }
        PrintString(registers_.ReadGpr(10)); // Print string
        if (!globals::vm_as_backend) {
            std::cout << "]" << std::endl;
        }
        break;
    }
    case SYSCALL_EXIT: {
        stop_requested_ = true; // Stop the VM
        if (!globals::vm_as_backend) {
            std::cout << "VM_EXIT" << std::endl;
        }
        output_status_ = "VM_EXIT";
        std::cout << "Exited with exit code: " << registers_.ReadGpr(10) << std::endl;
        exit(0); // Exit the program
        break;
    }
    case SYSCALL_READ: { // Read
      uint64_t file_descriptor = registers_.ReadGpr(10);
      uint64_t buffer_address = registers_.ReadGpr(11);
      uint64_t length = registers_.ReadGpr(12);

      if (file_descriptor == 0) {
        // Read from stdin
        std::string input;
        {
          std::cout << "VM_STDIN_START" << std::endl;
          output_status_ = "VM_STDIN_START";
          std::unique_lock<std::mutex> lock(input_mutex_);
          input_cv_.wait(lock, [this]() { 
            return !input_queue_.empty(); 
          });
          output_status_ = "VM_STDIN_END";
          std::cout << "VM_STDIN_END" << std::endl;

          input = input_queue_.front();
          input_queue_.pop();
        }


        std::vector<uint8_t> old_bytes_vec(length, 0);
        std::vector<uint8_t> new_bytes_vec(length, 0);

        for (size_t i = 0; i < length; ++i) {
          old_bytes_vec[i] = memory_controller_.ReadByte(buffer_address + i);
        }
        
        for (size_t i = 0; i < input.size() && i < length; ++i) {
          memory_controller_.WriteByte(buffer_address + i, static_cast<uint8_t>(input[i]));
        }
        if (input.size() < length) {
          memory_controller_.WriteByte(buffer_address + input.size(), '\0');
        }

        for (size_t i = 0; i < length; ++i) {
          new_bytes_vec[i] = memory_controller_.ReadByte(buffer_address + i);
        }

        current_delta_.memory_changes.push_back({
          buffer_address, 
          old_bytes_vec, 
          new_bytes_vec
        });

        uint64_t old_reg = registers_.ReadGpr(10);
        unsigned int reg_index = 10;
        unsigned int reg_type = 0; // 0 for GPR, 1 for CSR, 2 for FPR
        uint64_t new_reg = std::min(static_cast<uint64_t>(length), static_cast<uint64_t>(input.size()));
        registers_.WriteGpr(10, new_reg); 
        if (old_reg != new_reg) {
          current_delta_.register_changes.push_back({reg_index, reg_type, old_reg, new_reg});
        }

      } else {
          std::cerr << "Unsupported file descriptor: " << file_descriptor << std::endl;
      }
      break;
    }
    case SYSCALL_WRITE: { // Write
        uint64_t file_descriptor = registers_.ReadGpr(10);
        uint64_t buffer_address = registers_.ReadGpr(11);
        uint64_t length = registers_.ReadGpr(12);

        if (file_descriptor == 1) { // stdout
          std::cout << "VM_STDOUT_START";
          output_status_ = "VM_STDOUT_START";
          uint64_t bytes_printed = 0;
          for (uint64_t i = 0; i < length; ++i) {
              char c = memory_controller_.ReadByte(buffer_address + i);
              // if (c == '\0') {
              //     break;
              // }
              std::cout << c;
              bytes_printed++;
          }
          std::cout << std::flush; 
          output_status_ = "VM_STDOUT_END";
          std::cout << "VM_STDOUT_END" << std::endl;

          uint64_t old_reg = registers_.ReadGpr(10);
          unsigned int reg_index = 10;
          unsigned int reg_type = 0; // 0 for GPR, 1 for CSR, 2 for FPR
          uint64_t new_reg = std::min(static_cast<uint64_t>(length), bytes_printed);
          registers_.WriteGpr(10, new_reg);
          if (old_reg != new_reg) {
            current_delta_.register_changes.push_back({reg_index, reg_type, old_reg, new_reg});
          }
        } else {
            std::cerr << "Unsupported file descriptor: " << file_descriptor << std::endl;
        }
        break;
    }
    default: {
      std::cerr << "Unknown syscall number: " << syscall_number << std::endl;
      break;
    }
  }
}

void RV5SVM::MemoryAccess() {
	auto sign_extend = [](uint64_t value, unsigned int bits) -> uint64_t {
		if (bits >= 64) {
			return value;
		}
		
		unsigned int shift = 64 - bits;

		int64_t signed_value = static_cast<int64_t>(value);
		signed_value <<= shift;
		signed_value >>= shift;

		return static_cast<uint64_t>(signed_value);
	};

	if (this->mem_instruction->mem_read) {
		uint64_t& mem_out = this->mem_instruction->mem_out;
		uint64_t& address = this->mem_instruction->alu_out;
		mem_out = 0;

		for(size_t i=0;i<this->mem_instruction->mem_access_bytes;i++){
			mem_out += memory_controller_.ReadByte(address+i) << (i*8);
		}
		
		if(this->mem_instruction->sign_extend){
			mem_out = sign_extend(mem_out, this->mem_instruction->mem_access_bytes*8);
		}
	}

	if (this->mem_instruction->mem_write) {
		uint64_t& address = this->mem_instruction->alu_out;

		uint64_t write_data = (this->mem_instruction->mem_write_data_from_gpr) ? 
			this->mem_instruction->rs2_value :
			this->mem_instruction->frs2_value;			

		for(size_t i=0;i<this->mem_instruction->mem_access_bytes;i++){
			memory_controller_.WriteByte(address + i, 0xFF & (write_data >> (i*8)));
		}
	}
}

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

// XXX: CONTINUE FROM HERE
// TODO: implement pipelining (multiple instructions in this function, data forwarding, hazard detection unit, branch prediction)
void RVSSVM::Run() {
  ClearStop();
  uint64_t instruction_executed = 0;

  while (!stop_requested_ && program_counter_ < program_size_) {
    // this line mannn. why???
    if (instruction_executed > vm_config::config.getInstructionExecutionLimit())
      break;


    Fetch();
    Decode();
    Execute();
    // FIXME: why "WriteMemory"? rewrite the below function. should be similar to memory stage in hardware.
    WriteMemory();
    WriteBack();
    instructions_retired_++;
    instruction_executed++;
    cycle_s_++;
    std::cout << "Program Counter: " << program_counter_ << std::endl;
  }
  if (program_counter_ >= program_size_) {
    std::cout << "VM_PROGRAM_END" << std::endl;
    output_status_ = "VM_PROGRAM_END";
  }
  DumpRegisters(globals::registers_dump_file_path, registers_);
  DumpState(globals::vm_state_dump_file_path);
}

void RVSSVM::DebugRun() {
  ClearStop();
  uint64_t instruction_executed = 0;
  while (!stop_requested_ && program_counter_ < program_size_) {
    if (instruction_executed > vm_config::config.getInstructionExecutionLimit())
      break;
    current_delta_.old_pc = program_counter_;
    if (std::find(breakpoints_.begin(), breakpoints_.end(), program_counter_) == breakpoints_.end()) {
      Fetch();
      Decode();
      Execute();
      WriteMemory();
      WriteBack();
      instructions_retired_++;
      instruction_executed++;
      cycle_s_++;
      std::cout << "Program Counter: " << program_counter_ << std::endl;

      current_delta_.new_pc = program_counter_;
      // history_.push(current_delta_);
      undo_stack_.push(current_delta_);
      while (!redo_stack_.empty()) {
        redo_stack_.pop();
      }
      current_delta_ = StepDelta();
      if (program_counter_ < program_size_) {
        std::cout << "VM_STEP_COMPLETED" << std::endl;
        output_status_ = "VM_STEP_COMPLETED";
      } else if (program_counter_ >= program_size_) {
        std::cout << "VM_LAST_INSTRUCTION_STEPPED" << std::endl;
        output_status_ = "VM_LAST_INSTRUCTION_STEPPED";
      }
      DumpRegisters(globals::registers_dump_file_path, registers_);
      DumpState(globals::vm_state_dump_file_path);

      unsigned int delay_ms = vm_config::config.getRunStepDelay();
      std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
      
    } else {
      std::cout << "VM_BREAKPOINT_HIT " << program_counter_ << std::endl;
      output_status_ = "VM_BREAKPOINT_HIT";
      break;
    }
  }
  if (program_counter_ >= program_size_) {
    std::cout << "VM_PROGRAM_END" << std::endl;
    output_status_ = "VM_PROGRAM_END";
  }
  DumpRegisters(globals::registers_dump_file_path, registers_);
  DumpState(globals::vm_state_dump_file_path);
}

void RVSSVM::Step() {
  current_delta_.old_pc = program_counter_;
  if (program_counter_ < program_size_) {
    Fetch();
    Decode();
    Execute();
    WriteMemory();
    WriteBack();
    instructions_retired_++;
    cycle_s_++;
    std::cout << "Program Counter: " << std::hex << program_counter_ << std::dec << std::endl;

    current_delta_.new_pc = program_counter_;

    // history_.push(current_delta_);

    undo_stack_.push(current_delta_);
    while (!redo_stack_.empty()) {
      redo_stack_.pop();
    }

    current_delta_ = StepDelta();


    if (program_counter_ < program_size_) {
      std::cout << "VM_STEP_COMPLETED" << std::endl;
      output_status_ = "VM_STEP_COMPLETED";
    } else if (program_counter_ >= program_size_) {
      std::cout << "VM_LAST_INSTRUCTION_STEPPED" << std::endl;
      output_status_ = "VM_LAST_INSTRUCTION_STEPPED";
    }

  } else if (program_counter_ >= program_size_) {
    std::cout << "VM_PROGRAM_END" << std::endl;
    output_status_ = "VM_PROGRAM_END";
  }
  DumpRegisters(globals::registers_dump_file_path, registers_);
  DumpState(globals::vm_state_dump_file_path);
}

void RVSSVM::Undo() {
  if (undo_stack_.empty()) {
    std::cout << "VM_NO_MORE_UNDO" << std::endl;
    output_status_ = "VM_NO_MORE_UNDO";
    return;
  }

  StepDelta last = undo_stack_.top();
  undo_stack_.pop();

  // if (!history_.can_undo()) {
  //     std::cout << "Nothing to undo.\n";
  //     return;
  // }

  // StepDelta last = history_.undo();

  for (const auto &change : last.register_changes) {
    switch (change.reg_type) {
      case 0: { // GPR
        registers_.WriteGpr(change.reg_index, change.old_value);
        break;
      }
      case 1: { // CSR
        registers_.WriteCsr(change.reg_index, change.old_value);
        break;
      }
      case 2: { // FPR
        registers_.WriteFpr(change.reg_index, change.old_value);
        break;
      }
      default:std::cerr << "Invalid register type: " << change.reg_type << std::endl;
        break;
    }
  }

  for (const auto &change : last.memory_changes) {
    for (size_t i = 0; i < change.old_bytes_vec.size(); ++i) {
      memory_controller_.WriteByte(change.address + i, change.old_bytes_vec[i]);
    }
  }

  program_counter_ = last.old_pc;
  instructions_retired_--;
  cycle_s_--;
  std::cout << "Program Counter: " << program_counter_ << std::endl;

  redo_stack_.push(last);

  output_status_ = "VM_UNDO_COMPLETED";
  std::cout << "VM_UNDO_COMPLETED" << std::endl;

  DumpRegisters(globals::registers_dump_file_path, registers_);
  DumpState(globals::vm_state_dump_file_path);
}

void RVSSVM::Redo() {
  if (redo_stack_.empty()) {
    std::cout << "VM_NO_MORE_REDO" << std::endl;
    return;
  }

  StepDelta next = redo_stack_.top();
  redo_stack_.pop();

  // if (!history_.can_redo()) {
  //       std::cout << "Nothing to redo.\n";
  //       return;
  //   }

  //   StepDelta next = history_.redo();

  for (const auto &change : next.register_changes) {
    switch (change.reg_type) {
      case 0: { // GPR
        registers_.WriteGpr(change.reg_index, change.new_value);
        break;
      }
      case 1: { // CSR
        registers_.WriteCsr(change.reg_index, change.new_value);
        break;
      }
      case 2: { // FPR
        registers_.WriteFpr(change.reg_index, change.new_value);
        break;
      }
      default:std::cerr << "Invalid register type: " << change.reg_type << std::endl;
        break;
    }
  }

  for (const auto &change : next.memory_changes) {
    for (size_t i = 0; i < change.new_bytes_vec.size(); ++i) {
      memory_controller_.WriteByte(change.address + i, change.new_bytes_vec[i]);
    }
  }

  program_counter_ = next.new_pc;
  instructions_retired_++;
  cycle_s_++;
  DumpRegisters(globals::registers_dump_file_path, registers_);
  DumpState(globals::vm_state_dump_file_path);
  std::cout << "Program Counter: " << program_counter_ << std::endl;
  undo_stack_.push(next);

}

void RVSSVM::Reset() {
  program_counter_ = 0;
  instructions_retired_ = 0;
  cycle_s_ = 0;
  registers_.Reset();
  memory_controller_.Reset();
  control_unit_.Reset();
  branch_flag_ = false;
  next_pc_ = 0;
  execution_result_ = 0;
  memory_result_ = 0;

  return_address_ = 0;
  csr_target_address_ = 0;
  csr_old_value_ = 0;
  csr_write_val_ = 0;
  csr_uimm_ = 0;
  current_delta_.register_changes.clear();
  current_delta_.memory_changes.clear();
  current_delta_.old_pc = 0;
  current_delta_.new_pc = 0;
  undo_stack_ = std::stack<StepDelta>();
  redo_stack_ = std::stack<StepDelta>();

}




