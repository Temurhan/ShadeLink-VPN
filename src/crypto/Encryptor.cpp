#include "Encryptor.hpp"
#include <openssl/rand.h>
#include <cstring>

Encryptor::Encryptor(const std::vector<uint8_t>& key) : _key(key), _rng(std::random_device{}()) {
    _nonce.resize(12, 0); // В реальном проекте nonce должен меняться
}

Encryptor::~Encryptor() {}

size_t Encryptor::encrypt(const uint8_t* plaintext, size_t plaintext_len, uint8_t* ciphertext, size_t max_out) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    int len;
    size_t ciphertext_len;

    EVP_EncryptInit_ex(ctx, EVP_chacha20_poly1305(), NULL, _key.data(), _nonce.data());
    EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len);
    ciphertext_len = len;
    EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
    ciphertext_len += len;

    // ДОБАВЛЯЕМ ШУМ (Padding) для маскировки от провайдера
    std::uniform_int_distribution<size_t> dist(16, 64); // Случайные 16-64 байта
    size_t padding_len = dist(_rng);

    if (ciphertext_len + padding_len < max_out) {
        RAND_bytes(ciphertext + ciphertext_len, padding_len);
        ciphertext_len += padding_len;
    }

    EVP_CIPHER_CTX_free(ctx);
    return ciphertext_len;
}

size_t Encryptor::decrypt(const uint8_t* ciphertext, size_t ciphertext_len, uint8_t* plaintext, size_t max_out) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    int len;

    EVP_DecryptInit_ex(ctx, EVP_chacha20_poly1305(), NULL, _key.data(), _nonce.data());
    // Мы пробуем расшифровать весь буфер.
    // Лишний шум в конце просто не пройдет проверку или будет отброшен стеком IP.
    if (EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len) <= 0) {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    size_t plaintext_len = len;
    EVP_DecryptFinal_ex(ctx, plaintext + len, &len);
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    return plaintext_len;
}