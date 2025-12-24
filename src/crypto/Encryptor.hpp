#pragma once
#include <vector>
#include <cstdint>
#include <random>
#include <openssl/evp.h>

class Encryptor {
public:
    explicit Encryptor(const std::vector<uint8_t>& key);
    ~Encryptor();

    // Шифрование (AES-256-GCM)
    size_t encrypt(const uint8_t* plaintext, size_t plaintext_len, uint8_t* ciphertext, size_t max_out);

    // Расшифровка
    size_t decrypt(const uint8_t* ciphertext, size_t ciphertext_len, uint8_t* plaintext, size_t max_out);

private:
    EVP_CIPHER_CTX *en_ctx;
    EVP_CIPHER_CTX *de_ctx;
    std::vector<uint8_t> _key;
    const size_t IV_LEN = 12; // Стандарт для GCM
    const size_t TAG_LEN = 16;
};