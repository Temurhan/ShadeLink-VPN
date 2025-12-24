#!/bin/bash
# ShadeLink: Professional Build & Install Script

# Сохраняем корневой путь проекта СРАЗУ
ROOT_DIR=$(pwd)

echo "=== 1. Installing system dependencies ==="
sudo apt update
sudo apt install -y build-essential cmake libssl-dev libncurses-dev pkg-config net-tools

echo "=== 2. Preparing network modules ==="
sudo modprobe tun

echo "=== 3. Building the project (C++17) ==="
mkdir -p build && cd build
cmake ..
make

echo "=== 4. Setting up permissions (Capabilities) ==="
if [ -f "vpn_free" ]; then
    sudo setcap cap_net_admin,cap_net_raw+eip ./vpn_free
    echo "SUCCESS: ShadeLink binary is ready in build/ folder."
else
    echo "ERROR: Build failed. Check compilation logs above."
    exit 1
fi

echo "=== 5. Creating Desktop & Menu shortcuts ==="
# Возвращаемся в корень, чтобы ярлык создался там
cd $ROOT_DIR

cat <<EOF > shadelink.desktop
[Desktop Entry]
Version=1.0
Type=Application
Name=ShadeLink
Comment=Secure Stealth VPN by Tim Dev
# Используем полный путь к бинарнику
Exec=gnome-terminal -- title "ShadeLink Console" -- bash -c "pkexec ${ROOT_DIR}/build/vpn_free; exec bash"
# Указываем твою новую иконку
Icon=${ROOT_DIR}/icon.png
Path=${ROOT_DIR}/build/
Terminal=true
Categories=Network;Security;
EOF

# Копируем в меню приложений
mkdir -p ~/.local/share/applications
cp shadelink.desktop ~/.local/share/applications/
chmod +x ~/.local/share/applications/shadelink.desktop

# Копируем на рабочий стол
cp shadelink.desktop ~/Desktop/
chmod +x ~/Desktop/shadelink.desktop

echo "--------------------------------------------------"
echo "INSTALLATION COMPLETE!"
echo "1. Run it from your Application Menu (Search for 'ShadeLink')"
echo "2. Or use the shortcut on your Desktop"
echo "3. Don't forget to edit /etc/shadelink/config.txt later"
echo "--------------------------------------------------"