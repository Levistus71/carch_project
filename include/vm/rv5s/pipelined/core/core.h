#pragma once

#include "./instruction_context/instruction_context.h"
#include "../hardware/decode_unit.h"
#include "../hardware/branch_predictor.h"
#include "../hardware/hazard_detector.h"
#include "../../../alu.h"
#include "../../../registers.h"
#include "../../../memory_controller.h"
#include "vm_asm_mw.h"
#include "vm/vm_base.h"

namespace rv5s{
    
class PipelinedCore{
public:
    std::deque<PipelinedInstrContext> instruction_deque_;
    uint64_t program_counter_{};

    bool hazard_detection_enabled_ = false;
    bool data_forwarding_enabled_ = false;

	bool branch_prediction_enabled_ = false;
	bool branch_prediction_static_ = false;
    
    bool is_stop_requested_ = false;

    // Debug vars
    bool debug_mode_{true};
    std::deque<PipelinedInstrContext> undo_instruction_stack_;
    size_t max_undo_stack_size_{256};
    bool stop_requested_ = false;
    std::vector<uint64_t> breakpoints_;

    // hardware
	PipelinedDecodeUnit decode_unit_;
  	BranchPredictor branch_predictor_;
    HazardDetector hazard_detector_;
    alu::Alu alu_;
    register_file::RegisterFile register_file_;
    memory_controller::MemoryController memory_controller_;
    

    // for input handling in syscalls:
	std::mutex input_mutex_;
	std::condition_variable input_cv_;
	std::queue<std::string> input_queue_;

    uint64_t program_size_ = 0;

    VmBase::Stats core_stats_;

    PipelinedCore();

    uint64_t GetProgramCounter() const;
    void AddToProgramCounter(int64_t value);
    void SetProgramCounter(uint64_t value);

    PipelinedInstrContext& GetIfInstruction();
    PipelinedInstrContext& GetIdInstruction();
    PipelinedInstrContext& GetExInstruction();
    PipelinedInstrContext& GetMemInstruction();
    PipelinedInstrContext& GetWbInstruction();

    void ClearStop();

    void Reset();

    void Load(AssembledProgram& program);
    void Load();

    VmBase::Stats& GetStats();
};

} // namespace rv5s
