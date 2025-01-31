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

public:
    Logger(std::string name);
    virtual ~Logger();
    virtual void add(const std::string& message);
    virtual void clear();
    virtual void draw();
};

};

#endif
