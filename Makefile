CXX = g++
# Добавляем -Isrc, чтобы пути типа "core/..." работали внутри папки src
CXXFLAGS = -std=c++17 -Wall -I. -Isrc
LDFLAGS = -lncurses -lpthread -lssl -lcrypto

SOURCES = main.cpp src/core/TunInterface.cpp src/network/SocketHandler.cpp src/core/EventManager.cpp src/crypto/Encryptor.cpp
TARGET = shadelink

all:
	$(CXX) $(CXXFLAGS) $(SOURCES) $(LDFLAGS) -o $(TARGET)

clean:
	rm -f $(TARGET)