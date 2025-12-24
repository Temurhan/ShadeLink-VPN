#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <deque>
#include <arpa/inet.h>
#include <ncurses.h>
#include <cstring>
#include <algorithm>
#include "core/TunInterface.hpp"
#include "network/SocketHandler.hpp"
#include "core/EventManager.hpp"
#include "crypto/Encryptor.hpp"

struct Config {
    std::string mode;
    std::string remote_ip;
    int port;
    std::string key_str;
};

std::deque<std::string> logs;
long total_sent = 0, total_recv = 0;

void add_log(const std::string& msg) {
    logs.push_back(msg);
    if (logs.size() > 8) logs.pop_front();
}

Config load_config() {
    std::string path = "config.txt";
    std::ifstream file(path);
    if (!file.is_open()) {
        path = "/etc/shadelink/config.txt";
        file.open(path);
    }

    // Значения по умолчанию
    Config cfg = {"client", "127.0.0.1", 51820, "VERY_SECRET_KEY_32_CHARS_LONG!!!"};

    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            // Убираем возможные пробелы в начале или конце строки (опционально)
            if (line.empty()) continue;

            if (line.find("mode=") == 0) {
                cfg.mode = line.substr(5);
            } else if (line.find("remote_ip=") == 0) {
                cfg.remote_ip = line.substr(10);
            } else if (line.find("port=") == 0) {
                try {
                    cfg.port = std::stoi(line.substr(5));
                } catch (...) { cfg.port = 51820; }
            } else if (line.find("key=") == 0) {
                cfg.key_str = line.substr(4);
            }
        }
    }
    return cfg;
}

// ... (все твои include и Config остаются без изменений) ...

void draw_ui(const Config& cfg) {
    // УДАЛИЛИ отсюда цикл while(running)
    clear();
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    attron(COLOR_PAIR(1));
    box(stdscr, 0, 0);
    attroff(COLOR_PAIR(1));

    // Твой ASCII ART
    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(1, 4, "  ____  _                      _     _       _      ");
    mvprintw(2, 4, " / ___|| |__   __ _  __| | ___| |   (_)_ __ | | __  ");
    mvprintw(3, 4, " \\___ \\| '_ \\ / _` |/ _` |/ _ \\ |   | | '_ \\| |/ /  ");
    mvprintw(4, 4, "  ___) | | | | (_| | (_| |  __/ |___| | | | |   <   ");
    mvprintw(5, 4, " |____/|_| |_|\\__,_|\\__,_|\\___|_____|_|_| |_|_|\\_\\  ");
    mvprintw(6, 10, "--- [ AUTHORED BY: Reimbayew Temur ] ---");
    attroff(COLOR_PAIR(1) | A_BOLD);

    mvprintw(8, 4,  "Status:  [ ACTIVE ]");
    mvprintw(9, 4,  "Mode:    %s", cfg.mode.c_str());
    mvprintw(10, 4, "Remote:  %s:%d", cfg.remote_ip.c_str(), cfg.port);

    attron(COLOR_PAIR(2));
    mvprintw(12, 4, "Traffic: TX %ld KB | RX %ld KB", total_sent/1024, total_recv/1024);
    attroff(COLOR_PAIR(2));

    mvprintw(14, 2, " System Events:");
    for (int i = 0; i < (int)logs.size(); ++i) {
        if (15 + i < max_y - 3) {
            mvprintw(15 + i, 4, ">> %s", logs[i].c_str());
        }
    }

    mvprintw(max_y - 2, 2, "Press 'q' to disconnect and exit...");
    refresh();
}

int main() {
    Config cfg = load_config();
    bool is_server = (cfg.mode == "server");

    // Инициализация ncurses
    initscr();
    start_color();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE); // НЕ блокировать программу при ожидании клавиши
    keypad(stdscr, TRUE);

    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);

    try {
        // ... (твой код инициализации TUN, Socket, Encryptor остается прежним) ...
        std::string tun_name = is_server ? "tun_srv" : "tun_cli";
        TunInterface tun;
        SocketHandler net(is_server ? "0.0.0.0" : cfg.remote_ip, cfg.port);
        EventManager em(10);

        std::vector<uint8_t> key_bytes(32, 0);
        memcpy(key_bytes.data(), cfg.key_str.c_str(), std::min((size_t)32, cfg.key_str.length()));
        Encryptor encryptor(key_bytes);

        if (!tun.allocate(tun_name)) throw std::runtime_error("TUN failed");

        std::string vpn_ip = is_server ? "10.0.0.1" : "10.0.0.2";
        system(("sudo ip link set dev " + tun_name + " up").c_str());
        system(("sudo ip addr add " + vpn_ip + "/24 dev " + tun_name).c_str());

        em.add_fd(tun.get_fd(), EPOLLIN);
        em.add_fd(net.get_socket_fd(), EPOLLIN);

        struct sockaddr_in peer_addr;
        bool peer_known = !is_server;
        if (!is_server) {
            memset(&peer_addr, 0, sizeof(peer_addr));
            peer_addr.sin_family = AF_INET;
            peer_addr.sin_port = htons(cfg.port);
            inet_pton(AF_INET, cfg.remote_ip.c_str(), &peer_addr.sin_addr);
        }

        std::vector<uint8_t> buf(4096);
        uint8_t out_buf[4096];
        add_log("ShadeLink started successfully");

        // ОСНОВНОЙ ЦИКЛ
        bool running = true;
        while (running) {
            draw_ui(cfg);

            // ПРОВЕРКА КЛАВИШИ 'q'
            int ch = getch();
            if (ch == 'q' || ch == 'Q') {
                running = false;
                break;
            }

            // Обработка сети с маленьким тайм-аутом (50мс)
            int nfds = em.wait(50);
            for (int i = 0; i < nfds; ++i) {
                int fd = em.get_event(i).data.fd;
                if (fd == tun.get_fd()) {
                    int n = tun.read_packet((char*)buf.data(), 1400);
                    if (n > 0 && peer_known) {
                        int enc_n = encryptor.encrypt(buf.data(), n, out_buf, 4096);
                        net.send_to(out_buf, enc_n, peer_addr);
                        total_sent += enc_n;
                    }
                } else if (fd == net.get_socket_fd()) {
                    struct sockaddr_in sender;
                    int n = net.receive_from(buf.data(), buf.size(), sender);
                    if (n > 0) {
                        int dec_n = encryptor.decrypt(buf.data(), n, out_buf, 4096);
                        if (dec_n > 0) {
                            tun.write_packet((char*)out_buf, dec_n);
                            total_recv += n;
                            if (is_server && !peer_known) {
                                add_log("Peer connected");
                                peer_addr = sender; peer_known = true;
                            }
                        }
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        endwin();
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    endwin();
    std::cout << "\n[!] ShadeLink VPN stopped by user." << std::endl;
    return 0;
}
