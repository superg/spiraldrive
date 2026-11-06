#ifndef PLEXTOR_FIRMWARE_ROM_DATA_H
#define PLEXTOR_FIRMWARE_ROM_DATA_H

#include "mcs251.h"

#define PLEXTOR_IMAGE_HEADER_CANONICAL_BASE 0x00f80000ul
#define PLEXTOR_IMAGE_HEADER_SIZE 0x50u
#define PLEXTOR_MAIN_ROM_DATA_CANONICAL_BASE 0x00f82320ul
#define PLEXTOR_MAIN_ROM_DATA_SIZE 0x25e0u
#define PLEXTOR_FIXED_ROM_DATA_CANONICAL_BASE 0x00ff8000ul
#define PLEXTOR_FIXED_ROM_DATA_SIZE 0x1ffeu
#define PLEXTOR_INDEXED_STATE_REFERENCE_CANONICAL_BASE 0x00f894e3ul
#define PLEXTOR_INDEXED_STATE_REFERENCE_SIZE 73u

#if defined(PLEXTOR_HOST_TEST)
#define PLEXTOR_IMAGE_HEADER_ADDRESS(canonical) ((fw_addr_t)(canonical))
#define PLEXTOR_MAIN_ROM_ADDRESS(canonical) ((fw_addr_t)(canonical))
#define PLEXTOR_FIXED_ROM_ADDRESS(canonical) ((fw_addr_t)(canonical))
#define PLEXTOR_INDEXED_STATE_REFERENCE_ADDRESS(canonical) ((fw_addr_t)(canonical))
#else
extern const u8 PlextorMainRomData[PLEXTOR_MAIN_ROM_DATA_SIZE];
extern const u8 PlextorIndexedStateReferencePayloads[PLEXTOR_INDEXED_STATE_REFERENCE_SIZE];
#define PLEXTOR_IMAGE_HEADER_ADDRESS(canonical) ((fw_addr_t)(canonical))
#define PLEXTOR_MAIN_ROM_ADDRESS(canonical) ((fw_addr_t) & PlextorMainRomData[(fw_addr_t)(canonical) - PLEXTOR_MAIN_ROM_DATA_CANONICAL_BASE])
#define PLEXTOR_FIXED_ROM_ADDRESS(canonical) ((fw_addr_t)(canonical))
#define PLEXTOR_INDEXED_STATE_REFERENCE_ADDRESS(canonical) ((fw_addr_t) & PlextorIndexedStateReferencePayloads[(fw_addr_t)(canonical) - PLEXTOR_INDEXED_STATE_REFERENCE_CANONICAL_BASE])
#endif

#endif
