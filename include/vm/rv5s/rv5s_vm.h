/**
 * @file rvss_vm.h
 * @brief RVSS VM definition
 * @author Vishank Singh, https://github.com/VishankSingh
 */
#ifndef RVSS_VM_H
#define RVSS_VM_H


#include "vm/vm_base.h"

#include "vm/decoder/rv5s_decode_unit.h"
#include "vm/instruction_context/instr_context.h"

#include <stack>
#include <vector>
#include <iostream>
#include <cstdint>

// TODO: use a circular buffer instead of a stack for undo/redo

struct RegisterChange {
  unsigned int reg_index;
  unsigned int reg_type; // 0 for GPR, 1 for CSR, 2 for FPR
  uint64_t old_value;
  uint64_t new_value;
};

struct MemoryChange {
  uint64_t address;
  std::vector<uint8_t> old_bytes_vec; 
  std::vector<uint8_t> new_bytes_vec; 
};

struct StepDelta {
  uint64_t old_pc;
  uint64_t new_pc;
  std::vector<RegisterChange> register_changes;
  std::vector<MemoryChange> memory_changes;
};

class RV5SVM : public VmBase {
public:
  // consts 
  bool pipelining_enabled = false;
  bool data_forwarding_enabled = false;
  bool hazard_detection_enabled = false;

  InstrContext* if_instruction{nullptr};
  InstrContext* id_instruction{nullptr};
  InstrContext* ex_instruction{nullptr};
  InstrContext* mem_instruction{nullptr};
  InstrContext* wb_instruction{nullptr};

  RV5SDecodeUnit decode_unit;

  std::atomic<bool> stop_requested_ = false;


  std::stack<StepDelta> undo_stack_;
  std::stack<StepDelta> redo_stack_;

  StepDelta current_delta_;

  // intermediate variables
  int64_t execution_result_{};
  int64_t memory_result_{};
  // int64_t memory_address_{};
  // int64_t memory_data_{};
  uint64_t return_address_{};

  bool branch_flag_ = false;
  int64_t next_pc_{}; // for jal, jalr,

  // CSR intermediate variables
  uint16_t csr_target_address_{};
  uint64_t csr_old_value_{};
  uint64_t csr_write_val_{};
  uint8_t csr_uimm_{};

  void Fetch();

  void Decode();

  void Execute();
  void ResolveBranch();
  void ExecuteBasic();
  void ExecuteFloat();
  void ExecuteDouble();
  void ExecuteCsr();
  void HandleSyscall();

  void MemoryAccess();

  void WriteBack();
  void WriteBackCsr();

  RV5SVM();
  ~RV5SVM();

  void Run() override;
  void DebugRun() override;
  void Step() override;
  void Undo() override;
  void Redo() override;
  void Reset() override;

  void RequestStop() {
    stop_requested_ = true;
  }

  bool IsStopRequested() const {
    return stop_requested_;
  }
  
  void ClearStop() {
    stop_requested_ = false;
  }

  void PrintType() {
    std::cout << "rvssvm" << std::endl;
  }
};

#endif // RVSS_VM_H
