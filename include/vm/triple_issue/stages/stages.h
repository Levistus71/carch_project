#pragma once
#include "../core/core.h"

namespace triple_issue{


/**
 * An instruction should follow this path:
 * if_id_2 -> if_id_1 -> id_issue_2 -> id_issue_1 -> Rest
 * 
 * id_issue_1 is the oldest instruction. if_id_2 is the youngest.
 * 
 */

class TripleIssueStages{
public:
    /**
     * If num_fetch is 3, it stores all 3 of them in if-id
     * If num_fetch is 2, it stores them in if_id_2 and if_id_3
     * If num_fetch is 1, it stores it in if_id_3 (youngest instruction)
     * If num_fetch is 0, it doesn't fetch (no update to if_id_1/if_id_2/if_id_3)
     */
    static void Fetch(TripleIssueCore& vm_core, int num_fetch);


    /**
     * Decodes instructions in place from vm_core.pipeline_reg_instrs_.if_id_1 and if_id_2, doens't move anything
     */
    static void Decode(TripleIssueCore& vm_core);

    /**
     * If all 3 can be pushed into the reserve stations, it pushes all of them
     * If 2 can be pushed, it stores the unpushed instruction in id_issue_1 and id_issue_2
     * If only 1 can be pushed, it stores the unpushed instruction in id_issue_1 (oldest unpushed instruction)
     * If nothing can be pushed, it does nothing
     * 
     * Returns the number of pushed instructions
     */
    static int Issue(TripleIssueCore& vm_core);

    /**
     * The branch logic is done here. if the branch is predicted incorrectly, the vm_core.to_commit_tag_
     * gets updated here. The eviction is done in commit_buffer
     */
    static void ExecuteAlu(TripleIssueCore& vm_core);
    static void ExecuteFalu(TripleIssueCore& vm_core);

    static void MemoryAccess(TripleIssueCore& vm_core);

    static void WriteBack(dual_issue::DualIssueInstrContext& wb_instruction, TripleIssueCore& vm_core);


private:
    // FIXME: this doesn't belong here
    static void HandleSyscall(TripleIssueCore& vm_core);

    static void ExecuteBasic(TripleIssueCore& vm_core, dual_issue::DualIssueInstrContext& instr);
    static void ExecuteFloat(TripleIssueCore& vm_core, dual_issue::DualIssueInstrContext& instr);
    static void ExecuteDouble(TripleIssueCore& vm_core, dual_issue::DualIssueInstrContext& instr);
};


} // namespace rv5s