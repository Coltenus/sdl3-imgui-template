#include "utils/shell.h"

namespace utils {

PersistentShell::PersistentShell() {
#ifdef _WIN32
    separator = "> ";
#else
    separator = "$ ";
#endif
    init();
}

PersistentShell::~PersistentShell() {
#ifdef _WIN32
    CloseHandle(hChildStdinWrite);
    CloseHandle(hChildStdoutRead);
    TerminateProcess(procInfo.hProcess, 0);
    CloseHandle(procInfo.hProcess);
    CloseHandle(procInfo.hThread);
#else
    if (shell) {
        pclose(shell);
    }
#endif
}

void PersistentShell::init() {
    if(path.size() > 0) {
        return;
    }
#ifdef _WIN32
    SECURITY_ATTRIBUTES saAttr{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
    HANDLE hChildStdinRead, hChildStdoutWrite;

    if (!CreatePipe(&hChildStdoutRead, &hChildStdoutWrite, &saAttr, 0) ||
        !SetHandleInformation(hChildStdoutRead, HANDLE_FLAG_INHERIT, 0) ||
        !CreatePipe(&hChildStdinRead, &hChildStdinWrite, &saAttr, 0) ||
        !SetHandleInformation(hChildStdinWrite, HANDLE_FLAG_INHERIT, 0)) {
        throw std::runtime_error("Failed to create pipes");
    }

    STARTUPINFO siStartInfo = {};
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdOutput = hChildStdoutWrite;
    siStartInfo.hStdError = hChildStdoutWrite;
    siStartInfo.hStdInput = hChildStdinRead;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    if (!CreateProcess(nullptr, (LPSTR)"powershell.exe", nullptr, nullptr, TRUE, 0, nullptr, nullptr, &siStartInfo, &procInfo)) {
        throw std::runtime_error("Failed to start powershell.exe");
    }

    CloseHandle(hChildStdoutWrite);
    CloseHandle(hChildStdinRead);
#else
    shell = popen("/bin/bash", "w+");
    if (!shell) {
        throw std::runtime_error("Failed to start bash");
    }
#endif

    clear_output();
}

std::string PersistentShell::execute(const std::string& command) {
    std::string result;
    char buffer[4096];

#ifdef _WIN32
    DWORD dwWritten;
    WriteFile(hChildStdinWrite, command.c_str(), command.size(), &dwWritten, nullptr);
    WriteFile(hChildStdinWrite, "\n", 1, &dwWritten, nullptr);

    DWORD dwRead;
    while (ReadFile(hChildStdoutRead, buffer, sizeof(buffer), &dwRead, nullptr) && dwRead > 0) {
        buffer[dwRead] = '\0';
        result.append(buffer, dwRead);

        if (result.find(separator) != std::string::npos) {
            break;
        }
    }    
#else
    fprintf(shell, "%s\n", command.c_str());
    fflush(shell);

    while (fgets(buffer, sizeof(buffer), shell)) {
        result.append(buffer);

        if (result.find(separator) != std::string::npos) {
            break;
        }
    }
#endif

    try {
        result.erase(0, result.find('\n') + 1);

        path = result;
        path.erase(0, path.find_last_of('\n') + 1);
        path = path.substr(0, path.size() - 2);
        path = path.substr(3);

        if(result.find_last_of('\n') != std::string::npos) {
            result.erase(result.find_last_of('\n'), result.size() - result.find_last_of('\n'));
        }
        else {
            result = "";
        }
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        if(result.size() == 0) {
            std::cerr << "No output from command: " << command << std::endl;
        }
        else {
            std::cerr << "Output from command: " << command << std::endl;
            std::cerr << result << std::endl;
        }
    }

    return result;
}

void PersistentShell::clear_output() {
    std::string result;
#ifdef _WIN32
    DWORD dwRead;
    char buffer[4096];
    while (ReadFile(hChildStdoutRead, buffer, sizeof(buffer), &dwRead, nullptr) && dwRead > 0) {
        buffer[dwRead] = '\0';
        result.append(buffer, dwRead);

        if (result.find(separator) != std::string::npos) {
            break;
        }
    }
#else
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), shell)) {
        result.append(buffer);

        if (result.find(separator) != std::string::npos) {
            break;
        }
    }
#endif

    path = result;
    path.erase(0, path.find_last_of('\n') + 1);
    path = path.substr(0, path.size() - 2);
    path = path.substr(3);
}

std::string PersistentShell::get_path() {
    return path;
}

void PersistentShell::reset() {
    path.clear();
#ifdef _WIN32
    TerminateProcess(procInfo.hProcess, 0);
    CloseHandle(procInfo.hProcess);
    CloseHandle(procInfo.hThread);
    init();
#else
    pclose(shell);
    init();
#endif
}

};
