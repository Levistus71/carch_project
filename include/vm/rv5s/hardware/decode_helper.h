#pragma once

#include "vm/rv5s/core/instruction_context/instruction_context.h"
#include "vm/alu.h"

namespace DecoderHelper
{

    void SetContextValues(rv5s::InstrContext& instr_context);
} // namespace DecoderHelper