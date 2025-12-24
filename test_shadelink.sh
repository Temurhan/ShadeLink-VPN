#!/bin/bash

# Цвета для вывода
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}[*] Starting ShadeLink VPN Automated Test...${NC}"

# 1. Создаем временные конфиги
echo "mode=server
port=51820
key=VERY_SECRET_KEY_32_CHARS_LONG!!!12" > config_server.txt

echo "mode=client
remote_ip=127.0.0.1
port=51820
key=VERY_SECRET_KEY_32_CHARS_LONG!!!12" > config_client.txt

# 2. Собираем проект (предполагаем, что есть Makefile)
echo -e "${GREEN}[*] Compiling...${NC}"
make clean && make
if [ $? -ne 0 ]; then
    echo -e "${RED}[!] Compilation failed!${NC}"
    exit 1
fi

# 3. Запускаем сервер в фоне
echo -e "${GREEN}[*] Launching Server...${NC}"
sudo ./shadelink config_server.txt &
SRV_PID=$!
sleep 2

# 4. Запускаем клиент в фоне
echo -e "${GREEN}[*] Launching Client...${NC}"
sudo ./shadelink config_client.txt &
CLI_PID=$!
sleep 3

# 5. ТЕСТ: Пинг внутри VPN канала
echo -e "${GREEN}[*] Testing connectivity (Ping 10.8.0.1)...${NC}"
ping -c 4 10.8.0.1

if [ $? -eq 0 ]; then
    echo -e "${GREEN}[V] TEST PASSED: VPN Tunnel is working!${NC}"
else
    echo -e "${RED}[X] TEST FAILED: No connectivity between nodes.${NC}"
fi

# 6. Уборка
echo -e "${GREEN}[*] Cleaning up...${NC}"
sudo kill $SRV_PID $CLI_PID
rm config_server.txt config_client.txt
echo -e "${GREEN}[*] Done.${NC}"