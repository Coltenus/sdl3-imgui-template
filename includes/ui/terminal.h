#ifndef TERMINAL_H
#define TERMINAL_H

#include "ui/logger.h"
#include "utils/shell.h"

namespace ui {

class Terminal : public Logger {
private:
    std::string path;
    char input[256];
    utils::PersistentShell shell;
    int focus;
    std::string buffer;
    void add(const char* fmt, ...) override;

public:
    Terminal(std::string name);
    ~Terminal();
    void execute(const char* fmt, ...);
    void clear() override;
    void reset();
    void draw() override;
};

};

#endif
