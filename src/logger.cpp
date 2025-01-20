#include "logger.h"

Logger::Logger(std::string name) {
    scroll = 0;
    this->name = name;
}

Logger::~Logger() {
}

void Logger::add(const std::string& message) {
    for(int i = 0; i < message.size(); i++) {
        if(message[i] == '\n') {
            LineOffsets.push_back(Buf.size() - message.size() + i);
        }
    }
    Buf.appendf("%s\n", message.c_str());
    LineOffsets.push_back(Buf.size());
    scroll = 2;
}

void Logger::clear() {
    Buf.clear();
    LineOffsets.clear();
    scroll = 0;
}

void Logger::draw() {
    ImGui::SetNextWindowSize(ImVec2(500,400), ImGuiCond_FirstUseEver);
    ImGui::Begin(name.c_str());
    bool copy = ImGui::Button("Copy");
    ImGui::SameLine();
    bool clear_ = ImGui::Button("Clear");
    ImGui::Separator();
    ImGui::BeginChild("scrolling");
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,1));
    if (copy) ImGui::LogToClipboard();
    if (clear_) clear();
    ImGui::TextUnformatted(Buf.begin());
    if(scroll) {
        ImGui::SetScrollHereY(1.0f);
        scroll--;
    }
    ImGui::PopStyleVar();
    ImGui::EndChild();
    ImGui::End();
}