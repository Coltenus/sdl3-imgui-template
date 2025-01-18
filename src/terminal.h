#ifndef TERMINAL_H
#define TERMINAL_H

#include "imgui.h"
#include "shell.h"

class Terminal {
private:
    ImGuiTextBuffer Buf;
    ImVector<int> LineOffsets;
    int scroll;
    std::string name, path;
    char input[256];
    PersistentShell shell;
    int focus;

public:
    Terminal(std::string name);
    ~Terminal();
    void execute(std::string command);
    void clear();
    void reset();
    void draw();
};

#endif
