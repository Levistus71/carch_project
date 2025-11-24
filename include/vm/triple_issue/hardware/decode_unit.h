#pragma once

#include <cstdint>
#include "vm/dual_issue/hardware/decode_unit.h"
#include "common/instructions.h"
#include "vm/alu.h"
#include "vm/registers.h"


namespace triple_issue
{
struct TripleIssueInstrContext;

/**
 * @brief The DecodeUnit class is the decode unit of the CPU. Responsible for setting the control signals and getting the alu signal
 */
class TripleIssueDecodeUnit : dual_issue::DualIssueDecodeUnit {
public:
    ~TripleIssueDecodeUnit() = default;

    /**
     * @brief 
     */
    void DecodeInstruction(TripleIssueInstrContext& instr_context);

    /**
     * 
     */
    void SetRegImmValues(TripleIssueInstrContext& instr_context, register_file::RegisterFile& rf);
    
private:    

    void SetAluQue(TripleIssueInstrContext& instr_context);
};

} // namespace rv5s
