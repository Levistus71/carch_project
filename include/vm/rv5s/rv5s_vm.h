/**
 * @file rvss_vm.h
 * @brief RVSS VM definition
 * @author Vishank Singh, https://github.com/VishankSingh
 */
#ifndef RVSS_VM_H
#define RVSS_VM_H


#include "vm/vm_base.h"

#include "vm/rv5s/decoder/rv5s_decode_unit.h"
#include "vm/rv5s/instruction_context/instr_context.h"
#include "vm/rv5s/pipelined/branch_predictor/branch_predictor.h"

#include <stack>
#include <vector>
#include <iostream>
#include <cstdint>
#include <cctype>
#include <tuple> 
#include <queue>
#include <thread>
#include <chrono>

#include "utils.h"
#include "globals.h"
#include "common/instructions.h"
#include "config.h"



class RV5SVM : public VmBase {

private:
	// vm vars 
	bool pipelining_enabled = false;
	bool data_forwarding_enabled = false;
	bool hazard_detection_enabled = false;

	bool branch_prediction_enabled = false;
	bool branch_prediction_static = false;
	
	std::deque<InstrContext> instruction_deque;
	RV5SDecodeUnit decode_unit;
  	RV5SBranchPredictor branch_predictor;

	
	// DEBUG VARS
	bool stop_requested = false;
  	std::deque<InstrContext> undo_instruction_stack;
	size_t max_undo_stack_size;
	
	// for input handling in syscalls:
	std::mutex input_mutex;
	std::condition_variable input_cv;
	std::queue<std::string> input_queue;
	
	// STAGES:
	void Fetch();

	void Decode(bool debug_mode);
	void HandleSyscall(bool debug_mode);

	void Execute();
	void ResolveBranch();
	void ExecuteBasic();
	void ExecuteFloat();
	void ExecuteDouble();
	
	void MemoryAccess(bool debug_mode);
	
	void WriteBack(bool debug_mode);
	void WriteBackCsr(bool debug_mode);	
	
	// hazards:
	bool DetectDataHazardWithoutForwarding();
	bool DetectDataHazardWithForwarding();
	bool DetectControlHazard();
  	void InsertBubble();
  	void HandleControlHazard();
	
	// RUN:
  	// SingleCycle
	void RunSingleCycle();

	// Pipeline
  	void DrivePipeline(bool debug_mode);
  	void PopWbInstruction(bool debug_mode);

	void RunPipelined();
  	void StepPipelined(bool debug_mode);

	void RunPipelinedWithoutHazardDetection(bool debug_mode);
  	void StepPipelinedWithoutHazardDetection(bool debug_mode);
  
	void RunPipelinedWithHazardWithoutForwarding(bool debug_mode);
  	void StepPipelinedWithHazardWithoutForwarding(bool debug_mode);
  
	void RunPipelinedWithHazardWithForwarding(bool debug_mode);
  	void StepPipelinedWithHazardWithForwarding(bool debug_mode);
	
	// DEBUG RUN:
  	// SingleCycle
	void DebugRunSingleCycle();
	void SingleCycleStep(bool dump);
	void SingleCycleUndo();
	// Pipeline
	void DebugRunPipelined();
	void PipelinedUndo();

public:

	bool GetBranchPredictionStatus(){
		return branch_prediction_enabled;
	}
	bool BranchPredictionIsStatic(){
		return branch_prediction_static;
	}

    InstrContext& GetIfInstruction(){
      return instruction_deque[0];
    }
    InstrContext& GetIdInstruction(){
      return instruction_deque[1];
    }
    InstrContext& GetExInstruction(){
      return instruction_deque[2];
    }
    InstrContext& GetMemInstruction(){
      return instruction_deque[3];
    }
    InstrContext& GetWbInstruction(){
      return instruction_deque[4];
    }

    bool PipeliningEnabled(){
      return pipelining_enabled;
    }

    std::vector<uint64_t> GetInstructionPCs(){
      if(pipelining_enabled)
        return {instruction_deque[0].pc, instruction_deque[1].pc, instruction_deque[2].pc, instruction_deque[3].pc, instruction_deque[4].pc};
      else
        return {this->program_counter_};
    }


    void LoadVM() override;

	RV5SVM() : VmBase(), instruction_deque(5) {
		DumpRegisters(globals::registers_dump_file_path, registers_);
		DumpState(globals::vm_state_dump_file_path);

		pipelining_enabled = vm_config::config.getPipeliningStatus();
		hazard_detection_enabled = vm_config::config.getHazardDetectionStatus();
		data_forwarding_enabled = vm_config::config.getDataFowardingStatus();

		GetIfInstruction().nopify();
		GetIdInstruction().nopify();
		GetExInstruction().nopify();
		GetMemInstruction().nopify();
		GetWbInstruction().nopify();
	}
    ~RV5SVM() = default;

    void Run() override;

    void DebugRun() override;

    void Step() override;


    void Undo() override;

    void Reset() override;

    void RequestStop() {
      this->stop_requested = true;
    }

    bool IsStopRequested() const {
      return this->stop_requested;
    }
    
    void ClearStop() {
      this->stop_requested = false;
    }

    void PrintType() {
      globals::vm_cout_file << "rvssvm" << std::endl;
    }
};

#endif // RVSS_VM_H
