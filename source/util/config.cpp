#include "config.hpp"

#include "../hos/hos.hpp"
#include "logger.hpp"

namespace cfg {

    namespace {

        bool cachedKeyGen = false;
        u32 keyGen = 0;

        /**
         * @brief Reads key generation from BCT.
         * 
         * @note https://switchbrew.org/wiki/BCT#Structure
         */
        void CacheKeyGenerationFs() {
            if (cachedKeyGen)
                return;

            /* Open boot storage. */
            FsStorage boot0;
            R_RETURN(fsOpenBisStorage(&boot0, FsBisPartitionId_BootPartition1Root));
            IStorage storage(std::move(boot0));

            /* Read version from BCT. */
            u32 temp_keyGen;
            R_RETURN(storage.Read(0x2330, &temp_keyGen, sizeof(u32)));

            /* Save key gen and set cache flag. */
            keyGen = temp_keyGen;
            cachedKeyGen = true;
        }

        /**
         * @brief Cycle through key generations and test with SPL if that version is supported.
         * 
         * @note Inacurate due to Atmosphère providing key generation 11 on key generation 10.
         */
        /*void CacheKeyGenerationSpl() {
            u8 dummySource[0x10];
            u8 dummyKek[0x10];
            while (true) {
                Result rc = splCryptoGenerateAesKek(dummySource, keyGen, 0, dummyKek);
                LOG("gen: %d, rc = 0x%x", keyGen, rc);
                if (R_FAILED(rc)) {
                    keyGen--;
                    break;
                }
                keyGen++;
            }
            cachedKeyGen = true;
        }*/

    }

    bool SuppostsKeyGeneration(u32 keyGeneration) {
        CacheKeyGenerationFs();

        return keyGeneration <= keyGen;
    }

    u32 GetHighestSupportedKeyGen() {
        CacheKeyGenerationFs();

        return keyGen;
    }

}
