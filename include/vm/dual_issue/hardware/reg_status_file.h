#pragma once
#include "../core/instruction_context/instruction_context.h"
#include "vm/registers.h"

namespace dual_issue{

class TagFile{
public:
    TagFile();

    void Reset();

    std::pair<bool, uint64_t> GetRobIdxGpr(uint64_t reg) const;
    std::pair<bool, uint64_t> GetRobIdxFpr(uint64_t reg) const;

    void WriteGprRobIdx(uint64_t reg, uint64_t rob_idx);
    void WriteFprRobIdx(uint64_t reg, uint64_t rob_idx);

    void EndDependencyGpr(uint64_t rd_reg, uint64_t rob_idx);
    void EndDependencyFpr(uint64_t rd_reg, uint64_t rob_idx);


private:
    static constexpr size_t NUM_GPR = 32;
    static constexpr size_t NUM_FPR = 32;

    std::array<uint64_t, NUM_GPR> gpr_idxs{0};
    std::array<uint64_t, NUM_GPR> fpr_idxs{0};

    std::array<bool, NUM_GPR> gpr_valid{false};
    std::array<bool, NUM_FPR> fpr_valid{false};
};


class RegisterStatusFile{
public:
    std::pair<bool, uint64_t> QueryTableRobIdx(uint8_t reg_num, bool gpr_register);

    void UpdateTableRobIdx(uint8_t reg_num, bool gpr_register, uint64_t rob_idx);

    void EndDependency(size_t rd_reg_num, size_t rob_idx, bool gpr_register);

    void Reset();

private:
    TagFile tag_file_;
};


} // namespace dual_issue