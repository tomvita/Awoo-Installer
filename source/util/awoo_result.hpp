#pragma once
#include "../hos/hos.hpp"

#define MAKE_OWN_RC(name, desc) \
    MAKE_RC(name, 401, desc);

/* Protocol. */
MAKE_OWN_RC(LogCreateFailed, 1)
MAKE_OWN_RC(LogOpenFailed, 2)
MAKE_OWN_RC(LogStatFailed, 3)
MAKE_OWN_RC(NxlinkInitFailed, 4)
MAKE_OWN_RC(LogNotInitialized, 5)
MAKE_OWN_RC(EndOfFile, 20)
MAKE_OWN_RC(PartitionNotFound, 21)

/* FS */
MAKE_RC(GamecardNotInserted, 2, 2520)
MAKE_RC(InvalidPFS0Magic, 2, 4644)
MAKE_RC(InvalidHFS0Magic, 2, 4645)
MAKE_RC(InvalidNCAMagic, 2, 4517)
MAKE_RC(InvalidNCAHeaderFixedKeySignature, 2, 4518)
