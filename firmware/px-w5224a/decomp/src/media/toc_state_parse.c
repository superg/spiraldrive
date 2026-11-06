#include "toc_state_parse.h"
#include "platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/toc_state_parse_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadParseTocWord(u16 address)
{
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/toc_state_parse_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static void AddParseTocSummaryFlags(u8 flags)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12BE, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) | flags));
}

static u8 DecodeParseTocThresholdByte(u8 value, u8 exclusive_limit)
{
    if(value < exclusive_limit)
        return PARSE_TOC_BCD(value);
    return value;
}

static void ParseTocAdr2Record(fw_addr_t source)
{
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BB5DB, FirmwareRead8(source));
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BB5DE, PARSE_TOC_BCD(FirmwareRead8(source + 3u)));
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BB5DF, PARSE_TOC_BCD(FirmwareRead8(source + 4u)));
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BB5E0, PARSE_TOC_BCD(FirmwareRead8(source + 5u)));
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BB5E3, FirmwareRead8(source + 8u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_114E, 1u);
}

static void ParseTocAdr1Record(fw_addr_t source, u8 track_index)
{
    fw_addr_t destination = FW_HUGE_UNKNOWN_2BB03B + (fw_addr_t)track_index * 10u;
    u8 control_adr = FirmwareRead8(source);

    FirmwareWrite8(destination, control_adr);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u)
    {
        if((FirmwareRead8(source) & 0x40u) == 0u)
            AddParseTocSummaryFlags(1u);
        else
            AddParseTocSummaryFlags(2u);
    }
    FirmwareWrite8(destination + 2u, PARSE_TOC_BCD(FirmwareRead8(source + 2u)));
    FirmwareWrite8(destination + 3u, DecodeParseTocThresholdByte(FirmwareRead8(source + 3u), 0x9au));
    FirmwareWrite8(destination + 4u, DecodeParseTocThresholdByte(FirmwareRead8(source + 4u), 0x5au));
    FirmwareWrite8(destination + 5u, DecodeParseTocThresholdByte(FirmwareRead8(source + 5u), 0x75u));
    FirmwareWrite8(destination + 7u, PARSE_TOC_BCD(FirmwareRead8(source + 7u)));
    FirmwareWrite8(destination + 8u, PARSE_TOC_BCD(FirmwareRead8(source + 8u)));
    FirmwareWrite8(destination + 9u, PARSE_TOC_BCD(FirmwareRead8(source + 9u)));
}

/* RAM:fab902 */
void PLEXTOR_NEAR_FUNCTION ParseRawReadQRecordTableIntoTocState(void)
{
    u8 track_index = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1149) + 1u);
    u8 adr3_index = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_114A) + 1u);
    u8 adr4_index = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_114B) + 1u);
    u8 adr5_index = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_114C) + 1u);
    u8 adr6_index = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_114D) + 1u);
    u16 record_count = ReadParseTocWord(FW_NEAR_UNKNOWN_800C);
    u16 record_index;

    for(record_index = 0u; record_index < record_count; ++record_index)
    {
        fw_addr_t source = FW_NEAR_UNKNOWN_8012 + (fw_addr_t)record_index * 10u;
        u8 adr = (u8)(FirmwareRead8(source) & 0x0fu);

        if(adr == 2u)
        {
            ParseTocAdr2Record(source);
        }
        else if(adr == 3u)
        {
            PARSE_TOC_COPY_DECODE(source, FW_NEAR_UNKNOWN_2BB423 + (fw_addr_t)adr3_index * 10u);
            adr3_index++;
        }
        else if(adr == 4u)
        {
            PARSE_TOC_COPY_DECODE(source, FW_NEAR_UNKNOWN_2BB4FF + (fw_addr_t)adr4_index * 10u);
            adr4_index++;
        }
        else if(adr == 5u)
        {
            PARSE_TOC_COPY_DECODE(source, FW_NEAR_UNKNOWN_2BB5E5 + (fw_addr_t)adr5_index * 10u);
            adr5_index++;
        }
        else if(adr == 6u)
        {
            PARSE_TOC_COPY_DECODE(source, FW_NEAR_UNKNOWN_2BB77F + (fw_addr_t)adr6_index * 10u);
            adr6_index++;
        }
        else if(adr == 1u)
        {
            ParseTocAdr1Record(source, track_index);
            track_index++;
        }
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1149, (u8)(track_index - 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_114A, (u8)(adr3_index - 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_114B, (u8)(adr4_index - 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_114C, (u8)(adr5_index - 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_114D, (u8)(adr6_index - 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1148, FirmwareReadNear8(FW_NEAR_UNKNOWN_1149));
}
