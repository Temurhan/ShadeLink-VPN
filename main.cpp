#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <deque>
#include <arpa/inet.h>
#include <ncurses.h>
#include <cstring>
#include <algorithm>
#include <csignal>
#include <atomic>
#include <unistd.h>
#include <sys/wait.h>
#include <chrono>
#include <thread>
#include <mutex>
#include <exception>

#include "core/TunInterface.hpp"
#include "network/SocketHandler.hpp"
#include "core/EventManager.hpp"
#include "crypto/Encryptor.hpp"
#include <unistd.h>


// --- 1. КОНСТАНТЫ И СТРУКТУРЫ ---
const size_t MTU = 1400;
const size_t MAX_BUF = 4096;

struct Config {
    std::string mode;
    std::string remote_ip;
    int port;
    std::string key_str;
};

struct SharedState {
    std::atomic<long> tx{0}, rx{0};
    std::deque<std::string> log_queue;
    std::mutex log_mtu;
    std::atomic<bool> is_running{true};
    std::exception_ptr worker_exception = nullptr;
};

SharedState state;

// --- 2. ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ (ОБЪЯВЛЕНИЕ) ---

void add_log(const std::string& msg) {
    std::lock_guard<std::mutex> lock(state.log_mtu);
    state.log_queue.push_back(msg);
    if (state.log_queue.size() > 8) state.log_queue.pop_front();
}

void safe_exec(const std::vector<std::string>& args) {
    pid_t pid = fork();
    if (pid == 0) {
        std::vector<char*> c_args;
        for (const auto& arg : args) c_args.push_back(const_cast<char*>(arg.c_str()));
        c_args.push_back(nullptr);
        execvp(c_args[0], c_args.data());
        exit(1);
    } else if (pid > 0) {
        waitpid(pid, nullptr, 0);
    }
}

void configure_network_safe(const std::string& dev, const std::string& ip) {
    safe_exec({"sudo", "ip", "addr", "add", ip, "dev", dev});
    safe_exec({"sudo", "ip", "link", "set", "dev", dev, "up"});
    safe_exec({"sudo", "ip", "link", "set", "dev", dev, "mtu", "1400"});
}

Config load_config() {
    std::string path = "config.txt";
    std::ifstream file(path);
    if (!file.is_open()) file.open("/etc/shadelink/config.txt");
    Config cfg = {"client", "127.0.0.1", 51820, "VERY_SECRET_KEY_32_CHARS_LONG!!!"};
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.find("mode=") == 0) cfg.mode = line.substr(5);
            else if (line.find("remote_ip=") == 0) cfg.remote_ip = line.substr(10);
            else if (line.find("port=") == 0) try { cfg.port = std::stoi(line.substr(5)); } catch(...) {}
            else if (line.find("key=") == 0) cfg.key_str = line.substr(4);
        }
    }
    return cfg;
}

void draw_ui(const Config& cfg) {
    clear();
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    attron(COLOR_PAIR(1)); box(stdscr, 0, 0); attroff(COLOR_PAIR(1));

    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(1, 4, "  ____  _                      _     _       _      ");
    mvprintw(2, 4, " / ___|| |__   __ _  __| | ___| |   (_)_ __ | | __  ");
    mvprintw(3, 4, " \\___ \\| '_ \\ / _` |/ _` |/ _ \\ |   | | '_ \\| |/ /  ");
    mvprintw(4, 4, "  ___) | | | | (_| | (_| |  __/ |___| | | | |   <   ");
    mvprintw(5, 4, " |____/|_| |_|\\__,_|\\__,_|\\___|_____|_|_| |_|_|\\_\\  ");
    mvprintw(6, 10, "--- [ ShadeLink VPN v2.0 - Reimbayew Temur ] ---");
    attroff(COLOR_PAIR(1) | A_BOLD);

    mvprintw(8, 4,  "Status:  [ %s ]", state.is_running ? "ACTIVE" : "ERROR/STOP");
    mvprintw(9, 4,  "Mode:    %s", cfg.mode.c_str());
    mvprintw(10, 4, "Remote:  %s:%d", cfg.remote_ip.c_str(), cfg.port);

    attron(COLOR_PAIR(2));
    mvprintw(12, 4, "Traffic: TX %ld KB | RX %ld KB", state.tx.load()/1024, state.rx.load()/1024);
    attroff(COLOR_PAIR(2));

    mvprintw(14, 2, " System Events:");
    std::lock_guard<std::mutex> lock(state.log_mtu);
    for (int i = 0; i < (int)state.log_queue.size(); ++i) {
        if (15 + i < max_y - 2) mvprintw(15 + i, 4, ">> %s", state.log_queue[i].c_str());
    }
    mvprintw(max_y - 2, 2, "Press 'q' to disconnect and exit...");
    refresh();
}

// --- 3. DATA PLANE (СЕТЕВОЙ ПОТОК) ---

void network_worker(Config cfg) {
    try {
        bool is_server = (cfg.mode == "server");
        TunInterface tun;
        SocketHandler net(is_server ? "0.0.0.0" : cfg.remote_ip, cfg.port);
        EventManager em(100);

        std::vector<uint8_t> key_bytes(32, 0);
        memcpy(key_bytes.data(), cfg.key_str.c_str(), std::min((size_t)32, cfg.key_str.length()));
        Encryptor encryptor(key_bytes);

        std::string tun_name = is_server ? "shadelink_srv" : "shadelink_cli";
        if (!tun.allocate(tun_name)) throw std::runtime_error("TUN allocation failed");

        std::string vpn_ip = is_server ? "10.8.0.1/24" : "10.8.0.2/24";
        configure_network_safe(tun_name, vpn_ip);
        add_log("Network UP: " + vpn_ip);

        em.add_fd(tun.get_fd(), EPOLLIN);
        em.add_fd(net.get_socket_fd(), EPOLLIN);

        struct sockaddr_in peer_addr;
        bool peer_known = !is_server;
        if (!is_server) {
            memset(&peer_addr, 0, sizeof(peer_addr)); // Это у тебя есть, хорошо
            peer_addr.sin_family = AF_INET;
            peer_addr.sin_port = htons(cfg.port);
            if (inet_pton(AF_INET, cfg.remote_ip.c_str(), &peer_addr.sin_addr) <= 0) {
                throw std::runtime_error("Invalid remote IP address");
            }
            peer_known = true;
        }

        std::vector<uint8_t> buf(MAX_BUF);
        uint8_t out_buf[MAX_BUF];

        while (state.is_running) {
            int nfds = em.wait(50);
            for (int i = 0; i < nfds; ++i) {
                int fd = em.get_event(i).data.fd;

                // --- ПЕРЕДАЧА (TUN -> NET) ---
                if (fd == tun.get_fd()) {
                    int n = tun.read_packet((char*)buf.data(), MTU);
                    if (n > 0 && peer_known) {
                        int enc_n = encryptor.encrypt(buf.data(), n, out_buf, MAX_BUF);
                        net.send_to(out_buf, enc_n, peer_addr);
                        state.tx += enc_n;
                        // Убираем лишние логи отсюда, чтобы не спамить при пинге
                    }
                }

                // --- ПРИЕМ (NET -> TUN) ---
                else if (fd == net.get_socket_fd()) {
                    struct sockaddr_in sender;
                    int n = net.receive_from(buf.data(), MAX_BUF, sender);

                    if (n > 0) {
                        // ШАГ 1: Пакет вообще долетел до программы?
                        add_log("NET: Recv " + std::to_string(n) + " bytes from " + std::string(inet_ntoa(sender.sin_addr)));

                        int dec_n = encryptor.decrypt(buf.data(), n, out_buf, MAX_BUF);

                        // ШАГ 2: Ключи совпали? (дешифровка прошла успешно)
                        if (dec_n > 0) {
                            tun.write_packet((char*)out_buf, dec_n);
                            state.rx += n;
                            add_log("RX: Packet decrypted and delivered"); // Добавь это
                            if (is_server && !peer_known) {
                                add_log("AUTH: Peer handshake successful!");
                                peer_addr = sender;
                                peer_known = true;
                            }
                        } else {
                            // Если дешифратор вернул 0 или меньше — ключи РАЗНЫЕ
                            add_log("CRYPTO ERROR: Decryption failed (Wrong key?)");

                        }
                    }
                }
            }
        }
    } catch (...) {
        state.worker_exception = std::current_exception();
        state.is_running = false;
    }
}

// --- 4. CONTROL PLANE (ГЛАВНЫЙ ПОТОК) ---

int main() {


    if (geteuid() != 0) {
        std::cerr << "ERROR: ShadeLink VPN must be run as root (use sudo)." << std::endl;
        return 1;
    }
    std::signal(SIGINT, [](int){ state.is_running = false; });

    Config cfg = load_config();

    initscr(); start_color(); noecho(); curs_set(0); nodelay(stdscr, TRUE); keypad(stdscr, TRUE);
    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);

    // Запускаем Data Plane в отдельном потоке
    std::thread worker(network_worker, cfg);

    while (state.is_running) {
        if (state.worker_exception) break;

        draw_ui(cfg);

        int ch = getch();
        if (ch == 'q' || ch == 'Q') state.is_running = false;

        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // UI FPS = 10
    }

    state.is_running = false;
    if (worker.joinable()) worker.join();
    endwin();

    if (state.worker_exception) {
        try { std::rethrow_exception(state.worker_exception); }
        catch (const std::exception& e) { std::cerr << "\n[FATAL ERROR]: " << e.what() << std::endl; }
    }

    std::cout << "[!] ShadeLink VPN stopped." << std::endl;
    return 0;
}