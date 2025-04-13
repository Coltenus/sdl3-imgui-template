#include "ui/logger.h"

namespace ui {

Logger::Logger(std::string name) {
    scroll = 0;
    this->name = name;
}

Logger::~Logger() {
}

void Logger::add_string(const std::string& message) {
    for(int i = 0; i < message.size(); i++) {
        if(message[i] == '\n') {
            LineOffsets.push_back(Buf.size() - message.size() + i);
        }
    }
    Buf.appendf("%s\n", message.c_str());
    LineOffsets.push_back(Buf.size());
    scroll = 2;
}

void Logger::add(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    std::string message = fmt_string(fmt, args);
    va_end(args);
    add_string(message);
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

std::string Logger::fmt_string(const char* fmt, va_list args) {
    va_list args_copy;
    va_copy(args_copy, args);
    int size = vsnprintf(nullptr, 0, fmt, args) + 1;
    va_end(args_copy);
    char* buffer = new char[size];
    vsnprintf(buffer, size, fmt, args);
    std::string message = buffer;
    delete[] buffer;
    return message;
}

};
