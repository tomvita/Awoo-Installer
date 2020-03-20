#include "crypto.hpp"
#include <mbedtls/bignum.h>
#include <cstring>
#include "logger.hpp"
#include "awoo_result.hpp"
#include "pkeys.hpp"

namespace Crypto {

    namespace {

        const u8 *GetRsa2048Modulus(NcaFixedKeyGeneration keygen) {
            switch (keygen) {
                case NcaFixedKeyGeneration::Version100:
                    return ncaHeaderFixedKeyModuli[0];
                case NcaFixedKeyGeneration::Version910:
                    return ncaHeaderFixedKeyModuli[1];
                default:
                    return nullptr;
            }
        }

        constexpr inline const size_t Rsa2048Bytes = 0x100;
        constexpr inline const size_t Rsa2048Bits = Rsa2048Bytes * 8;

        void calculateMGF1andXOR(unsigned char* data, size_t data_size, const void* source, size_t source_size) {
            unsigned char h_buf[Rsa2048Bytes] = {0};
            std::memcpy(h_buf, source, source_size);

            unsigned char mgf1_buf[0x20];
            size_t ofs = 0;
            unsigned int seed = 0;
            while (ofs < data_size) {
                for (unsigned int i = 0; i < sizeof(seed); i++) {
                    h_buf[source_size + 3 - i] = (seed >> (8 * i)) & 0xFF;
                }
                sha256CalculateHash(mgf1_buf, h_buf, source_size + 4);
                for (unsigned int i = ofs; i < data_size && i < ofs + 0x20; i++) {
                    data[i] ^= mgf1_buf[i - ofs];
                }
                seed++;
                ofs += 0x20;
            }
        }

        bool VerifyRsa2048Pss(const void *data, size_t size, const u8 *signature, const u8 *modulus) {
            /* TODO: drop mbedtls. */
            mbedtls_mpi signature_mpi;
            mbedtls_mpi modulus_mpi;
            mbedtls_mpi e_mpi;
            mbedtls_mpi message_mpi;

            mbedtls_mpi_init(&signature_mpi);
            mbedtls_mpi_init(&modulus_mpi);
            mbedtls_mpi_init(&e_mpi);
            mbedtls_mpi_init(&message_mpi);
            mbedtls_mpi_lset(&message_mpi, Rsa2048Bits);

            unsigned char m_buf[Rsa2048Bytes];
            unsigned char h_buf[0x24];
            const unsigned char E[3] = {1, 0, 1};

            mbedtls_mpi_read_binary(&e_mpi, E, 3);
            mbedtls_mpi_read_binary(&signature_mpi, signature, Rsa2048Bytes);
            mbedtls_mpi_read_binary(&modulus_mpi, modulus, Rsa2048Bytes);
            mbedtls_mpi_exp_mod(&message_mpi, &signature_mpi, &e_mpi, &modulus_mpi, NULL);

            if (mbedtls_mpi_write_binary(&message_mpi, m_buf, Rsa2048Bytes) != 0) {
                LOG("Failed to export exponentiated RSA message!");
                return false;
            }

            mbedtls_mpi_free(&signature_mpi);
            mbedtls_mpi_free(&modulus_mpi);
            mbedtls_mpi_free(&e_mpi);
            mbedtls_mpi_free(&message_mpi);

            /* There's no automated PSS verification as far as I can tell. */
            if (m_buf[Rsa2048Bytes-1] != 0xBC) {
                return false;
            }

            std::memset(h_buf, 0, 0x24);
            std::memcpy(h_buf, m_buf + Rsa2048Bytes - 0x20 - 0x1, 0x20);

            /* Decrypt maskedDB. */
            calculateMGF1andXOR(m_buf, Rsa2048Bytes - 0x20 - 1, h_buf, 0x20);

            m_buf[0] &= 0x7F; /* Constant lmask for rsa-2048-pss. */

            /* Validate DB. */
            for (unsigned int i = 0; i < Rsa2048Bytes - 0x20 - 0x20 - 1 - 1; i++) {
                if (m_buf[i] != 0) {
                    return false;
                }
            }
            if (m_buf[Rsa2048Bytes - 0x20 - 0x20 - 1 - 1] != 1) {
                return false;
            }

            /* Check hash correctness. */
            unsigned char validate_buf[8 + 0x20 + 0x20];
            unsigned char validate_hash[0x20];
            std::memset(validate_buf, 0, 0x48);

            sha256CalculateHash(&validate_buf[8], data, size);
            std::memcpy(&validate_buf[0x28], &m_buf[Rsa2048Bytes - 0x20 - 0x20 - 1], 0x20);
            sha256CalculateHash(validate_hash, validate_buf, 0x48);

            return std::memcmp(h_buf, validate_hash, 0x20) == 0;
        }

        u8 ncaHeaderKey[0x20] = {};

    }

    Result Initialize() {
        u8 kek[0x10] = {};

        R_TRY(splCryptoGenerateAesKek(ncaHeaderKekSource, 0, 0, kek));
        R_TRY(splCryptoGenerateAesKey(kek, ncaHeaderKeySource, ncaHeaderKey));
        R_TRY(splCryptoGenerateAesKey(kek, ncaHeaderKeySource + 0x10, ncaHeaderKey + 0x10));

        return ResultSuccess();
    }

    Result VerifyNcaHeader(const NcaHeader *header) {
        /* Verify NCA header magic. */
        R_UNLESS(header->magic == MAGIC_NCA3, ResultInvalidNCAMagic());

        /* Get required modulus. */
        const u8 *modulus = Crypto::GetRsa2048Modulus(header->fixedKeyGeneration);
        R_UNLESS(modulus, ResultUnknownFixedKeySignature());

        /* Verify fixed key signature. */
        R_UNLESS(Crypto::VerifyRsa2048Pss(&header->magic, 0x200, header->fixedKeySignature, modulus), ResultInvalidNCAHeaderFixedKeySignature());

        /* Header is valid. */
        return ResultSuccess();
    }

    AesXtr::AesXtr(const u8 *key, bool is_encryptor) {
        aes128XtsContextCreate(&ctx, key, key + 0x10, is_encryptor);
    }

    void AesXtr::Encrypt(void *destination, const void *source, size_t l, size_t sector, size_t sector_size) {
        u8 *dst = (u8 *) destination;
        const u8 *src = (const u8 *)source;
        for (size_t i = 0; i < l; i += sector_size) {
            /* Encrypt block. */
            aes128XtsContextResetSector(&ctx, sector++, true);
            aes128XtsEncrypt(&ctx, dst, src, sector_size);
            /* Step forward. */
            dst = dst + sector_size;
            src = src + sector_size;
        }
    }

    void AesXtr::Decrypt(void *destination, const void *source, size_t l, size_t sector, size_t sector_size) {
        u8 *dst = (u8 *) destination;
        const u8 *src = (const u8 *)source;
        for (size_t i = 0; i < l; i += sector_size) {
            /* Decrypt block. */
            aes128XtsContextResetSector(&ctx, sector++, true);
            aes128XtsDecrypt(&ctx, dst, src, sector_size);
            /* Step forward. */
            dst = dst + sector_size;
            src = src + sector_size;
        }
    }

    AesXtr GetHeaderDecryptor() {
        return AesXtr(ncaHeaderKey, false);
    }

    AesXtr GetHeaderEncryptor() {
        return AesXtr(ncaHeaderKey, true);
    }

}