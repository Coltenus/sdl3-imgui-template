#ifndef TERMINAL_H
#define TERMINAL_H

#include "logger.h"
#include "shell.h"

class Terminal : public Logger {
private:
    std::string path;
    char input[256];
    PersistentShell shell;
    int focus;
    std::string buffer;
    void add(const std::string& message) override;

public:
    Terminal(std::string name);
    ~Terminal();
    void execute(std::string command);
    void clear() override;
    void reset();
    void draw() override;
};

#endif
