#include <assert.h>
#include <stdint.h>
#include "platform/integer_division.h"

int main(void)
{
    static const s16 signed16_dividends[] = { (s16)0x8000u, -32767, -257, -1, 0, 1, 257, 32766, 32767 };
    int dividend;
    int divisor;
    unsigned dividend_index;
    uint32_t random_state = UINT32_C(0x25110401);
    unsigned random_index;

    for(dividend = -128; dividend <= 127; ++dividend)
    {
        for(divisor = -128; divisor <= 127; ++divisor)
        {
            s8 remainder;
            s8 quotient;
            int expected_quotient;
            int expected_remainder;

            if(divisor == 0)
                continue;
            expected_quotient = dividend / divisor;
            expected_remainder = dividend % divisor;
            quotient = DivideS8WithRemainder((s8)dividend, (s8)divisor, &remainder);
            assert((u8)quotient == (u8)expected_quotient);
            assert(remainder == (s8)expected_remainder);
        }
        assert(DivideS8ByTwoNative((s8)dividend) == (s8)(dividend / 2));
    }
    for(divisor = -128; divisor <= 127; ++divisor)
        if(divisor != 0)
            assert(DivideS8OperatingBudgetNative((u8)(s8)divisor) == (s8)(120 / divisor));
    for(dividend_index = 0; dividend_index != sizeof(signed16_dividends) / sizeof(signed16_dividends[0]); ++dividend_index)
    {
        s32 wide_dividend = signed16_dividends[dividend_index];

        for(divisor = -32768; divisor <= 32767; ++divisor)
        {
            SignedDivision16Result result;

            if(divisor == 0)
                continue;
            result = DivideS16WithRemainder((s16)wide_dividend, (s16)divisor);
            assert((u16)result.quotient == (u16)(wide_dividend / divisor));
            assert(result.remainder == (s16)(wide_dividend % divisor));
            assert(DivideS16Quotient((s16)wide_dividend, (s16)divisor) == result.quotient);
        }
    }
    for(random_index = 0; random_index != 100000u; ++random_index)
    {
        SignedDivision32Result result;
        int32_t dividend32;
        int32_t divisor32;
        int64_t expected_quotient;
        int64_t expected_remainder;

        random_state = random_state * UINT32_C(1664525) + UINT32_C(1013904223);
        dividend32 = (int32_t)random_state;
        random_state = random_state * UINT32_C(1664525) + UINT32_C(1013904223);
        divisor32 = (int32_t)random_state;
        if(divisor32 == 0)
            divisor32 = 1;
        expected_quotient = (int64_t)dividend32 / divisor32;
        expected_remainder = (int64_t)dividend32 % divisor32;
        result = DivideS32WithRemainder((s32)dividend32, (s32)divisor32);
        assert((u32)result.quotient == (u32)expected_quotient);
        assert(result.remainder == (s32)expected_remainder);
        assert(DivideS32Quotient((s32)dividend32, (s32)divisor32) == result.quotient);
    }
    {
        SignedDivision32Result wrapped = DivideS32WithRemainder((s32)0x80000000ul, -1l);
        assert((u32)wrapped.quotient == 0x80000000ul);
        assert(wrapped.remainder == 0);
    }
    random_state = UINT32_C(0x10425101);
    for(random_index = 0; random_index != 100000u; ++random_index)
    {
        UnsignedDivision32Result result;
        uint32_t dividend32;
        uint32_t divisor32;

        random_state = random_state * UINT32_C(1664525) + UINT32_C(1013904223);
        dividend32 = random_state;
        random_state = random_state * UINT32_C(1664525) + UINT32_C(1013904223);
        divisor32 = random_state;
        if(divisor32 == 0u)
            divisor32 = 1u;
        result = DivideU32WithRemainder((u32)dividend32, (u32)divisor32);
        assert(result.quotient == (u32)(dividend32 / divisor32));
        assert(result.remainder == (u32)(dividend32 % divisor32));
        assert(DivideU32Quotient((u32)dividend32, (u32)divisor32) == result.quotient);
    }
    {
        static const u32 edge_values[] = { 0ul, 1ul, 2ul, 0x7ffffffful, 0x80000000ul, 0xfffffffeul, 0xfffffffful };
        unsigned left_index;
        unsigned right_index;

        for(left_index = 0u; left_index != sizeof(edge_values) / sizeof(edge_values[0]); ++left_index)
        {
            for(right_index = 1u; right_index != sizeof(edge_values) / sizeof(edge_values[0]); ++right_index)
            {
                UnsignedDivision32Result result = DivideU32WithRemainder(edge_values[left_index], edge_values[right_index]);

                assert(result.quotient == edge_values[left_index] / edge_values[right_index]);
                assert(result.remainder == edge_values[left_index] % edge_values[right_index]);
            }
        }
    }
    {
        UnsignedDivision32Result short_zero = DivideU32WithRemainder(0x0000abcdul, 0ul);
        UnsignedDivision32Result long_zero = DivideU32WithRemainder(0x1234abcdul, 0ul);

        assert(short_zero.quotient == 0x0000abcdul);
        assert(short_zero.remainder == 0ul);
        assert(long_zero.quotient == 0xabcdfffful);
        assert(long_zero.remainder == 0x0000abcdul);
    }
    return 0;
}
