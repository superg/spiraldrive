#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/get_configuration_features.h"

static u8 memory[0x1000000ul];
static u8 serial_payload_length;
static unsigned serial_calls;
static unsigned capacity_calls;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory));
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory));
    memory[address] = value;
}

u8 AppendDriveSerialNumberFeaturePayload(fw_addr_t destination)
{
    u8 index;

    ++serial_calls;
    for(index = 0u; index < serial_payload_length; ++index)
        FirmwareWrite8(destination + index, (u8)(0xa0u + index));
    return serial_payload_length;
}

u32 ComputeAndPublishMediaCapacityGeometry(void)
{
    ++capacity_calls;
    memory[0x000c9aul] = 0x11u;
    memory[0x000c9bul] = 0x22u;
    memory[0x000c9cul] = 0x33u;
    memory[0x000c9dul] = 0x44u;
    return 0x11223344ul;
}

static void StoreTemplate(fw_addr_t address, const u8 *bytes, u8 length)
{
    u8 index;

    for(index = 0u; index < length; ++index)
        memory[address + index] = bytes[index];
}

static void LoadFeatureTemplates(void)
{
    static const u8 feature_0108[] = { 0x01u, 0x08u, 0x03u, 0x08u };
    static const u8 feature_0000[] = { 0x00u, 0x00u, 0x03u, 0x0cu, 0x00u, 0x08u, 0x00u, 0x00u, 0x00u, 0x09u, 0x00u, 0x00u, 0x00u, 0x0au, 0x00u, 0x00u };
    static const u8 feature_0001[] = { 0x00u, 0x01u, 0x03u, 0x04u, 0x00u, 0x00u, 0x00u, 0x02u };
    static const u8 feature_0002[] = { 0x00u, 0x02u, 0x03u, 0x04u, 0x00u, 0x00u, 0x00u, 0x00u };
    static const u8 feature_0003[] = { 0x00u, 0x03u, 0x03u, 0x04u, 0x29u, 0x00u, 0x00u, 0x00u };
    static const u8 feature_0010[] = { 0x00u, 0x10u, 0x00u, 0x08u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x01u, 0x00u };
    static const u8 feature_001d[] = { 0x00u, 0x1du, 0x03u, 0x00u };
    static const u8 feature_001e[] = { 0x00u, 0x1eu, 0x04u, 0x04u, 0x03u, 0x00u, 0x00u, 0x00u };
    static const u8 feature_0020[] = { 0x00u, 0x20u, 0x04u, 0x0cu, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x01u, 0x00u, 0x00u };
    static const u8 feature_0021[] = { 0x00u, 0x21u, 0x04u, 0x08u, 0x3fu, 0x0fu, 0x00u, 0x01u, 0x07u, 0x00u, 0x00u, 0x00u };
    static const u8 feature_0023[] = { 0x00u, 0x23u, 0x00u, 0x00u };
    static const u8 feature_0024[] = { 0x00u, 0x24u, 0x04u, 0x04u, 0x00u, 0x00u, 0x00u, 0x00u };
    static const u8 feature_0026[] = { 0x00u, 0x26u, 0x00u, 0x00u };
    static const u8 feature_0028[] = { 0x00u, 0x28u, 0x08u, 0x04u, 0x01u, 0x00u, 0x00u, 0x00u };
    static const u8 feature_002d[] = { 0x00u, 0x2du, 0x08u, 0x04u, 0x17u, 0x00u, 0x3fu, 0x0fu };
    static const u8 feature_002e[] = { 0x00u, 0x2eu, 0x04u, 0x04u, 0x3fu, 0x00u, 0x80u, 0x00u };
    static const u8 feature_0100[] = { 0x01u, 0x00u, 0x03u, 0x00u };
    static const u8 feature_0101[] = { 0x01u, 0x01u, 0x00u, 0x04u, 0x00u, 0x00u, 0x00u, 0x00u };
    static const u8 feature_0103[] = { 0x01u, 0x03u, 0x00u, 0x04u, 0x07u, 0x00u, 0x01u, 0x00u };
    static const u8 feature_0104[] = { 0x01u, 0x04u, 0x03u, 0x00u };
    static const u8 feature_0105[] = { 0x01u, 0x05u, 0x03u, 0x00u };
    static const u8 feature_0107[] = { 0x01u, 0x07u, 0x0cu, 0x04u, 0x0cu, 0x00u, 0x00u, 0x00u };

    StoreTemplate(0xff90feul, feature_0108, sizeof(feature_0108));
    StoreTemplate(0xff9102ul, feature_0000, sizeof(feature_0000));
    StoreTemplate(0xff9112ul, feature_0001, sizeof(feature_0001));
    StoreTemplate(0xff911aul, feature_0002, sizeof(feature_0002));
    StoreTemplate(0xff9122ul, feature_0003, sizeof(feature_0003));
    StoreTemplate(0xff912aul, feature_0010, sizeof(feature_0010));
    StoreTemplate(0xff9136ul, feature_001d, sizeof(feature_001d));
    StoreTemplate(0xff913aul, feature_001e, sizeof(feature_001e));
    StoreTemplate(0xff9142ul, feature_0020, sizeof(feature_0020));
    StoreTemplate(0xff9152ul, feature_0021, sizeof(feature_0021));
    StoreTemplate(0xff915eul, feature_0023, sizeof(feature_0023));
    StoreTemplate(0xff9162ul, feature_0024, sizeof(feature_0024));
    StoreTemplate(0xff916aul, feature_0026, sizeof(feature_0026));
    StoreTemplate(0xff916eul, feature_0028, sizeof(feature_0028));
    StoreTemplate(0xff9176ul, feature_002d, sizeof(feature_002d));
    StoreTemplate(0xff917eul, feature_002e, sizeof(feature_002e));
    StoreTemplate(0xff9186ul, feature_0100, sizeof(feature_0100));
    StoreTemplate(0xff918aul, feature_0101, sizeof(feature_0101));
    StoreTemplate(0xff9192ul, feature_0103, sizeof(feature_0103));
    StoreTemplate(0xff919aul, feature_0104, sizeof(feature_0104));
    StoreTemplate(0xff919eul, feature_0105, sizeof(feature_0105));
    StoreTemplate(0xff91a2ul, feature_0107, sizeof(feature_0107));
}

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    serial_payload_length = 0u;
    serial_calls = 0u;
    capacity_calls = 0u;
    LoadFeatureTemplates();
}

static u16 ResponseFeatureAt(u16 offset)
{
    return (u16)(((u16)memory[0x000b2aul + offset] << 8) | memory[0x000b2bul + offset]);
}

static void CheckExactFeatureAndMissingFeature(void)
{
    u16 length;

    Reset();
    memory[0x000b31ul] = 10u;
    length = BuildGetConfigurationFeatureResponse(2u, 0x0010u, 8u);
    assert(length == 20u && ResponseFeatureAt(8u) == 0x0010u);
    assert((memory[0x000b34ul] & 1u) != 0u);
    assert(memory[0x000b38ul] == 8u && memory[0x000b3bul] == 1u);

    Reset();
    length = BuildGetConfigurationFeatureResponse(2u, 0x0011u, 8u);
    assert(length == 8u);
}

static void CheckGapSelection(void)
{
    u16 length;

    Reset();
    length = BuildGetConfigurationFeatureResponse(0u, 0x0011u, 8u);
    assert(ResponseFeatureAt(8u) == 0x001du);
    assert(length > 8u);

    Reset();
    length = BuildGetConfigurationFeatureResponse(0u, 0x0030u, 8u);
    assert(ResponseFeatureAt(8u) == 0x0100u);
    assert(length > 8u);

    Reset();
    assert(BuildGetConfigurationFeatureResponse(2u, 0x0030u, 8u) == 8u);
}

static void CheckCapacityFeature(void)
{
    u16 length;

    Reset();
    memory[0x000b31ul] = 10u;
    memory[0x000c9eul] = 1u;
    memory[0x00fb80ul] = 2u;
    memory[0x00fb88ul] = 3u;
    length = BuildGetConfigurationFeatureResponse(2u, 0x0020u, 8u);
    assert(length == 24u && capacity_calls == 1u);
    assert(ResponseFeatureAt(8u) == 0x0020u);
    assert((memory[0x000b34ul] & 1u) != 0u);
    assert(memory[0x000b36ul] == 0x11u && memory[0x000b39ul] == 0x44u);
    assert(memory[0x000b3cul] == 8u && memory[0x000b3ful] == 1u);
}

static void CheckCurrentOnlyFiltering(void)
{
    u16 length;

    Reset();
    length = BuildGetConfigurationFeatureResponse(1u, 0x0010u, 8u);
    assert(length == 24u);
    assert(ResponseFeatureAt(8u) == 0x001du);
    assert(ResponseFeatureAt(12u) == 0x0100u);
    assert(ResponseFeatureAt(16u) == 0x0104u);
    assert(ResponseFeatureAt(20u) == 0x0105u);
}

static void CheckFeature0024PresenceGate(void)
{
    Reset();
    assert(BuildGetConfigurationFeatureResponse(2u, 0x0024u, 8u) == 8u);

    Reset();
    memory[0x000b31ul] = 10u;
    memory[0x000c9eul] = 1u;
    memory[0x000c88ul] = 0x80u;
    assert(BuildGetConfigurationFeatureResponse(2u, 0x0024u, 8u) == 8u);

    Reset();
    memory[0x000b31ul] = 10u;
    memory[0x000c9eul] = 1u;
    memory[0x00fb80ul] = 2u;
    assert(BuildGetConfigurationFeatureResponse(2u, 0x0024u, 8u) == 16u);
    assert(ResponseFeatureAt(8u) == 0x0024u);
}

static void CheckProfileAndTrackCurrentBits(void)
{
    Reset();
    memory[0x000b31ul] = 10u;
    memory[0x2bb045ul] = 0x50u;
    memory[0x00fb80ul] = 2u;
    assert(BuildGetConfigurationFeatureResponse(2u, 0x0026u, 8u) == 12u);
    assert((memory[0x000b34ul] & 1u) != 0u);

    Reset();
    memory[0x000b31ul] = 9u;
    memory[0x001132ul] = 0x10u;
    assert(BuildGetConfigurationFeatureResponse(2u, 0x002eu, 8u) == 16u);
    assert((memory[0x000b34ul] & 1u) != 0u);

    Reset();
    memory[0x000b31ul] = 9u;
    memory[0x0012cbul] = 11u;
    memory[0x2b800cul] = 0u;
    assert(BuildGetConfigurationFeatureResponse(2u, 0x0103u, 8u) == 16u);
    assert((memory[0x000b34ul] & 1u) != 0u);
}

static void CheckSerialNumberFeature(void)
{
    Reset();
    assert(BuildGetConfigurationFeatureResponse(2u, 0x0108u, 8u) == 8u);
    assert(serial_calls == 1u);

    Reset();
    serial_payload_length = 8u;
    assert(BuildGetConfigurationFeatureResponse(2u, 0x0108u, 8u) == 20u);
    assert(ResponseFeatureAt(8u) == 0x0108u);
    assert(memory[0x000b36ul] == 0xa0u && memory[0x000b3dul] == 0xa7u);
}

int main(void)
{
    CheckExactFeatureAndMissingFeature();
    CheckGapSelection();
    CheckCapacityFeature();
    CheckCurrentOnlyFiltering();
    CheckFeature0024PresenceGate();
    CheckProfileAndTrackCurrentBits();
    CheckSerialNumberFeature();
    puts("get_configuration_features_tests: ok");
    return 0;
}
