/**
 * @file vm_base.cpp
 * @brief File containing the base class for the virtual machine
 * @author Vishank Singh, https://VishankSingh
 */

#include "vm/vm_base.h"

#include "globals.h"
#include "config.h"

#include <cstdint>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cstring>
#include <thread>


uint64_t VmBase::GetProgramCounter() const {
    return program_counter_;
}

void VmBase::AddToProgramCounter(int64_t value) {
    program_counter_ = static_cast<uint64_t>(static_cast<int64_t>(program_counter_) + value);
}

void VmBase::SetProgramCounter(uint64_t value){
    this->program_counter_ = value;
}



void VmBase::AddBreakpoint(uint64_t val, bool is_line) {
    if (is_line) {
        // If the value is a line number, convert it to an instruction address
        if (program_.line_number_instruction_number_mapping.find(val) == program_.line_number_instruction_number_mapping.end()) {
            std::cerr << "Invalid line number: " << val << std::endl;
            return;
        }
        uint64_t line = val;
        uint64_t bp = program_.line_number_instruction_number_mapping[line] * 4;
        if (CheckBreakpoint(bp)) {
            std::cerr << "Breakpoint already exists at line: " << line << std::endl;
            return;
        }
        breakpoints_.emplace_back(bp);
    } else {
        if (val % 4 != 0) {
            std::cerr << "Invalid instruction address: " << val << ". Must be a multiple of 4." << std::endl;
            return;
        }
        if (CheckBreakpoint(val)) {
            std::cerr << "Breakpoint already exists at address: " << val << std::endl;
            return;
        }
        breakpoints_.emplace_back(val);
    }

    DumpState(globals::vm_state_dump_file_path);
}

void VmBase::RemoveBreakpoint(uint64_t val, bool is_line) {
    if (is_line) {
        // If the value is a line number, convert it to an instruction address
        if (program_.line_number_instruction_number_mapping.find(val) == program_.line_number_instruction_number_mapping.end()) {
            std::cerr << "Invalid line number: " << val << std::endl;
            return;
        }
        uint64_t line = val;
        uint64_t bp = program_.line_number_instruction_number_mapping[line] * 4;
        if (!CheckBreakpoint(bp)) {
            std::cerr << "No breakpoint exists at line: " << line << std::endl;
            return;
        }
        breakpoints_.erase(std::remove(breakpoints_.begin(), breakpoints_.end(), bp), breakpoints_.end());
    } else {
        if (val % 4 != 0) {
            std::cerr << "Invalid instruction address: " << val << ". Must be a multiple of 4." << std::endl;
            return;
        }
        if (!CheckBreakpoint(val)) {
            std::cerr << "No breakpoint exists at address: " << val << std::endl;
            return;
        }
        breakpoints_.erase(std::remove(breakpoints_.begin(), breakpoints_.end(), val), breakpoints_.end());
    }
    DumpState(globals::vm_state_dump_file_path);


}

bool VmBase::CheckBreakpoint(uint64_t address) {
    return std::find(breakpoints_.begin(), breakpoints_.end(), address) != breakpoints_.end();
}


void VmBase::PrintString(uint64_t address) {
    while (true) {
        char c = memory_controller_.ReadByte(address);
        if (c == '\0') break;
        globals::vm_cout_file << c;
        address++;
    }
}

void VmBase::DumpState(const std::filesystem::path &filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file for dumping VM state: " << filename.string() << std::endl;
        return;
    }

    unsigned int instruction_number = program_counter_ / 4;
    unsigned int current_line = program_.instruction_number_line_number_mapping[instruction_number];

    file << "{\n";
    file << "    \"program_counter\": " << "\"0x" 
         << std::hex << std::setw(8) << std::setfill('0') 
         << program_counter_ 
         << std::dec << std::setfill(' ') 
         << "\",\n";
    file << "    \"current_line\": " << current_line << ",\n";
    file << "    \"current_instruction\": " << "\"0x" 
         << std::hex << std::setw(8) << std::setfill('0') 
         << "TODO: there are multiple \"current instructions\"" 
         << std::dec << std::setfill(' ') 
         << "\",\n";
    file << "    \"disassembly_line_number\": " << program_.instruction_number_disassembly_mapping[instruction_number] << ",\n";
    file << "    \"breakpoints\": [";
    for (size_t i = 1; i < breakpoints_.size(); ++i) {
        program_.instruction_number_line_number_mapping[breakpoints_[i] / 4];
        file << program_.instruction_number_line_number_mapping[breakpoints_[i] / 4];
        if (i < breakpoints_.size() - 1) {
            file << ", ";
        }
    }
    file << "],\n";
    file << "    \"output_status\": \"" << output_status_ << "\"\n";
    file << "}\n";
    file.close();

}

void VmBase::ModifyRegister(const std::string &reg_name, uint64_t value) {
    registers_.ModifyRegister(reg_name, value);
}