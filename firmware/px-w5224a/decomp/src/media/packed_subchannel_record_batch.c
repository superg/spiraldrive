#include "packed_subchannel_record_batch.h"
#include "../platform/arithmetic.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"
#include "raw_read_descriptor_completion.h"

extern void InitializeControllerBufferCursorRegistersIfNeeded(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0003ForPackedSubchannelBatch(void);
extern void PLEXTOR_NEAR_FUNCTION SetBit0003ForPackedSubchannelBatch(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit001EForPackedSubchannelBatch(void);
extern void PLEXTOR_NEAR_FUNCTION SetBit0009Atomic(void);

static u16 ReadPackedBatchWord(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static void WritePackedBatchWord(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static u32 ReadPackedBatchDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WritePackedBatchDword(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void CopyPackedRecordToOutput(u8 record_index)
{
    fw_addr_t output = ReadPackedBatchDword(FW_NEAR_UNKNOWN_0F56);
    fw_addr_t input = FW_HUGE_UNKNOWN_2BB919 + (u16)record_index * 18u;
    u8 offset;

    for(offset = 0u; offset < 18u; ++offset)
    {
        FirmwareWrite8(output, FirmwareRead8(input + offset));
        output = FirmwareAddressIncrement24(output, 1u);
    }
    WritePackedBatchDword(FW_NEAR_UNKNOWN_0F56, output);
}

static void ResetPackedSubchannelPipeline(void)
{
    u16 slot_count = (u16)ReadPackedBatchDword(FW_NEAR_UNKNOWN_156A);

    WritePackedBatchWord(FW_NEAR_UNKNOWN_1298, 0u);
    WritePackedBatchWord(FW_NEAR_UNKNOWN_1294, 0u);
    WritePackedBatchWord(FW_NEAR_UNKNOWN_1296, 0u);
    WritePackedBatchWord(FW_NEAR_UNKNOWN_1292, 0u);
    WritePackedBatchWord(FW_NEAR_UNKNOWN_129A, (u16)(slot_count - 1u));
    WritePackedBatchWord(FW_NEAR_UNKNOWN_129C, (u16)(slot_count - 6u));
    WritePackedBatchDword(FW_NEAR_UNKNOWN_129E, 0x000088bbul);
    WritePackedBatchDword(FW_NEAR_UNKNOWN_1262, 0u);
    WritePackedBatchDword(FW_NEAR_UNKNOWN_1266, 0u);
    WritePackedBatchDword(FW_NEAR_UNKNOWN_126A, ReadPackedBatchDword(FW_NEAR_UNKNOWN_1562));
    WritePackedBatchDword(FW_NEAR_UNKNOWN_126E, 0u);
}

static void CompletePackedSubchannelGap(void)
{
    fw_addr_t output = (ReadPackedBatchDword(FW_NEAR_UNKNOWN_1556) + FW_HUGE_BUFFER_BANK_BASE - 0x9000ul) & FW_24BIT_ADDRESS_MASK;
    u16 progress = (u16)(ReadPackedBatchWord(FW_NEAR_UNKNOWN_0F50) + 2u);

    WritePackedBatchDword(FW_NEAR_UNKNOWN_0F56, output);
    WritePackedBatchWord(FW_NEAR_UNKNOWN_0F50, progress);
    FirmwareWrite8(output, (u8)(progress >> 8));
    output = FirmwareAddressIncrement24(output, 1u);
    FirmwareWrite8(output, (u8)progress);
    output = FirmwareAddressIncrement24(output, 1u);
    FirmwareWrite8(output, 0u);
    output = FirmwareAddressIncrement24(output, 1u);
    FirmwareWrite8(output, 0u);
    WritePackedBatchDword(FW_NEAR_UNKNOWN_0F56, output);
    progress = (u16)(progress + 2u);
    WritePackedBatchWord(FW_NEAR_UNKNOWN_0F50, progress);
    WritePackedBatchDword(FW_NEAR_UNKNOWN_1598, 9000ul);
    if(progress < 5u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2E, 5u);
        EnqueueRawSectorDescriptorWithTimingGate(0x6400u, 5u);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2E, 6u);
        EnqueueRawSectorDescriptorWithTimingGate(0u, 0u);
    }
}

/* RAM:f93068 */
void PLEXTOR_NEAR_FUNCTION ProcessPackedSubchannelRecordBatch(void)
{
    u8 record_index;
    u8 last_record_accepted = 0u;

    if(ReadPackedBatchDword(FW_NEAR_UNKNOWN_1262) == 0x0000fffful)
    {
        u8 wait_count = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F22) + 1u);

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F22, wait_count);
        if((s8)wait_count > 10)
        {
            InitializeControllerBufferCursorRegistersIfNeeded();
            EnqueueRawSectorDescriptorForParsedQStatus();
            SetBit0009Atomic();
        }
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1300) != 1u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) != 2u)
    {
        ServiceParsedQDescriptorCountdownAndPositionAdvance();
        return;
    }

    PackControllerSubchannelSymbolsTo72Bytes(ReadPackedBatchDword(FW_NEAR_UNKNOWN_1566) + MultiplyU32Low(0x60ul, ReadPackedBatchDword(FW_NEAR_UNKNOWN_1262)));

    for(record_index = 0u; record_index < 4u; ++record_index)
    {
        fw_addr_t record = FW_HUGE_UNKNOWN_2BB919 + (u16)record_index * 18u;
        u8 sequence = FirmwareRead8(record + 2u);
        u8 record_class = (u8)((FirmwareRead8(record + 3u) & 0x70u) >> 4);
        u8 header = (u8)(FirmwareRead8(record) & 0xf0u);

        last_record_accepted = 0u;
        if(header == 0x80u)
        {
            SetBit0003ForPackedSubchannelBatch();
            if(record_class == FirmwareReadNear8(FW_NEAR_UNKNOWN_0F4C) && sequence == FirmwareReadNear8(FW_NEAR_UNKNOWN_0F4D))
            {
                last_record_accepted = 1u;
            }
            else if((s16)ReadPackedBatchWord(FW_NEAR_UNKNOWN_0F50) > 0 && sequence == 0u)
            {
                u8 expected_class;

                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F4D, 0u);
                expected_class = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F4C) + 1u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F4C, expected_class);
                if(record_class == expected_class)
                    last_record_accepted = 1u;
            }
        }
        else if(header != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0F4B) < 2u)
        {
            u32 delay;

            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F4B, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F4B) + 1u));
            delay = ReadPackedBatchDword(FW_NEAR_UNKNOWN_0F52);
            if(delay < 0x8caul)
                WritePackedBatchDword(FW_NEAR_UNKNOWN_0F52, delay + 0x4bul);
            ResetPackedSubchannelPipeline();
            ServiceParsedQDescriptorCountdownAndPositionAdvance();
            return;
        }

        if(last_record_accepted != 0u)
        {
            if(ReadBit001EForPackedSubchannelBatch() != 0u)
            {
                u8 table_index;
                fw_addr_t output = FirmwareAddressIncrement24(ReadPackedBatchDword(FW_NEAR_UNKNOWN_0F56), 18u);

                WritePackedBatchDword(FW_NEAR_UNKNOWN_0F56, output);
                for(table_index = 0u; table_index < 5u; ++table_index)
                {
                    u16 table_value = ReadPackedBatchWord(FW_NEAR_UNKNOWN_F58C + (u16)table_index * 2u);

                    if(table_value == (u16)(ReadPackedBatchWord(FW_NEAR_UNKNOWN_0F50) / 18u) && table_value != 0xffffu)
                    {
                        WritePackedBatchDword(FW_NEAR_UNKNOWN_0F56, (output - 18u) & FW_24BIT_ADDRESS_MASK);
                        CopyPackedRecordToOutput(record_index);
                        WritePackedBatchWord(FW_NEAR_UNKNOWN_F58C + (u16)table_index * 2u, 0xffffu);
                        break;
                    }
                }
            }
            else
            {
                CopyPackedRecordToOutput(record_index);
            }
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F4D, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F4D) + 1u));
            WritePackedBatchWord(FW_NEAR_UNKNOWN_0F50, (u16)(ReadPackedBatchWord(FW_NEAR_UNKNOWN_0F50) + 18u));
            WritePackedBatchWord(FW_NEAR_UNKNOWN_0F4E, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F4B, 0u);
        }
    }

    if(last_record_accepted == 0u)
        WritePackedBatchWord(FW_NEAR_UNKNOWN_0F4E, (u16)(ReadPackedBatchWord(FW_NEAR_UNKNOWN_0F4E) + 1u));

    if(ReadBit0003ForPackedSubchannelBatch() != 0u)
    {
        if(ReadPackedBatchWord(FW_NEAR_UNKNOWN_0F50) == 0u)
        {
            u16 threshold = (u16)((u16)(8u - FirmwareReadNear8(FW_NEAR_UNKNOWN_0F4C)) * 64u);

            if(ReadPackedBatchWord(FW_NEAR_UNKNOWN_0F4E) <= threshold)
                return;
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F4C) < 7u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F4D, 0u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F4C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0F4C) + 1u));
                WritePackedBatchWord(FW_NEAR_UNKNOWN_0F4E, 0u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F4B, 0u);
                return;
            }
        }
        else if(ReadPackedBatchWord(FW_NEAR_UNKNOWN_0F4E) < 0x48u)
        {
            return;
        }
        CompletePackedSubchannelGap();
    }
    else if(ReadPackedBatchWord(FW_NEAR_UNKNOWN_0F4E) > 0x20u)
    {
        fw_addr_t output = ReadPackedBatchDword(FW_NEAR_UNKNOWN_0F56);
        u8 index;

        for(index = 0u; index < 4u; ++index)
        {
            FirmwareWrite8(output, 0u);
            output = FirmwareAddressIncrement24(output, 1u);
        }
        WritePackedBatchDword(FW_NEAR_UNKNOWN_0F56, output);
        WritePackedBatchDword(FW_NEAR_UNKNOWN_1598, 0x96ul);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F2E, 5u);
        EnqueueRawSectorDescriptorWithTimingGate(0x2400u, 5u);
    }
}
