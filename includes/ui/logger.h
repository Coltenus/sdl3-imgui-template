#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <imgui.h>

namespace ui {

class Logger {
protected:
    ImGuiTextBuffer Buf;
    ImVector<int> LineOffsets;
    int scroll;
    std::string name;
    virtual void add_string(const std::string& message);
    std::string fmt_string(const char* fmt, va_list args);

public:
    Logger(std::string name);
    virtual ~Logger();
    virtual void add(const char* fmt, ...);
    virtual void clear();
    virtual void draw();
};

};

#endif
