#ifndef ARDUINO // Not Arduino
#include "SerialXCommunication.h"

#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iomanip>

SerialXCommunication::SerialXCommunication(int port)
    : port_(port), backlog_(1) {}

SerialXCommunication::~SerialXCommunication() {
    destroy();
}

bool SerialXCommunication::begin() {
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        std::cerr << "Errore creazione socket\n";
        return false;
    }

    int opt = 1;
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address_.sin_family = AF_INET;
    address_.sin_addr.s_addr = INADDR_ANY;
    address_.sin_port = htons(port_);

    if (bind(server_fd_, reinterpret_cast<sockaddr*>(&address_), sizeof(address_)) < 0) {
        std::cerr << "Errore bind sulla porta " << port_ << "\n";
        ::close(server_fd_);
        server_fd_ = -1;
        return false;
    }

    if (listen(server_fd_, backlog_) < 0) {
        std::cerr << "Errore listen\n";
        ::close(server_fd_);
        server_fd_ = -1;
        return false;
    }

    std::cout << "[SerialXCommunication] In ascolto sulla porta TCP " << port_ << "...\n";
    return true;
}

bool SerialXCommunication::open() {
    if (server_fd_ < 0) {
        std::cerr << "[SerialXCommunication] begin() non chiamato o fallito\n";
        return false;
    }

    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);
    int fd = accept(server_fd_, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
    if (fd < 0) {
        std::cerr << "Errore accept\n";
        return false;
    }

    client_fd_ = fd;

    char ipBuf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, ipBuf, sizeof(ipBuf));
    clientIp_ = ipBuf;

    std::cout << "[SerialXCommunication] Client connesso: " << clientIp_ << "\n";
    return true;
}

bool SerialXCommunication::readLine(std::string& outLine) {
    if (client_fd_ < 0) return false;

    outLine.clear();
    char c;
    while (true) {
        ssize_t n = recv(client_fd_, &c, 1, 0);
        if (n <= 0) {
            // Connessione chiusa o errore
            return false;
        }
        if (c == '\n') {
            // Rimuove eventuale '\r' finale (CRLF)
            if (!outLine.empty() && outLine.back() == '\r') {
                outLine.pop_back();
            }
            return true;
        }
        outLine.push_back(c);
    }
}

bool SerialXCommunication::sendLine(const std::string& line) {
    if (client_fd_ < 0) return false;

    std::string toSend = line + "\n";
    size_t totalSent = 0;
    while (totalSent < toSend.size()) {
        ssize_t n = send(client_fd_, toSend.data() + totalSent, toSend.size() - totalSent, 0);
        if (n <= 0) {
            std::cerr << "[SerialXCommunication] Errore invio dati\n";
            return false;
        }
        totalSent += static_cast<size_t>(n);
    }
    return true;
}

bool SerialXCommunication::sendLine() {
    return sendLine(std::string());
}

bool SerialXCommunication::sendLine(int value) {
    return sendLine(std::to_string(value));
}

bool SerialXCommunication::sendLine(long value) {
    return sendLine(std::to_string(value));
}

bool SerialXCommunication::sendLine(unsigned int value) {
    return sendLine(std::to_string(value));
}

bool SerialXCommunication::sendLine(unsigned long value) {
    return sendLine(std::to_string(value));
}

bool SerialXCommunication::sendLine(float value, int decimals) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(decimals) << value;
    return sendLine(oss.str());
}

bool SerialXCommunication::sendLine(double value, int decimals) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(decimals) << value;
    return sendLine(oss.str());
}

bool SerialXCommunication::sendLine(char value) {
    return sendLine(std::string(1, value));
}

void SerialXCommunication::close() {
    if (client_fd_ >= 0) {
        std::cout << "[SerialXCommunication] Client disconnesso: " << clientIp_ << "\n";
        ::close(client_fd_);
        client_fd_ = -1;
        clientIp_.clear();
    }
}

void SerialXCommunication::destroy() {
    close(); // Chiude il client corrente, se presente
    if (server_fd_ >= 0) {
        ::close(server_fd_);
        server_fd_ = -1;
    }
}
#endif
