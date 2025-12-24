#include "Encryptor.hpp"
#include <openssl/rand.h>
#include <cstring>

Encryptor::Encryptor(const std::vector<uint8_t>& key) : _key(key) {
    en_ctx = EVP_CIPHER_CTX_new();
    de_ctx = EVP_CIPHER_CTX_new();
    // Мы не инициализируем ключи здесь, так как для GCM нужен новый IV на каждый пакет
}

Encryptor::~Encryptor() {
    if (en_ctx) EVP_CIPHER_CTX_free(en_ctx);
    if (de_ctx) EVP_CIPHER_CTX_free(de_ctx);
}

size_t Encryptor::encrypt(const uint8_t* plaintext, size_t plaintext_len, uint8_t* ciphertext, size_t max_out) {
    int len;
    size_t ciphertext_len;
    uint8_t iv[IV_LEN];
    RAND_bytes(iv, IV_LEN); // Генерируем случайный IV для защиты от атак повтора

    // Копируем IV в начало выходного буфера
    std::memcpy(ciphertext, iv, IV_LEN);

    EVP_EncryptInit_ex(en_ctx, EVP_aes_256_gcm(), NULL, _key.data(), iv);
    EVP_EncryptUpdate(en_ctx, ciphertext + IV_LEN, &len, plaintext, plaintext_len);
    ciphertext_len = len;

    EVP_EncryptFinal_ex(en_ctx, ciphertext + IV_LEN + len, &len);
    ciphertext_len += len;

    // Добавляем тег аутентичности в конец
    EVP_CIPHER_CTX_ctrl(en_ctx, EVP_CTRL_GCM_GET_TAG, TAG_LEN, ciphertext + IV_LEN + ciphertext_len);

    return IV_LEN + ciphertext_len + TAG_LEN;
}

size_t Encryptor::decrypt(const uint8_t* ciphertext, size_t ciphertext_len, uint8_t* plaintext, size_t max_out) {
    if (ciphertext_len < IV_LEN + TAG_LEN) return 0;

    int len;
    size_t out_len;
    const uint8_t* iv = ciphertext;
    const uint8_t* actual_ciphertext = ciphertext + IV_LEN;
    size_t actual_ciphertext_len = ciphertext_len - IV_LEN - TAG_LEN;
    const uint8_t* tag = ciphertext + ciphertext_len - TAG_LEN;

    EVP_DecryptInit_ex(de_ctx, EVP_aes_256_gcm(), NULL, _key.data(), iv);
    EVP_DecryptUpdate(de_ctx, plaintext, &len, actual_ciphertext, actual_ciphertext_len);
    out_len = len;

    // Проверка тега (защита от модификации пакета)
    EVP_CIPHER_CTX_ctrl(de_ctx, EVP_CTRL_GCM_SET_TAG, TAG_LEN, (void*)tag);

    if (EVP_DecryptFinal_ex(de_ctx, plaintext + len, &len) <= 0) {
        return 0; // Ошибка: пакет был изменен или ключ неверный
    }

    out_len += len;
    return out_len;
}