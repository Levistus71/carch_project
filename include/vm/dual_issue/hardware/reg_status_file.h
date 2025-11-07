#pragma once
#include "../core/instruction_context/instruction_context.h"
#include "vm/registers.h"

namespace dual_issue{

class TagFile{
public:
    TagFile();

    void Reset();

    std::pair<bool, uint64_t> GetTagGpr(uint64_t reg) const;
    std::pair<bool, uint64_t> GetTagFpr(uint64_t reg) const;

    std::pair<bool, uint64_t> GetValueGpr(uint64_t reg) const;
    std::pair<bool, uint64_t> GetValueFpr(uint64_t reg) const;

    void WriteGprTag(uint64_t reg, uint64_t tag);
    void WriteFprTag(uint64_t reg, uint64_t tag);

    void WriteGprValue(uint64_t reg, uint64_t val);
    void WriteFprValue(uint64_t reg, uint64_t val);


private:
    static constexpr size_t NUM_GPR = 32;
    static constexpr size_t NUM_FPR = 32;

    std::array<uint64_t, NUM_GPR> gpr_tags{0};
    std::array<uint64_t, NUM_GPR> fpr_tags{0};

    std::array<uint64_t, NUM_GPR> gpr_values{0};
    std::array<uint64_t, NUM_GPR> fpr_values{0};

    std::array<bool, NUM_GPR> gpr_valid{false};
    std::array<bool, NUM_FPR> fpr_valid{false};

    std::array<bool, NUM_GPR> gpr_ready{false};
    std::array<bool, NUM_GPR> fpr_ready{false};
};


class RegisterStatusFile{
public:
    std::pair<bool, uint64_t> QueryTableTag(uint8_t reg_num, bool gpr_register);
    std::pair<bool, uint64_t> QueryTableValue(uint8_t reg_num, bool gpr_register);

    void UpdateTableTag(uint8_t reg_num, bool gpr_register, uint64_t tag);
    void UpdateTableValue(uint8_t reg_num, bool gpr_register, uint64_t val);

private:
    TagFile tag_file_;
};


} // namespace dual_issue