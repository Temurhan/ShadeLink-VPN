#!/bin/bash

APP_NAME="shadelink"
VERSION="1.0"
BUILD_ROOT="deb_package"

echo "=== 1. Compiling the project ==="
mkdir -p build && cd build
cmake ..
make
cd ..

echo "=== 2. Creating folder structure ==="
# Полная очистка и создание структуры
rm -rf $BUILD_ROOT
mkdir -p $BUILD_ROOT/DEBIAN
mkdir -p $BUILD_ROOT/usr/bin
mkdir -p $BUILD_ROOT/etc/$APP_NAME
mkdir -p $BUILD_ROOT/usr/share/applications
mkdir -p $BUILD_ROOT/usr/share/pixmaps

# Копируем бинарный файл
cp build/vpn_free $BUILD_ROOT/usr/bin/$APP_NAME

# Создаем конфиг (если его нет в build, создаем дефолтный)
cat <<EOF > $BUILD_ROOT/etc/$APP_NAME/config.txt
mode=client
remote_ip=127.0.0.1
port=51820
key=secret_key
EOF

# Копируем иконку (убедись, что icon.png лежит в корне vpn_free)
cp icon.png $BUILD_ROOT/usr/share/pixmaps/$APP_NAME.png

echo "=== 3. Creating Control files ==="
cat <<EOF > $BUILD_ROOT/DEBIAN/control
Package: $APP_NAME
Version: $VERSION
Architecture: amd64
Maintainer: Reimbayew Temur
Depends: libssl-dev, libncursesw6, net-tools
Description: ShadeLink VPN - Professional Stealth Tunneling Tool.
EOF

# Файл со списком конфигов (чтобы не затирались при обновлении)
echo "/etc/$APP_NAME/config.txt" > $BUILD_ROOT/DEBIAN/conffiles

# Скрипт после установки (права доступа)
cat <<EOF > $BUILD_ROOT/DEBIAN/postinst
#!/bin/bash
chmod +x /usr/bin/$APP_NAME
setcap cap_net_admin,cap_net_raw=eip /usr/bin/$APP_NAME
echo "ShadeLink VPN has been installed successfully!"
EOF
chmod 755 $BUILD_ROOT/DEBIAN/postinst

# Создаем ярлык
cat <<EOF > $BUILD_ROOT/usr/share/applications/$APP_NAME.desktop
[Desktop Entry]
Name=ShadeLink VPN
Comment=Stealth VPN by ShadeLink Team
Exec=x-terminal-emulator -e bash -c "pkexec /usr/bin/$APP_NAME; read"
Icon=/usr/share/pixmaps/$APP_NAME.png
Terminal=false
Type=Application
Categories=Network;Security;
EOF

echo "=== 4. Packaging .deb ==="
dpkg-deb --root-owner-group --build $BUILD_ROOT ${APP_NAME}_${VERSION}.deb

echo "Done! Package created: ${APP_NAME}_${VERSION}.deb"