#include "TunInterface.hpp"
#include <fcntl.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

// Конструктор: инициализируем дескриптор значением -1 (не открыт)
TunInterface::TunInterface() : tun_fd(-1) {}

// Деструктор: автоматически закрываем интерфейс при выходе из области видимости
TunInterface::~TunInterface() {
    close_interface();
}

bool TunInterface::allocate(const std::string& dev_name) {
    struct ifreq ifr;

    // Открываем клонирующее устройство TUN/TAP
    if ((tun_fd = open("/dev/net/tun", O_RDWR)) < 0) {
        perror("[Core] Opening /dev/net/tun");
        return false;
    }

    std::memset(&ifr, 0, sizeof(ifr));

    // IFF_TUN: уровень IP пакетов (Layer 3)
    // IFF_NO_PI: не добавлять лишние заголовки пакетов
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

    if (!dev_name.empty()) {
        std::strncpy(ifr.ifr_name, dev_name.c_str(), IFNAMSIZ);
    }

    // Системный вызов для создания интерфейса в ядре
    if (ioctl(tun_fd, TUNSETIFF, (void*)&ifr) < 0) {
        perror("[Core] ioctl(TUNSETIFF)");
        close(tun_fd);
        tun_fd = -1;
        return false;
    }

    this->name = ifr.ifr_name;
    std::cout << "[Core] Interface " << this->name << " allocated successfully." << std::endl;
    return true;
}

ssize_t TunInterface::read_packet(char* buffer, size_t size) {
    if (tun_fd < 0) return -1;
    return read(tun_fd, buffer, size);
}

ssize_t TunInterface::write_packet(const char* buffer, size_t size) {
    if (tun_fd < 0) return -1;
    return write(tun_fd, buffer, size);
}

void TunInterface::close_interface() {
    if (tun_fd >= 0) {
        close(tun_fd);
        tun_fd = -1;
        std::cout << "[Core] Interface closed and FD released." << std::endl;
    }
}