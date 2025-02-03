#ifndef SHELL_H
#define SHELL_H

#include <string>
#include <iostream>
#include <stdexcept>
#ifdef _WIN32
#include <windows.h>
#else
#include <cstdio>
#include <unistd.h>
#include <sys/wait.h>
#endif

namespace utils {

class PersistentShell {
private:
#ifdef _WIN32
    HANDLE hChildStdinWrite, hChildStdoutRead;
    PROCESS_INFORMATION procInfo;
#else
    FILE* shell;
#endif
    std::string path, separator;

public:
    PersistentShell();
    ~PersistentShell();
    void init();
    std::string execute(const std::string& command);
    void clear_output();
    std::string get_path();
    void reset();
};

};

#endif
