#pragma once
#include "./instruction_context/instruction_context.h"
#include "../hardware/reservation_station.h"
#include "vm/registers.h"
#include "vm/memory_controller.h"


namespace dual_issue{

class RegisterStatusFile;
class CommitBuffer;
class CommonDataBus;

class DualIssueCore{
public:
    std::deque<DualIssueInstrContext> instruction_deque_;
    uint64_t pc = 0;

    uint64_t to_commit_tag_ = 0; // if a branch is predicted incorrectly, we need to nuke all instructions "younger" than the new instruction (after branch) 

    bool is_stop_requested_ = false;

    // Debug vars
    bool debug_mode_{true};
    std::deque<DualIssueInstrContext> undo_instruction_stack_;
    size_t max_undo_stack_size_{256};
    bool stop_requested_ = false;
    std::vector<uint64_t> breakpoints_;

    // Hardware
    alu::Alu alu_;
    register_file::RegisterFile register_file_;
    memory_controller::MemoryController memory_controller_;
    ReservationStation alu_que_;
    ReservationStation lsu_que_;
    CommonDataBus broadcast_bus_;
    CommitBuffer commit_buffer_;
    RegisterStatusFile reg_status_file_;

    // for input handling in syscalls:
	std::mutex input_mutex_;
	std::condition_variable input_cv_;
	std::queue<std::string> input_queue_;

    uint64_t program_size_ = 0;

};


} // namespace dual_issue