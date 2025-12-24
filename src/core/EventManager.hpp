#ifndef EVENT_MANAGER_HPP
#define EVENT_MANAGER_HPP

#include <sys/epoll.h>
#include <vector>
#include <stdexcept>
#include <unistd.h>

class EventManager {
public:
    explicit EventManager(int max_events = 10);
    ~EventManager();

    // Запрещаем копирование /We prohibit copying
    EventManager(const EventManager&) = delete;
    EventManager& operator=(const EventManager&) = delete;

    // Регистрация дескриптора (TUN или Socket)
    // Registering a descriptor (TUN or Socket)
    void add_fd(int fd, uint32_t event_flags);

    // Ожидание событий
    //Waiting for events
    int wait(int timeout_ms = -1);

    // Получение данных о конкретном событии по индексу
    //Getting data about a specific event by index
    const struct epoll_event& get_event(int index) const;

private:
    int _epoll_fd;
    int _max_events;
    std::vector<struct epoll_event> _events;
};

#endif