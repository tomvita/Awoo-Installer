#pragma once
#include "../hos/hos.hpp"

namespace Crypto {

    Result Initialize();

    const u8 *GetRsa2048Modulus(NcaFixedKeyGeneration keygen);

    bool VerifyRsa2048Pss(const void *data, size_t size, const u8 *signature, const u8 *modulus);

    class AesXtr {
      private:
        Aes128XtsContext ctx;

      public:
        AesXtr(const u8 *key, bool is_encryptor);

        void Encrypt(void *dst, const void *src, size_t l, size_t sector, size_t sector_size);
        void Decrypt(void *dst, const void *src, size_t l, size_t sector, size_t sector_size);
    };

    AesXtr GetHeaderDecryptor();
    AesXtr GetHeaderEncryptor();
}
