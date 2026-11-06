#ifndef PLEXTOR_MCS251_H
#define PLEXTOR_MCS251_H

/* Keil C251 types, with host-test equivalents of the same width. */
#if defined(PLEXTOR_HOST_TEST)
#include <stdint.h>
#define PLEXTOR_NEAR_FUNCTION
#define PLEXTOR_REENTRANT
typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
#else
/* Relocatable reconstruction functions use 24-bit calls. A native near-only
 * boundary must spell `near` locally and be banked with all of its callers. */
#define PLEXTOR_NEAR_FUNCTION
#define PLEXTOR_REENTRANT reentrant
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned int u16;
typedef signed int s16;
typedef unsigned long u32;
typedef signed long s32;
#endif
typedef char assert_u8_size[(sizeof(u8) == 1) ? 1 : -1];
typedef char assert_u16_size[(sizeof(u16) == 2) ? 1 : -1];
typedef char assert_u32_size[(sizeof(u32) == 4) ? 1 : -1];
typedef u32 fw_addr_t;

/* RAM:0000fc, distinct from SFR:fc. Main/monitor interrupt ownership. */
#if defined(PLEXTOR_HOST_TEST)
#define MAIN_VECTOR_SELECT FirmwareReadNear8(FW_NEAR_UNKNOWN_00FC)
#else
#define MAIN_VECTOR_SELECT (*(volatile u8 near *)0x00fc)
#endif

#endif
