#include <assert.h>
#include "platform/controller_measurement_mode2_adjustment.h"

static u8 call_count;
static u8 selected_mode;

void ClassifyAndAdjustControllerMeasurementForMode(u8 required_mode)
{
    ++call_count;
    selected_mode = required_mode;
}

int main(void)
{
    ClassifyAndAdjustControllerMeasurementForMode2();
    assert(call_count == 1u);
    assert(selected_mode == 2u);
    return 0;
}
