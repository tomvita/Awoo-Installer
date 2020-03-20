#pragma once
#include "../hos/hos.hpp"

namespace Crypto {

    Result Initialize();

    class AesXtr {
      private:
        Aes128XtsContext ctx;

      public:
        AesXtr(const u8 *key, bool is_encryptor);

        void Encrypt(void *dst, const void *src, size_t l, size_t sector, size_t sector_size);
        void Decrypt(void *dst, const void *src, size_t l, size_t sector, size_t sector_size);
    };

    Result VerifyNcaHeader(const NcaHeader *header);

    AesXtr GetHeaderDecryptor();
    AesXtr GetHeaderEncryptor();
}
