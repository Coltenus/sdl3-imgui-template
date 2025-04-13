#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#include <string>
#ifdef _WIN32
#include <windows.h>
#endif

namespace utils {

class SerialPort {
public:
    SerialPort();
    ~SerialPort();

    bool open(const std::string& portName, int baudRate);
    void close();
    bool write(const std::string& data);
    std::string read(std::string delimiter);
    int read(char* buffer, size_t size);
    std::string read_delim(std::string delimiter, int timeout = 1000);
    void read_file(std::string filename, int timeout = 1000);

private:
#ifdef _WIN32
    HANDLE port_handle;
#else
    int port_handle;
#endif
};

}

#endif
