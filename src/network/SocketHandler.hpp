#pragma once
#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

class SocketHandler {
public:
    SocketHandler(const std::string& ip, int port);
    ~SocketHandler();

    int get_socket_fd() const { return _sockfd; }

    // Для клиента: отправка и получение от фиксированного сервера
    void send_to_server(const uint8_t* buffer, size_t size);
    ssize_t receive_from_server(uint8_t* buffer, size_t size);

    // НОВЫЕ МЕТОДЫ: Универсальные (для режима сервера)
    ssize_t receive_from(uint8_t* buffer, size_t size, struct sockaddr_in& src_addr);
    ssize_t send_to(const uint8_t* buffer, size_t size, const struct sockaddr_in& dest_addr);

private:
    int _sockfd;
    struct sockaddr_in _server_addr;
};