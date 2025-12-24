#pragma once
#include <vector>
#include <cstdint>
#include <random>
#include <openssl/evp.h>

class Encryptor {
public:
    Encryptor(const std::vector<uint8_t>& key);
    ~Encryptor();

    // Шифрует и добавляет случайный шум (Padding)
    size_t encrypt(const uint8_t* plaintext, size_t plaintext_len, uint8_t* ciphertext, size_t max_out);

    // Расшифровывает и отбрасывает шум
    size_t decrypt(const uint8_t* ciphertext, size_t ciphertext_len, uint8_t* plaintext, size_t max_out);

private:
    std::vector<uint8_t> _key;
    std::vector<uint8_t> _nonce;
    std::mt19937 _rng; // Для генерации шума
};