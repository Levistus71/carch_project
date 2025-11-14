#pragma once
#include "vm/instruction_context.h"
#include "vm/dual_issue/core/instruction_context/instruction_context.h"

namespace triple_issue{

struct TripleIssueInstrContext : public dual_issue::DualIssueInstrContext {
    TripleIssueInstrContext() : DualIssueInstrContext(){}
    TripleIssueInstrContext(uint64_t pc) : DualIssueInstrContext(pc) {}
    ~TripleIssueInstrContext() = default;

    static TripleIssueInstrContext MakeTriple(const dual_issue::DualIssueInstrContext& d) {
        TripleIssueInstrContext t;
        static_cast<dual_issue::DualIssueInstrContext&>(t) = d;
        t.into_falu = false;
        return t;
    }

    bool into_falu = false;
};


} // namespace dual_issue