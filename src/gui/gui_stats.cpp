#include "gui/gui_stats.h"
#include "vm/vm_base.h"
#include "gui/gui_common.h"

void stats_main() {
    VmBase::Stats& stats = vm.GetStats();

    ImVec2 window_size = ImGui::GetWindowSize();
    ImVec2 size = { window_size.x, window_size.y };

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1211f, 0.1211f, 0.1211f, 1.00f)); // background color
    ImGui::PushFont(STANDARD_BOLD_MEDIUM_FONT);
    ImGui::BeginChild("Stats Window", size, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    std::vector<std::string> lines;

    if(vm.GetType()==VM::Which::SingleCycle){
        char buf[128];
        snprintf(buf, sizeof(buf), "Number of cycles: %zu", stats.cycles);
        lines.emplace_back(buf);

        snprintf(buf, sizeof(buf), "Number of instructions retired: %zu", stats.instrs_retired);
        lines.emplace_back(buf);

        snprintf(buf, sizeof(buf), "Cycles Per Instruction (CPI): %.3f", static_cast<float>(stats.cycles) / stats.instrs_retired);
        lines.emplace_back(buf);
    }
    else{
        char buf[128];
        snprintf(buf, sizeof(buf), "Number of cycles: %zu", stats.cycles);
        lines.emplace_back(buf);

        snprintf(buf, sizeof(buf), "Number of instructions retired: %zu", stats.instrs_retired);
        lines.emplace_back(buf);

        snprintf(buf, sizeof(buf), "Number of branches: %zu", stats.branch_instrs);
        lines.emplace_back(buf);

        snprintf(buf, sizeof(buf), "Branches predicted correctly: %zu", stats.branch_instrs - stats.branch_mispredicts);
        lines.emplace_back(buf);

        snprintf(buf, sizeof(buf), "Branches predicted incorrectly: %zu", stats.branch_mispredicts);
        lines.emplace_back(buf);

        snprintf(buf, sizeof(buf), "Branch Prediction Accuracy: %.2f%%",(1.0f - static_cast<float>(stats.branch_mispredicts) / stats.branch_instrs) * 100.0f);
        lines.emplace_back(buf);

        snprintf(buf, sizeof(buf), "Cycles Per Instruction (CPI): %.3f",static_cast<float>(stats.cycles) / stats.instrs_retired);
        lines.emplace_back(buf);

        snprintf(buf, sizeof(buf), "Instructions Per Cycle (IPC): %.3f",static_cast<float>(stats.instrs_retired) / stats.cycles);
        lines.emplace_back(buf);
    }

    float line_height = ImGui::GetTextLineHeight();
    float line_spacing = 10.0f; // buffer between lines
    float total_text_height = static_cast<float>(lines.size()) * (line_height + line_spacing);
    float window_height = ImGui::GetWindowSize().y;

    float start_y = (window_height - total_text_height) * 0.5f;
    if (start_y < 0.0f) start_y = 0.0f;

    ImGui::SetCursorPosY(start_y);

    auto CenterText = [](const char* text) {
        float window_width = ImGui::GetWindowSize().x;
        float text_width = ImGui::CalcTextSize(text).x;
        ImGui::SetCursorPosX((window_width - text_width) * 0.5f);
        ImGui::TextUnformatted(text);
    };

    for (size_t i = 0; i < lines.size(); ++i) {
        CenterText(lines[i].c_str());
        if (i < lines.size() - 1)
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + line_spacing);
    }
    ImGui::PopFont();
    ImGui::PopStyleColor();
    ImGui::EndChild();
}
