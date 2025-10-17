/**
 * @file control_unit_base.h
 * @brief Control unit base class definition
 * @author Vishank Singh, https://github.com/VishankSingh
 */
#ifndef CONTROL_UNIT_BASE_H
#define CONTROL_UNIT_BASE_H

#include <cstdint>

#include "common/instructions.h"
#include "registers.h"
#include "alu.h"
#include "vm/instruction_context/instr_context.h"
#include "vm/registers.h"

/**
 * @brief The DecodeUnit class is the decode unit of the CPU. Responsible for setting the control signals and getting the alu signal
 */
class RV5SDecodeUnit {
public:
    ~RV5SDecodeUnit() = default;

    /**
     * @brief 
     */
    void DecodeInstruction(InstrContext& instr_context, RegisterFile& rf);
    
private:
    /**
     * @brief DecodeInstrFields decodes the opcode, registers (rs1,rs2,rs3,rd), funct values and the immediate
     * @param instr_context The instruction being decoded.
     */
    void DecodeInstrFields(InstrContext& instr_context);

    /**
     * @brief SetRegImmValues sets the register values and generates the immediate. Simulates fetching from the register file
     */
    void RV5SDecodeUnit::SetRegImmValues(InstrContext& instr_context, RegisterFile& rf);

    /**
     * @brief SetControlSignals sets the alu flags, mux select line flags.
     * @param instr_context
     */
    void SetContextValues(InstrContext& instr_context);


    /**
     * @brief SetMemAccessSize sets the num_bytes field of InstrContext. Just tells how many bytes are being accesed in the memory phase
     */
    void SetMemValues(InstrContext& instr_context);

    /**
     * @brief GetAluSignal helper function used by SetContextValues which sets the alu flags
     * @param instr_context
     */
    alu::AluOp GetAluSignal(InstrContext& instr_context);

    /**
     * @brief ImmGenerator is a helper function used by SetRegImmValues, generates the immediate
     * @param instr_context
     */
    int32_t ImmGenerator(InstrContext& instr_context);
};

#endif // CONTROL_UNIT_BASE_H
