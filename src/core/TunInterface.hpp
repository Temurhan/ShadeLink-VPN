#ifndef TUN_INTERFACE_HPP
#define TUN_INTERFACE_HPP

#include <string>
#include <linux/if.h>
#include <sys/types.h> // Для ssize_t

class TunInterface {
public:
    TunInterface();
    ~TunInterface();

    // Запрещаем копирование (важно для работы с дескрипторами)
    TunInterface(const TunInterface&) = delete;
    TunInterface& operator=(const TunInterface&) = delete;

    // Метод для создания интерфейса
    bool allocate(const std::string& dev_name);

    // Чтение/Запись
    ssize_t read_packet(char* buffer, size_t size);
    ssize_t write_packet(const char* buffer, size_t size);

    void close_interface();

    // Геттер для дескриптора (понадобится для epoll)
    int get_fd() const { return tun_fd; }

private:
    int tun_fd;
    std::string name;
};

#endif // TUN_INTERFACE_HPP