#pragma once

#include "../core/instruction_context/instruction_context.h"
#include "vm/alu.h"

namespace DecoderHelper
{
    void SetContextValues(dual_issue::DualIssueInstrContext& instr_context);
} // namespace DecoderHelper