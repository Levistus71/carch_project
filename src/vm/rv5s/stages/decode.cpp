#include "vm/rv5s/rv5s_vm.h"

using instruction_set::Instruction;
using instruction_set::get_instr_encoding;

void RV5SVM::Decode(bool debug_mode) {
	InstrContext& id_instruction = GetIdInstruction();
	decode_unit.DecodeInstruction(id_instruction, registers_);

	if (id_instruction.opcode == get_instr_encoding(Instruction::kecall).opcode && 
		id_instruction.funct3 == get_instr_encoding(Instruction::kecall).funct3) {
		HandleSyscall(debug_mode);
		return;
	}
}


void RV5SVM::HandleSyscall(bool debug_mode){
	InstrContext& id_instruction = GetIdInstruction();
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
			this->stop_requested = true; // Stop the VM
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
					std::unique_lock<std::mutex> lock(this->input_mutex);
					this->input_cv.wait(lock, [this]() { 
						return !this->input_queue.empty(); 
					});
					output_status_ = "VM_STDIN_END";
					std::cout << "VM_STDIN_END" << std::endl;

					input = this->input_queue.front();
					this->input_queue.pop();
				}

				
				if(debug_mode){
					for (size_t i = 0; i < length; ++i) {
						id_instruction.mem_overwritten.push_back(memory_controller_.ReadByte(buffer_address + i));
					}
				}
				
				for (size_t i = 0; i < input.size() && i < length; ++i) {
					memory_controller_.WriteByte(buffer_address + i, static_cast<uint8_t>(input[i]));
				}
				if (input.size() < length) {
					memory_controller_.WriteByte(buffer_address + input.size(), '\0');
				}


				uint64_t new_reg = std::min(static_cast<uint64_t>(length), static_cast<uint64_t>(input.size()));
				this->registers_.WriteGpr(10, new_reg);
				if(debug_mode){
					id_instruction.reg_overwritten = this->registers_.ReadGpr(10);
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

				uint64_t new_reg = std::min(static_cast<uint64_t>(length), bytes_printed);
				registers_.WriteGpr(10, new_reg);
				if (debug_mode) {
					id_instruction.reg_overwritten = this->registers_.ReadGpr(10);
				}
			}
			else {
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