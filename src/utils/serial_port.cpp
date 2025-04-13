#include "utils/serial_port.h"
#ifdef _WIN32
#include <windows.h>
#include <setupapi.h>
#include <devguid.h>
#include <regstr.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#endif
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <fstream>

namespace utils {

SerialPort::SerialPort() : port_handle(nullptr) {}

SerialPort::~SerialPort() {
	close();
}

bool SerialPort::open(const std::string& port_name, int baud_rate) {
    if(port_handle) {
        close();
    }
#ifdef _WIN32
	// Windows implementation
	std::string full_port_name = "\\\\.\\" + port_name;
	port_handle = CreateFileA(full_port_name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
	
	if (port_handle == INVALID_HANDLE_VALUE) {
		port_handle = nullptr;
		return false;
	}
	
	DCB dcb = {0};
	dcb.DCBlength = sizeof(DCB);
	
	if (!GetCommState(port_handle, &dcb)) {
		close();
		return false;
	}
	
	dcb.BaudRate = baud_rate;
	dcb.ByteSize = 8;
	dcb.StopBits = ONESTOPBIT;
	dcb.Parity = NOPARITY;
	
	if (!SetCommState(port_handle, &dcb)) {
		close();
		return false;
	}

    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(port_handle, &timeouts)) {
        fprintf(stderr, "Error setting timeouts\n");
        close();
        return INVALID_HANDLE_VALUE;
    }
	
	return true;
#else
	// Unix/Linux implementation
	port_handle = open(port_name.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
	
	if (port_handle < 0) {
		port_handle = nullptr;
		return false;
	}
	
	struct termios options;
	tcgetattr(port_handle, &options);
	
	// Set baud rate
	cfsetispeed(&options, baud_rate);
	cfsetospeed(&options, baud_rate);
	
	// 8N1 mode
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
	
	// Apply settings
	tcsetattr(port_handle, TCSANOW, &options);

    // Set timeouts
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 50000; // 50ms
    if (setsockopt(port_handle, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) < 0) {
        close();
        return false;
    }
    timeout.tv_sec = 0;
    timeout.tv_usec = 50000; // 50ms
    if (setsockopt(port_handle, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout)) < 0) {
        close();
        return false;
    }
    // Set non-blocking mode
    int flags = fcntl(port_handle, F_GETFL, 0);
    if (flags == -1) {
        close();
        return false;
    }
    if (fcntl(port_handle, F_SETFL, flags | O_NONBLOCK) == -1) {
        close();
        return false;
    }
	
	return true;
#endif
}

void SerialPort::close() {
	if (port_handle) {
#ifdef _WIN32
		CloseHandle(port_handle);
#else
		::close(port_handle);
#endif
		port_handle = nullptr;
	}
}

bool SerialPort::write(const std::string& data) {
	if (!port_handle) return false;
	
#ifdef _WIN32
	DWORD bytes_written;
	return WriteFile(port_handle, data.c_str(), data.size(), &bytes_written, nullptr) != 0;
#else
	return ::write(port_handle, data.c_str(), data.size()) > 0;
#endif
}

std::string SerialPort::read(std::string delimiter) {
	if (!port_handle || delimiter.empty()) return "";
	
	std::string result;
	char buffer;
	
    // non-blocking read
#ifdef _WIN32
    DWORD bytes_read;
    while (ReadFile(port_handle, &buffer, 1, &bytes_read, nullptr) && bytes_read > 0) {
        result += buffer;
		if (result.find(delimiter) != std::string::npos) {
			break;
		}
    }
#else
	ssize_t bytes_read;
	while ((bytes_read = ::read(port_handle, &buffer, 1)) > 0) {
		result += buffer;
		if (result.find(delimiter) != std::string::npos) {
			break;
		}
	}
#endif
	
	return result;
}

int SerialPort::read(char* buffer, size_t size) {
	if (!port_handle || !buffer || size == 0) return 0;

	char result[size];
	int bytes = 0;
#ifdef _WIN32
	DWORD bytes_read;
	if (ReadFile(port_handle, result, size, &bytes_read, nullptr) == 0) {
		memset(result, 0, sizeof(result));
		return 0;
	}
	bytes = bytes_read;
#else
	ssize_t bytes_read = ::read(port_handle, result, size);
	if (bytes_read < 0) {
		memset(result, 0, sizeof(result));
		return 0;
	}
	bytes = bytes_read;
#endif

	// Copy the read data to the provided buffer
	std::memcpy(buffer, result, std::min(size, sizeof(result)));
	return bytes;
}

std::string SerialPort::read_delim(std::string delimiter, int timeout) {
	if (!port_handle || delimiter.empty()) return "";
	
	std::string result;
	std::chrono::steady_clock::time_point last_time = std::chrono::steady_clock::now();

	while (true) {
		std::string temp = read(delimiter);
		if (temp.empty()) {
			if(std::chrono::steady_clock::now() - last_time > std::chrono::milliseconds(timeout)) {
				break; // Timeout reached
			}
			continue;
		}
		last_time = std::chrono::steady_clock::now();
		result += temp;
		if (result.find(delimiter) != std::string::npos) {
			break;
		}
	}

	if (result.size() >= delimiter.size()) {
		result.erase(result.end() - delimiter.size(), result.end());
	}
	else {
		result.clear();
	}
	
	return result;
}

void SerialPort::read_file(std::string filename, int timeout) {
	if (!port_handle || filename.empty()) return;
	
	std::chrono::steady_clock::time_point last_time = std::chrono::steady_clock::now();
	std::ofstream file(filename, std::ios::binary);
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file for writing.");
	}

	char buf[2];
	
	while (true) {
		int bytes = read(buf, sizeof(buf));
		if(bytes == 0) {
			if(std::chrono::steady_clock::now() - last_time > std::chrono::milliseconds(timeout)) {
				break; // Timeout reached
			}
			continue;
		}
		last_time = std::chrono::steady_clock::now();
		if(bytes == 1) {
			file.write(buf, 1);
		}
		else if(buf[0] == '\r' && buf[1] == '\n') {
			break;
		}
		else {
			file.write(buf, sizeof(buf));
		}
	}
	file.close();
}

} // namespace utils

