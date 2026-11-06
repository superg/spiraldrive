#include "toc_session_append.h"
#include "platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/toc_session_append_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadAppendTocWord(u16 address)
{
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/toc_session_append_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static void WriteAppendTocWord(u16 address, u16 value)
{
#define PLEXTOR_BINDING_SECTION 3
#include "../platform/bindings/media/toc_session_append_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static void WriteAppendTocDword(u16 address, u32 value)
{
#define PLEXTOR_BINDING_SECTION 4
#include "../platform/bindings/media/toc_session_append_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static fw_addr_t AppendTocDescriptorAddress(u16 cursor)
{
    return FW_HUGE_UNKNOWN_2B8000 + (fw_addr_t)cursor * 11u;
}

static fw_addr_t AppendTocSummaryAddress(u8 point)
{
    return FW_NEAR_UNKNOWN_F2B6 + (fw_addr_t)point * 6u;
}

static void AddAppendTocSummaryFlags(u8 flags)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12BE, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) | flags));
}

static void AddAppendTocPresenceFlags(u8 flags)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14E3, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_14E3) | flags));
}

static fw_addr_t BeginAppendTocDescriptor(void)
{
    fw_addr_t descriptor = AppendTocDescriptorAddress(ReadAppendTocWord(FW_NEAR_UNKNOWN_12DC));

    FirmwareWrite8(descriptor, FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9));
    return descriptor;
}

static void FinishAppendTocDescriptor(void)
{
    WriteAppendTocWord(FW_NEAR_UNKNOWN_12DC, (u16)(ReadAppendTocWord(FW_NEAR_UNKNOWN_12DC) + 1u));
    WriteAppendTocWord(FW_NEAR_UNKNOWN_12CA, (u16)(ReadAppendTocWord(FW_NEAR_UNKNOWN_12CA) + 11u));
}

static void AppendDecodedTocPayload(fw_addr_t source)
{
    fw_addr_t descriptor = BeginAppendTocDescriptor();

    APPEND_TOC_COPY_DECODE(source, descriptor + 1u);
    FinishAppendTocDescriptor();
}

static void AppendBinaryTocPayload(fw_addr_t source)
{
    fw_addr_t descriptor = BeginAppendTocDescriptor();

    APPEND_TOC_COPY_BINARY(source, descriptor + 1u);
    FirmwareWrite8(descriptor + 7u, 0u);
    FinishAppendTocDescriptor();
}

static void AppendTocPointSummary(u8 point)
{
    fw_addr_t descriptor = BeginAppendTocDescriptor();
    fw_addr_t summary = AppendTocSummaryAddress(point);

    FirmwareWrite8(descriptor + 1u, FirmwareRead8(summary + 1u));
    FirmwareWrite8(descriptor + 2u, FirmwareRead8(summary + 2u));
    FirmwareWrite8(descriptor + 3u, FirmwareRead8(summary));
    FirmwareWrite8(descriptor + 8u, FirmwareRead8(summary + 3u));
    FirmwareWrite8(descriptor + 9u, FirmwareRead8(summary + 4u));
    FirmwareWrite8(descriptor + 10u, FirmwareRead8(summary + 5u));
    FirmwareWrite8(descriptor + 6u, 0u);
    FirmwareWrite8(descriptor + 5u, 0u);
    FirmwareWrite8(descriptor + 4u, 0u);
    FinishAppendTocDescriptor();
}

static void ScanAppendTocAdr5Record(fw_addr_t source, u8 point)
{
    u8 decoded_point;

    if(point == 0xb0u)
    {
        APPEND_TOC_COPY_TEN(source, FW_NEAR_UNKNOWN_F550);
        AddAppendTocPresenceFlags(0x01u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_F553) != 0xffu && APPEND_TOC_CONVERT_ADDRESS(FW_NEAR_UNKNOWN_F553) != 0)
            AddAppendTocSummaryFlags(0x04u);
    }
    if(point == 0xb1u)
    {
        APPEND_TOC_COPY_TEN(source, FW_NEAR_UNKNOWN_F564);
        AddAppendTocPresenceFlags(0x02u);
    }
    if(point == 0xb2u)
    {
        APPEND_TOC_COPY_TEN(source, FW_NEAR_UNKNOWN_14E4);
        AddAppendTocPresenceFlags(0x04u);
        APPEND_TOC_COPY_DECODE(FW_NEAR_UNKNOWN_14E4, FW_NEAR_UNKNOWN_F7A6);
    }
    if(point == 0xb3u)
    {
        APPEND_TOC_COPY_TEN(source, FW_NEAR_UNKNOWN_14EE);
        AddAppendTocPresenceFlags(0x08u);
        APPEND_TOC_COPY_DECODE(FW_NEAR_UNKNOWN_14EE, FW_NEAR_UNKNOWN_F7B0);
    }
    if(point == 0xb4u)
    {
        APPEND_TOC_COPY_TEN(source, FW_NEAR_UNKNOWN_14F8);
        AddAppendTocPresenceFlags(0x10u);
        APPEND_TOC_COPY_DECODE(FW_NEAR_UNKNOWN_14F8, FW_NEAR_UNKNOWN_F7BA);
    }
    if(point != 0u && point < 0x41u)
    {
        fw_addr_t destination;

        AddAppendTocPresenceFlags(0x20u);
        decoded_point = APPEND_TOC_BCD(point);
        destination = FW_NEAR_UNKNOWN_F7C4 + (fw_addr_t)decoded_point * 10u;
        APPEND_TOC_COPY_DECODE(source, destination);
        FirmwareWrite8(destination + 2u, APPEND_TOC_BCD(point));
        FirmwareWrite8(destination + 6u, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1C) + 1u));
    }
    if(point == 0xc0u)
    {
        APPEND_TOC_COPY_TEN(source, FW_NEAR_UNKNOWN_F55A);
        AddAppendTocPresenceFlags(0x40u);
    }
    if(point == 0xc1u)
    {
        APPEND_TOC_COPY_TEN(source, FW_NEAR_UNKNOWN_F56E);
        AddAppendTocPresenceFlags(0x80u);
    }
}

static void ScanAppendTocAdr1Record(fw_addr_t source, u8 point)
{
    u8 decoded_point;

    if(point == 0xa0u)
    {
        APPEND_TOC_COPY_TEN(source, FW_NEAR_UNKNOWN_F532);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_12BF, 1u);
        if(FirmwareRead8(source + 8u) == 0x10u)
        {
            fw_addr_t summary = AppendTocSummaryAddress(1u);

            FirmwareWrite8(summary, 1u);
            FirmwareWrite8(summary + 1u, 0x41u);
            FirmwareWrite8(summary + 3u, 0u);
            FirmwareWrite8(summary + 4u, 2u);
            FirmwareWrite8(summary + 5u, 0u);
            AddAppendTocSummaryFlags(0x0au);
        }
    }
    if(point == 0xa1u)
    {
        APPEND_TOC_COPY_TEN(source, FW_NEAR_UNKNOWN_F53C);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_12C0, APPEND_TOC_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_F543)));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_F53A) == 0x10u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_F53C) & 0x40u) != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_12C0, 1u);
    }
    if(point == 0xa2u)
    {
        u8 minute = APPEND_TOC_BCD(FirmwareRead8(source + 7u));
        u8 second = APPEND_TOC_BCD(FirmwareRead8(source + 8u));
        u8 frame = APPEND_TOC_BCD(FirmwareRead8(source + 9u));

        WriteAppendTocDword(FW_NEAR_UNKNOWN_12C1, (u32)APPEND_TOC_CONVERT_TUPLE(minute, second, frame));
        APPEND_TOC_COPY_TEN(source, FW_NEAR_UNKNOWN_F546);
    }

    decoded_point = APPEND_TOC_BCD(point);
    if(decoded_point < 100u)
    {
        fw_addr_t summary = AppendTocSummaryAddress(decoded_point);

        FirmwareWrite8(summary, APPEND_TOC_BCD(point));
        FirmwareWrite8(summary + 1u, FirmwareRead8(source));
        FirmwareWrite8(summary + 2u, 0u);
        FirmwareWrite8(summary + 3u, APPEND_TOC_BCD(FirmwareRead8(source + 7u)));
        FirmwareWrite8(summary + 4u, APPEND_TOC_BCD(FirmwareRead8(source + 8u)));
        FirmwareWrite8(summary + 5u, APPEND_TOC_BCD(FirmwareRead8(source + 9u)));
        if((FirmwareRead8(source) & 0x40u) == 0u)
            AddAppendTocSummaryFlags(0x01u);
        else
            AddAppendTocSummaryFlags(0x02u);
    }
}

static void BuildAppendTocTerminalSummary(void)
{
    u8 point = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0) + 1u);
    fw_addr_t summary = AppendTocSummaryAddress(point);

    FirmwareWrite8(summary, 0xaau);
    FirmwareWrite8(summary + 1u, FirmwareReadNear8(FW_NEAR_UNKNOWN_F546));
    FirmwareWrite8(summary + 2u, 0u);
    FirmwareWrite8(summary + 3u, APPEND_TOC_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_F54D)));
    FirmwareWrite8(summary + 4u, APPEND_TOC_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_F54E)));
    FirmwareWrite8(summary + 5u, APPEND_TOC_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_F54F)));
}

static void AppendMandatoryTocDescriptors(void)
{
    fw_addr_t descriptor;
    u8 track;
    u8 last_track;

    descriptor = BeginAppendTocDescriptor();
    APPEND_TOC_COPY_DECODE(FW_NEAR_UNKNOWN_F532, descriptor + 1u);
    FirmwareWrite8(descriptor + 7u, 0u);
    FirmwareWrite8(descriptor + 9u, FirmwareReadNear8(FW_NEAR_UNKNOWN_F53A));
    FirmwareWrite8(descriptor + 10u, 0u);
    FinishAppendTocDescriptor();

    descriptor = BeginAppendTocDescriptor();
    APPEND_TOC_COPY_DECODE(FW_NEAR_UNKNOWN_F53C, descriptor + 1u);
    FirmwareWrite8(descriptor + 7u, 0u);
    FirmwareWrite8(descriptor + 9u, 0u);
    FirmwareWrite8(descriptor + 10u, 0u);
    FinishAppendTocDescriptor();

    descriptor = BeginAppendTocDescriptor();
    APPEND_TOC_COPY_DECODE(FW_NEAR_UNKNOWN_F546, descriptor + 1u);
    FirmwareWrite8(descriptor + 7u, 0u);
    FinishAppendTocDescriptor();

    track = FirmwareReadNear8(FW_NEAR_UNKNOWN_FC19);
    last_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1A);
    while(track <= last_track)
    {
        u8 summary_point = track;

        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) & 8u) != 0u && FirmwareRead8(AppendTocSummaryAddress(track)) == 0xaau)
            summary_point = 1u;
        AppendTocPointSummary(summary_point);
        track++;
    }
}

static void AppendAdr5OptionalDescriptors(void)
{
    u8 presence = FirmwareReadNear8(FW_NEAR_UNKNOWN_14E3);

    if((presence & 0x01u) != 0u)
        AppendDecodedTocPayload(FW_NEAR_UNKNOWN_F550);
    if((presence & 0x02u) != 0u)
    {
        fw_addr_t descriptor = BeginAppendTocDescriptor();

        FirmwareWrite8(descriptor + 1u, FirmwareReadNear8(FW_NEAR_UNKNOWN_F564));
        FirmwareWrite8(descriptor + 2u, 0u);
        FirmwareWrite8(descriptor + 3u, FirmwareReadNear8(FW_NEAR_UNKNOWN_F566));
        FirmwareWrite8(descriptor + 4u, 0u);
        FirmwareWrite8(descriptor + 5u, 0u);
        FirmwareWrite8(descriptor + 6u, 0u);
        FirmwareWrite8(descriptor + 7u, 0u);
        FirmwareWrite8(descriptor + 8u, APPEND_TOC_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_F56B)));
        FirmwareWrite8(descriptor + 9u, APPEND_TOC_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_F56C)));
        FirmwareWrite8(descriptor + 10u, 0u);
        FinishAppendTocDescriptor();
    }
    if((presence & 0x04u) != 0u)
        AppendDecodedTocPayload(FW_NEAR_UNKNOWN_14E4);
    if((presence & 0x08u) != 0u)
        AppendDecodedTocPayload(FW_NEAR_UNKNOWN_14EE);
    if((presence & 0x10u) != 0u)
        AppendDecodedTocPayload(FW_NEAR_UNKNOWN_14F8);
    if((presence & 0x20u) != 0u)
    {
        u8 record = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1502) + 1u);
        u8 last_record = FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1C);

        while(record <= last_record)
        {
            AppendBinaryTocPayload(FW_NEAR_UNKNOWN_F7C4 + (fw_addr_t)record * 10u);
            record++;
        }
    }
    if((presence & 0x40u) != 0u)
    {
        fw_addr_t descriptor = BeginAppendTocDescriptor();

        FirmwareWrite8(descriptor + 1u, FirmwareReadNear8(FW_NEAR_UNKNOWN_F55A));
        FirmwareWrite8(descriptor + 2u, 0u);
        FirmwareWrite8(descriptor + 3u, FirmwareReadNear8(FW_NEAR_UNKNOWN_F55C));
        FirmwareWrite8(descriptor + 4u, FirmwareReadNear8(FW_NEAR_UNKNOWN_F55D));
        FirmwareWrite8(descriptor + 5u, FirmwareReadNear8(FW_NEAR_UNKNOWN_F55E));
        FirmwareWrite8(descriptor + 6u, FirmwareReadNear8(FW_NEAR_UNKNOWN_F55F));
        FirmwareWrite8(descriptor + 7u, 0u);
        FirmwareWrite8(descriptor + 8u, APPEND_TOC_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_F561)));
        FirmwareWrite8(descriptor + 9u, APPEND_TOC_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_F562)));
        FirmwareWrite8(descriptor + 10u, APPEND_TOC_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_F563)));
        FinishAppendTocDescriptor();
    }
    if((presence & 0x80u) != 0u)
    {
        fw_addr_t descriptor = BeginAppendTocDescriptor();

        FirmwareWrite8(descriptor + 1u, FirmwareReadNear8(FW_NEAR_UNKNOWN_F56E));
        FirmwareWrite8(descriptor + 2u, 0u);
        FirmwareWrite8(descriptor + 3u, FirmwareReadNear8(FW_NEAR_UNKNOWN_F570));
        FirmwareWrite8(descriptor + 4u, FirmwareReadNear8(FW_NEAR_UNKNOWN_F571));
        FirmwareWrite8(descriptor + 5u, FirmwareReadNear8(FW_NEAR_UNKNOWN_F572));
        FirmwareWrite8(descriptor + 6u, FirmwareReadNear8(FW_NEAR_UNKNOWN_F573));
        FirmwareWrite8(descriptor + 7u, 0u);
        FirmwareWrite8(descriptor + 8u, 0u);
        FirmwareWrite8(descriptor + 9u, 0u);
        FirmwareWrite8(descriptor + 10u, 0u);
        FinishAppendTocDescriptor();
    }
}

/* RAM:fabd63 */
void AppendFullTocSessionFromRawReadQRecordTable(void)
{
    u16 record_count;
    u16 record_index;
    u8 session_count;
    u8 state;

    state = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0xbfu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1132, state);
    session_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9);
    if(session_count != 0u)
        AddAppendTocSummaryFlags(0x04u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12C9, (u8)(session_count + 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14E2, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14E3, 0u);
    record_count = ReadAppendTocWord(FW_NEAR_UNKNOWN_800C);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1502, FirmwareReadNear8(FW_NEAR_UNKNOWN_FC1C));

    for(record_index = 0u; record_index < record_count; ++record_index)
    {
        fw_addr_t source = FW_NEAR_UNKNOWN_8012 + (fw_addr_t)record_index * 10u;
        u8 adr = (u8)(FirmwareRead8(source) & 0x0fu);
        u8 point = FirmwareRead8(source + 2u);

        if(adr == 5u)
            ScanAppendTocAdr5Record(source, point);
        else if(adr == 1u)
            ScanAppendTocAdr1Record(source, point);
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) & 3u) == 1u && APPEND_TOC_CONVERT_ADDRESS(FW_NEAR_UNKNOWN_F2BF) > 2249)
        AddAppendTocSummaryFlags(0x10u);

    BuildAppendTocTerminalSummary();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC19, APPEND_TOC_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_F539)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC1A, APPEND_TOC_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_F543)));
    if(ReadAppendTocWord(FW_NEAR_UNKNOWN_12DC) == 0u)
        WriteAppendTocWord(FW_NEAR_UNKNOWN_12DC, 1u);

    AppendMandatoryTocDescriptors();
    AppendAdr5OptionalDescriptors();

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u)
    {
        state = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0xbfu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1132, state);
    }
}
