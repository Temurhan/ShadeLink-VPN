import socket
from cryptography.hazmat.primitives.ciphers.aead import ChaCha20Poly1305

# Настройки
LISTEN_IP = "127.0.0.1"
LISTEN_PORT = 51820
KEY = b'\x42' * 32  # Твой ключ из C++ /Your key from C++

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((LISTEN_IP, LISTEN_PORT))

chacha = ChaCha20Poly1305(KEY)

print(f"The server is listening on {LISTEN_IP}:{LISTEN_PORT}...")

while True:
    data, addr = sock.recvfrom(2048)

    # Пакет/Package: [NONCE(12)][DATA(...)][TAG(16)]
    nonce = data[:12]
    ciphertext = data[12:] # Включает в себя данные и тег в конце  /Includes data and tag at the end

    try:
        # Расшифровываем /Deciphering
        decrypted_data = chacha.decrypt(nonce, ciphertext, None)
        print(f"--- Received package from {addr} ---")
        print(f"Size: {len(decrypted_data)} bayt ")



        # We'll just send it back (Echo Server)
        # But for a VPN, it should ideally be sent to its TUN interface.
        # Мы просто отправим его обратно (Echo Server)
        # Но для VPN по-хорошему его надо отправить в свой TUN интерфейс
        token = chacha.encrypt(nonce, decrypted_data, None)
        sock.sendto(nonce + token, addr)
        print("Echo reply sent.")

    except Exception as e:
        print(f"Decryption error: {e}")