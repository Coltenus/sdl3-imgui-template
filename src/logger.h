#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <imgui.h>

class Logger {
private:
    ImGuiTextBuffer Buf;
    ImVector<int> LineOffsets;
    int scroll;
    std::string name;

public:
    Logger(std::string name);
    ~Logger();
    void log(const std::string& message);
    void clear();
    void draw();
};

#endif
