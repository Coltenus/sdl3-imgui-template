#include "terminal.h"

Terminal::Terminal(std::string name) {
    scroll = 0;
    this->name = name;
    memset(input, 0, sizeof(input));
    this->focus = 1;
}

Terminal::~Terminal() {
}

void Terminal::execute(std::string command) {
    std::string result = shell.execute(command);
    for(int i = 0; i < result.size(); i++) {
        if(result[i] == '\n') {
            LineOffsets.push_back(Buf.size() - result.size() + i);
        }
    }
    Buf.appendf("%s> %s\n", shell.get_path().c_str(), command.c_str());
    LineOffsets.push_back(Buf.size());
    Buf.append(result.c_str());
    Buf.append("\n");
    LineOffsets.push_back(Buf.size());
    scroll = 2;
    focus = 1;
}

void Terminal::clear() {
    Buf.clear();
    LineOffsets.clear();
    scroll = 0;
    focus = 1;
}

void Terminal::reset() {
    shell.reset();
    clear();
}

void Terminal::draw() {
    ImGui::SetNextWindowSize(ImVec2(500,400), ImGuiCond_FirstUseEver);
    ImGui::Begin(name.c_str());
    bool copy = ImGui::Button("Copy");
    ImGui::SameLine();
    bool clear_ = ImGui::Button("Clear");
    ImGui::SameLine();
    bool reset_ = ImGui::Button("Reset");
    ImGui::Separator();
    ImGui::BeginChild("scrolling");
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,1));
    if (copy) ImGui::LogToClipboard();
    if (clear_) clear();
    if (reset_) reset();
    ImGui::TextUnformatted(Buf.begin());
    ImGui::Text("%s>", shell.get_path().c_str());
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1);
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::PushStyleColor(ImGuiCol_FrameBg, style.Colors[ImGuiCol_ChildBg]);
    ImGui::PushStyleColor(ImGuiCol_NavHighlight, style.Colors[ImGuiCol_ChildBg]);
    if(focus) {
        ImGui::SetKeyboardFocusHere();
        focus--;
    }
    if(ImGui::InputText("Input", input, IM_ARRAYSIZE(input), ImGuiInputTextFlags_EnterReturnsTrue)) {
        execute(input);
        memset(input, 0, sizeof(input));
    }
    if(scroll) {
        ImGui::SetScrollHereY(1.0f);
        scroll--;
    }
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    ImGui::EndChild();
    ImGui::End();
}
