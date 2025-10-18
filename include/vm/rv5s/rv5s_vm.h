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
#include <cctype>
#include <tuple> 
#include <queue>
#include <thread>
#include <chrono>

#include "utils.h"
#include "globals.h"
#include "common/instructions.h"
#include "config.h"


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

  bool stop_requested = false;


  std::deque<StepDelta> undo_stack;
  size_t max_undo_stack_size;

  StepDelta current_delta;

  // WHAT?: what are these
  // CSR intermediate variables
  uint16_t csr_target_address_{};
  uint64_t csr_old_value_{};
  uint64_t csr_write_val_{};
  uint8_t csr_uimm_{};

  void LoadVM() override;

  void Fetch();
  void DebugFetch();

  void Decode();
  void DebugDecode();

  void Execute();
  void DebugExecute();
  void ResolveBranch();
  void ExecuteBasic();
  void ExecuteFloat();
  void ExecuteDouble();
  void ExecuteCsr();
  void HandleSyscall();

  void MemoryAccess();
  void DebugMemoryAccess();

  void WriteBack();
  void DebugWriteBack();
  void WriteBackCsr();
  void DebugWriteBackCsr();

  RV5SVM();
  ~RV5SVM() = default;

  void Run() override;
  void RunSingleCycle();

  void DebugRun() override;
  void DebugRunSingleCycle();

  void Step() override;
  void SingleCycleStep(bool dump);

  void Undo() override;
  void SingleCycleUndo();

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
    std::cout << "rvssvm" << std::endl;
  }
};

#endif // RVSS_VM_H
