# ShadeLink VPN 🛡️

**Official Repository of ShadeLink VPN** **Author:** Reimbayew Temur 

ShadeLink is a lightweight, high-performance VPN solution for Linux, written in C++ . It is designed for users who need a secure tunnel with a transparent, real-time monitoring interface directly in the terminal.

---

## 🚀 Overview
ShadeLink uses a custom protocol over UDP to establish secure tunnels between a client and a server. The built-in TUI (Terminal User Interface) allows you to track traffic and system events without needing complex external tools.

## ✨ Key Features
* **AES-256-GCM Encryption:** Secure data transmission powered by the OpenSSL library  
* **Real-time TUI Dashboard:** Live monitoring of TX/RX traffic and connection status using Ncurses  
* **Event Logging:** Integrated system event panel to track connection successes and errors  
* **Optimized Networking:** Built with `epoll` for efficient, event-driven I/O handling  
* **Custom ASCII Art:** Professional branding displayed directly in the terminal 

## 📸 Preview
![ShadeLink Dashboard](preview.png)
*(Note: Upload your screenshot to the repository and name it preview.png to see it here)*

---

## 📥 Installation

### 1. Requirements
Make sure you have the following dependencies installed:
* `libncurses5-dev`
* `libssl-dev`
* `cmake`

### 2. Quick Install (Debian/Kali)
Download the latest `.deb` package from the [Releases](https://github.com/Temurhan/ShadeLink-VPN/releases) page and run:
```bash
sudo apt install ./shadelink_1.0.deb

### 3. Build from Source
If you want to compile the project manually:
```bash
mkdir build && cd build
cmake ..
make
sudo make install

## ⚖️ License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 📫 Contact
* **Author:** Reimbayew Temur
* **GitHub:** [@Temurhan](https://github.com/Temurhan)

