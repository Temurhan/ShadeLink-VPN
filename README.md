# ShadeLink VPN 🛡️
**Author:** Reimbayew Temur

ShadeLink is a lightweight, high-performance VPN tunnel for Linux written in C++. It features a real-time terminal dashboard (TUI) to monitor your traffic and connection status.

## Key Features
* **AES-256-GCM Encryption:** Secure data transmission using OpenSSL.
* **TUI Dashboard:** Live monitoring of TX/RX traffic, logs, and connection status using Ncurses.
* **Low Latency:** Built with `epoll` for efficient event-driven networking.
* **Easy Installation:** Provided as a Debian package for Kali Linux and Ubuntu.

## Preview
![ShadeLink Dashboard](https://github.com/Temurhan/ShadeLink-VPN/raw/main/preview.png)

## Installation

1. **Download the latest release:**
   Go to the [Releases](https://github.com/Temurhan/ShadeLink-VPN/releases) page and download `shadelink_1.0.deb`.

2. **Install the package:**
   ```bash
   sudo apt install ./shadelink_1.0.deb
