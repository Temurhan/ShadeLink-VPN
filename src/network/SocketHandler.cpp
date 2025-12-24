#include "SocketHandler.hpp"
#include <cstring>
#include <stdexcept>

SocketHandler::SocketHandler(const std::string& ip, int port) {
    _sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (_sockfd < 0) throw std::runtime_error("Socket creation failed");

    memset(&_server_addr, 0, sizeof(_server_addr));
    _server_addr.sin_family = AF_INET;
    _server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &_server_addr.sin_addr);

    // Если мы сервер (IP 0.0.0.0), нужно сделать bind
    if (ip == "0.0.0.0") {
        if (bind(_sockfd, (struct sockaddr*)&_server_addr, sizeof(_server_addr)) < 0) {
            throw std::runtime_error("Bind failed");
        }
    }
}

SocketHandler::~SocketHandler() {
    if (_sockfd >= 0) close(_sockfd);
}

void SocketHandler::send_to_server(const uint8_t* buffer, size_t size) {
    sendto(_sockfd, buffer, size, 0, (struct sockaddr*)&_server_addr, sizeof(_server_addr));
}

ssize_t SocketHandler::receive_from_server(uint8_t* buffer, size_t size) {
    socklen_t addr_len = sizeof(_server_addr);
    return recvfrom(_sockfd, buffer, size, 0, (struct sockaddr*)&_server_addr, &addr_len);
}

// Реализация новых методов
ssize_t SocketHandler::receive_from(uint8_t* buffer, size_t size, struct sockaddr_in& src_addr) {
    socklen_t addr_len = sizeof(src_addr);
    return recvfrom(_sockfd, buffer, size, 0, (struct sockaddr*)&src_addr, &addr_len);
}

ssize_t SocketHandler::send_to(const uint8_t* buffer, size_t size, const struct sockaddr_in& dest_addr) {
    return sendto(_sockfd, buffer, size, 0, (const struct sockaddr*)&dest_addr, sizeof(dest_addr));
}