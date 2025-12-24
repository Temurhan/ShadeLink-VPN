cmake_minimum_required(VERSION 3.10)
project(vpn_free)

set(CMAKE_CXX_STANDARD 17)

# Поиск библиотек
find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(NCURSES REQUIRED ncursesw)

# Указываем путь к твоим папкам core, network, crypto
# Теперь компилятор увидит "core/TunInterface.hpp" внутри папки src
include_directories(src)
include_directories(${NCURSES_INCLUDE_DIRS})

# Список файлов для сборки
add_executable(vpn_free
        main.cpp
        src/core/TunInterface.cpp
        src/network/SocketHandler.cpp
        src/core/EventManager.cpp
        src/crypto/Encryptor.cpp
)

# Линковка
target_link_libraries(vpn_free
        OpenSSL::Crypto
        ${NCURSES_LIBRARIES}
)