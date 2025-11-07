#pragma once

#include "../core/instruction_context/instruction_context.h"
#include "vm/alu.h"

namespace DecoderHelper
{
    void SetContextValues(rv5s::PipelinedInstrContext& instr_context);
} // namespace DecoderHelper