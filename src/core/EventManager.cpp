#include "EventManager.hpp"
#include <iostream>

EventManager::EventManager(int max_events) : _max_events(max_events) {
    _epoll_fd = epoll_create1(0);
    if (_epoll_fd < 0) {
        throw std::runtime_error("Error creating epoll");
    }
    _events.resize(_max_events);
}

EventManager::~EventManager() {
    if (_epoll_fd >= 0) {
        close(_epoll_fd);
    }
}

void EventManager::add_fd(int fd, uint32_t event_flags) {
    struct epoll_event ev;
    ev.events = event_flags;
    ev.data.fd = fd;

    if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        throw std::runtime_error("epoll_ctl error: ADD");
    }
}

int EventManager::wait(int timeout_ms) {
    int nfds = epoll_wait(_epoll_fd, _events.data(), _max_events, timeout_ms);
    if (nfds == -1 && errno != EINTR) {
        perror("epoll_wait");
    }
    return nfds;
}

const struct epoll_event& EventManager::get_event(int index) const {
    return _events.at(index);
}