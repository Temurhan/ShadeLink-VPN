<div align="center">

  <img src="https://img.icons8.com/fluency/144/shield-lock.png" alt="ShadeLink Logo" width="120" />

  # 🛡️ ShadeLink VPN v2.0
  
  **A High-Performance, Multi-Threaded C++ L3 VPN Tunnel**

  [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
  [![C++ Standard](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
  [![Platform](https://img.shields.io/badge/Platform-Linux-orange.svg)](https://www.linux.org/)
  [![Security](https://img.shields.io/badge/Security-OpenSSL-red.svg)](https://www.openssl.org/)

  [Description](#-about) • [Features](#-key-features) • [Installation](#-quick-start) • [Architecture](#-internal-architecture) • [Author](#-author)

</div>

---

## 📖 About
**ShadeLink** is a high-performance, multi-threaded VPN tunnel written in **C++17**. It operates at the **L3 (IP) level** using Linux virtual TUN interfaces and provides secure data transmission via symmetric encryption.

---

## 🚀 Key Features

* **⚡ Data/Control Plane Separation:** Network logic runs in a dedicated high-priority thread to eliminate packet latency during UI rendering.
* **🖥️ Interactive TUI:** Real-time traffic monitoring (TX/RX) and system event logging powered by the `ncurses` library.
* **🔒 Hardened Security:**
    * Symmetric traffic encryption via **OpenSSL** (AES/ChaCha20).
    * Secure system command execution using `fork/exec`.
    * Automatic **root privilege** verification.
* **📈 Efficiency:** Utilizes `epoll` via a custom `EventManager` for non-blocking descriptor handling.

---

## 🛠 Tech Stack

| Component | Technology |
| :--- | :--- |
| **Language** | C++17 |
| **Networking** | Linux TUN/TAP, UDP Sockets |
| **Cryptography** | OpenSSL (AES / ChaCha20) |
| **Interface** | ncurses |
| **Build System** | Makefile |

---

## 📦 Quick Start

### 1. Prerequisites
Ensure your system has the necessary headers for ncurses and OpenSSL:

```bash
sudo apt update
sudo apt install libncurses5-dev libncursesw5-dev libssl-dev build-essential
```

##2. Compilation

Clone the repository and build the binary:

```bash
git clone [https://github.com/Temurhan/ShadeLink-VPN.git](https://github.com/Temurhan/ShadeLink-VPN.git)
cd ShadeLink-VPN
make
```

##3. Configuration

Create a config.txt file in the project root:

```ini
mode=server        # server or client
port=51820
key=YOUR_32_CHAR_SECRET_KEY_HERE
remote_ip=127.0.0.1 # client mode only
```

##4. Run

The application requires root privileges to manage TUN interfaces:

```Bash
sudo ./shadelink
```

🏗 Internal Architecture
    network_worker: The core engine. Reads packets from TUN, encrypts them, and pumps them into the UDP socket.
    SharedState: A thread-safe storage for metrics and logs using std::atomic and mutexes.
    TunInterface: A C++ wrapper for /dev/net/tun interaction.
    Encryptor: The cryptographic module ensuring data confidentiality.

👨‍💻 Author

Reimbayew Temur System Software & Network Solutions Developer

<div align="center"> <sub>Built with ❤️ for secure networking.</sub> </div>
