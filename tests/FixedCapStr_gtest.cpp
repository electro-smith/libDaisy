#include <gtest/gtest.h>
#include "util/FixedCapStr.h"

using namespace daisy;

#if __cpp_constexpr >= 201603
#define HAS_CONSTEXPR_LAMBDA
#endif

TEST(util_FixedCapStr, a_basics)
{
    // basics

    // size
    static constexpr auto constexprSizeTest = FixedCapStr<8>("abcd").Size();
    EXPECT_EQ(constexprSizeTest, 4u);

    // truncation of size in ctor
    static constexpr auto constexprTruncationTest
        = FixedCapStr<8>("abcdefghijklmnopqrstuvw").Size();
    EXPECT_EQ(constexprTruncationTest, 8u);

    // maxSize
    static constexpr auto constexprMaxSizeTest
        = FixedCapStr<8>("abcde").Capacity();
    EXPECT_EQ(constexprMaxSizeTest, 8u);

    // empty
    static constexpr auto constexprEmptyTest = FixedCapStr<8>("").Empty();
    EXPECT_TRUE(constexprEmptyTest);
    static constexpr auto constexprEmptyTest2 = FixedCapStr<8>("a").Empty();
    EXPECT_FALSE(constexprEmptyTest2);
}

TEST(util_FixedCapStr, b_equals)
{
    // compare with different capacity
    EXPECT_TRUE(FixedCapStr<8>("abcd") == FixedCapStr<16>("abcd"));
    EXPECT_FALSE(FixedCapStr<8>("abcd") == FixedCapStr<16>("wxyz"));
    EXPECT_TRUE(FixedCapStr<13>("abcd") != FixedCapStr<6>("efgh"));
    EXPECT_FALSE(FixedCapStr<13>("abcd") != FixedCapStr<6>("abcd"));
    EXPECT_TRUE(FixedCapStr<8>("abcd") == "abcd");
    EXPECT_FALSE(FixedCapStr<8>("abcd") == "wxyz");
    EXPECT_TRUE(FixedCapStr<13>("abcd") != "efgh");
    EXPECT_FALSE(FixedCapStr<8>("abcd") != "abcd");

    // case sensitive
    EXPECT_FALSE(FixedCapStr<8>("abcd") == "Abcd");
    EXPECT_TRUE(FixedCapStr<8>("abcd") != "Abcd");

    // is constexpr ?
    static constexpr auto constexprTestEq = FixedCapStr<8>("abcd") == "abcd";
    EXPECT_TRUE(constexprTestEq);
    static constexpr auto constexprTestNeq = FixedCapStr<8>("abcd") != "defg";
    EXPECT_TRUE(constexprTestNeq);
}

TEST(util_FixedCapStr, c_startsWith)
{
    // startsWith with different maxLength
    EXPECT_TRUE(FixedCapStr<9>("abcdefg").StartsWith(FixedCapStr<23>("abcd")));
    EXPECT_TRUE(FixedCapStr<9>("abcdefg").StartsWith("abcd"));
    EXPECT_FALSE(
        FixedCapStr<9>("abcdefg").StartsWith(FixedCapStr<23>("bcdef")));
    EXPECT_FALSE(FixedCapStr<9>("abcdefg").StartsWith("ghij"));
    // case sensitive !
    EXPECT_FALSE(FixedCapStr<9>("abcdefg").StartsWith("aBcD"));

    // is constexpr ?
    static constexpr auto constexprTest
        = FixedCapStr<9>("abcdefg").StartsWith("abcd");
    EXPECT_TRUE(constexprTest);
}

TEST(util_FixedCapStr, d_startsWithIgnoringCase)
{
    // startsWithIgnoringCase with different maxLength
    EXPECT_TRUE(FixedCapStr<9>("aBcDeFg").StartsWithIgnoringCase(
        FixedCapStr<23>("AbCd")));
    EXPECT_TRUE(FixedCapStr<9>("aBcDeFg").StartsWithIgnoringCase("ABcD"));
    EXPECT_FALSE(FixedCapStr<9>("aBcDeFg").StartsWithIgnoringCase(
        FixedCapStr<23>("bCdEf")));
    EXPECT_FALSE(FixedCapStr<9>("aBcDeFg").StartsWithIgnoringCase("ghij"));

    // is constexpr ?
    static constexpr auto constexprTest
        = FixedCapStr<9>("abcdefg").StartsWithIgnoringCase("aBCd");
    EXPECT_TRUE(constexprTest);
}

TEST(util_FixedCapStr, e_endsWith)
{
    // endsWith with different maxLength
    EXPECT_TRUE(FixedCapStr<9>("abcdefg").EndsWith(FixedCapStr<23>("defg")));
    EXPECT_TRUE(FixedCapStr<9>("abcdefg").EndsWith("defg"));
    EXPECT_FALSE(FixedCapStr<9>("abcdefg").EndsWith(FixedCapStr<23>("abcd")));
    EXPECT_FALSE(FixedCapStr<9>("abcdefg").EndsWith("abcd"));
    // case sensitive !
    EXPECT_FALSE(FixedCapStr<9>("abcdefg").EndsWith("DefG"));

    // is constexpr ?
    static constexpr auto constexprTest
        = FixedCapStr<9>("abcdefg").EndsWith("defg");
    EXPECT_TRUE(constexprTest);
}

TEST(util_FixedCapStr, f_endsWithIgnoringCase)
{
    // endsWithIgnoringCase with different maxLength
    EXPECT_TRUE(FixedCapStr<9>("abcdefg").EndsWithIgnoringCase(
        FixedCapStr<23>("DeFg")));
    EXPECT_TRUE(FixedCapStr<9>("abcdefg").EndsWithIgnoringCase("cdEfG"));
    EXPECT_FALSE(FixedCapStr<9>("abcdefg").EndsWithIgnoringCase(
        FixedCapStr<23>("abcd")));
    EXPECT_FALSE(FixedCapStr<9>("abcdefg").EndsWithIgnoringCase("abcd"));

    // is constexpr ?
    static constexpr auto constexprTest
        = FixedCapStr<9>("abcdefg").EndsWithIgnoringCase("DeFg");
    EXPECT_TRUE(constexprTest);
}

TEST(util_FixedCapStr, g_clear)
{
    // clear
    auto str = FixedCapStr<9>("abcdefg");
    EXPECT_FALSE(str.Empty());
    str.Clear();
    EXPECT_TRUE(str.Empty());

#ifdef HAS_CONSTEXPR_LAMBDA
    // clear should also work in a constexpr use case
    constexpr auto getEmptyStringFunc = []() {
        FixedCapStr<9> str("abcdefg");
        str.Clear();
        return str;
    };
    static constexpr auto constexprClearTest = getEmptyStringFunc().Empty();
    EXPECT_TRUE(constexprClearTest);
#endif
}

TEST(util_FixedCapStr, h_reset)
{
    // reset
    auto str = FixedCapStr<6>("abcdefg");
    str.Reset("1234");
    EXPECT_STREQ(str, "1234");

    // respect max length
    str.Reset("1234567890");
    EXPECT_STREQ(str, "123456");

#ifdef HAS_CONSTEXPR_LAMBDA
    // should also work in a constexpr use case
    constexpr auto getStringFunc = []() {
        FixedCapStr<9> str("abcdefg");
        str.Reset("1234");
        return str;
    };
    static constexpr auto constexprResetTest = getStringFunc();
    EXPECT_STREQ(constexprResetTest, "1234");
#endif
}

TEST(util_FixedCapStr, i_appendString)
{
    // append
    auto str = FixedCapStr<9>("abcd");
    str.Append("1234");
    EXPECT_STREQ(str, "abcd1234");

    // respect max length
    str.Append("qwertz");
    EXPECT_STREQ(str, "abcd1234q");

#ifdef HAS_CONSTEXPR_LAMBDA
    // should also work in a constexpr use case
    constexpr auto getStringFunc = []() {
        FixedCapStr<9> str("abcd");
        str.Append("1234");
        return str;
    };
    static constexpr auto constexprAppendTest = getStringFunc();
    EXPECT_STREQ(constexprAppendTest, "abcd1234");
#endif
}

TEST(util_FixedCapStr, j_appendStringN)
{
    // append
    auto str = FixedCapStr<9>("abcd");
    str.Append("1234", 2);
    EXPECT_STREQ(str, "abcd12");

    // respect max length
    str.Append("qwertz", 5);
    EXPECT_STREQ(str, "abcd12qwe");

#ifdef HAS_CONSTEXPR_LAMBDA
    // should also work in a constexpr use case
    constexpr auto getStringFunc = []() {
        FixedCapStr<9> str("abcd");
        str.Append("1234", 2);
        return str;
    };
    static constexpr auto constexprAppendTest = getStringFunc();
    EXPECT_STREQ(constexprAppendTest, "abcd12");
#endif
}

TEST(util_FixedCapStr, k_appendCharacter)
{
    // append
    auto str1 = FixedCapStr<9>("abcd");
    str1.Append('1');
    EXPECT_STREQ(str1, "abcd1");

    // respect max length (don't overwrite terminating zero!)
    auto str2 = FixedCapStr<4>("abcd");
    str2.Append('1');
    EXPECT_STREQ(str2, "abcd");

#ifdef HAS_CONSTEXPR_LAMBDA
    // should also work in a constexpr use case
    constexpr auto getStringFunc = []() {
        FixedCapStr<9> str("abcd");
        str.Append('1');
        return str;
    };
    static constexpr auto constexprAppendTest = getStringFunc();
    EXPECT_STREQ(constexprAppendTest, "abcd1");
#endif
}

TEST(util_FixedCapStr, l_removePrefix)
{
    // remove prefix
    auto str = FixedCapStr<9>("abcd");
    str.RemovePrefix(2);
    EXPECT_STREQ(str, "cd");

#ifdef HAS_CONSTEXPR_LAMBDA
    // should also work in a constexpr use case
    constexpr auto getStringFunc = []() {
        FixedCapStr<9> str("abcd");
        str.RemovePrefix(2);
        return str;
    };
    static constexpr auto constexprAppendTest = getStringFunc();
    EXPECT_STREQ(constexprAppendTest, "cd");
#endif
}

TEST(util_FixedCapStr, m_removeSuffix)
{
    // remove suffix
    auto str = FixedCapStr<9>("abcd");
    str.RemoveSuffix(2);
    EXPECT_STREQ(str, "ab");

#ifdef HAS_CONSTEXPR_LAMBDA
    // should also work in a constexpr use case
    constexpr auto getStringFunc = []() {
        FixedCapStr<9> str("abcd");
        str.RemoveSuffix(2);
        return str;
    };
    static constexpr auto constexprAppendTest = getStringFunc();
    EXPECT_STREQ(constexprAppendTest, "ab");
#endif
}

TEST(util_FixedCapStr, n_swap)
{
    // remove suffix
    auto str1 = FixedCapStr<4>("12");
    auto str2 = FixedCapStr<4>("34");
    str1.Swap(str2);
    EXPECT_STREQ(str1, "34");
    EXPECT_STREQ(str2, "12");

#ifdef HAS_CONSTEXPR_LAMBDA
    // should also work in a constexpr use case
    constexpr auto getStringFunc = []() {
        auto str1 = FixedCapStr<4>("12");
        auto str2 = FixedCapStr<4>("34");
        str1.Swap(str2);
        return str1;
    };
    static constexpr auto constexprAppendTest = getStringFunc() == "34";
    EXPECT_TRUE(constexprAppendTest);
#endif
}

TEST(util_FixedCapStr, o_updateSize)
{
    // update size

    auto str      = FixedCapStr<4>("12");
    str.Data()[2] = '3';
    str.Data()[3] = 0;
    str.UpdateSize();
    EXPECT_EQ(str.Size(), 3u);

    // write beyond string capacity
    str.Data()[3] = '4';
    str.Data()[4] = '5';
    // terminating zero must be restored
    str.UpdateSize();
    EXPECT_EQ(str.Size(), 4u);
    EXPECT_EQ(str.Data()[4], 0);

#ifdef HAS_CONSTEXPR_LAMBDA
    // should also work in a constexpr use case
    constexpr auto getStringFunc = []() {
        auto str      = FixedCapStr<4>("12");
        str.Data()[2] = '3';
        str.Data()[3] = 0;
        str.UpdateSize();
        return str;
    };
    static constexpr auto constexprUpdateLengthTest = getStringFunc().Size();
    EXPECT_EQ(constexprUpdateLengthTest, 3u);
#endif
}

TEST(util_FixedCapStr, p_assignment)
{
    // assignment from different capacity string object, respecting capacity
    FixedCapStr<4> str;
    str = FixedCapStr<6>("abcdef");
    EXPECT_STREQ(str, "abcd");

    // assign from a shorter string
    str = FixedCapStr<2>("xy");
    EXPECT_STREQ(str, "xy");

    // assignment from const char*, respecting capacity
    str = "123456";
    EXPECT_STREQ(str, "1234");

    // assign from a shorter string
    str = "ab";
    EXPECT_STREQ(str, "ab");

#ifdef HAS_CONSTEXPR_LAMBDA
    // should also work in a constexpr use case
    constexpr auto getStringFunc1 = []() {
        FixedCapStr<2> str;
        str = FixedCapStr<4>("abc");
        return str;
    };
    constexpr auto getStringFunc2 = []() {
        FixedCapStr<2> str;
        str = "123";
        return str;
    };
    static constexpr auto constexprAssignTest1 = getStringFunc1();
    static constexpr auto constexprAssignTest2 = getStringFunc2();
    EXPECT_STREQ(constexprAssignTest1, "ab");
    EXPECT_STREQ(constexprAssignTest2, "12");
#endif
}

TEST(util_FixedCapStr, q_resetAt)
{
    // reset at
    FixedCapStr<5> str("abcd");
    str.ResetAt("12", 1);
    EXPECT_STREQ(str, "a12d");

    // reset at, exceeding current length and capacity
    str.ResetAt("uvwxyz", 3);
    EXPECT_STREQ(str, "a12uv");

#ifdef HAS_CONSTEXPR_LAMBDA
    // should also work in a constexpr use case
    constexpr auto getStringFunc = []() {
        FixedCapStr<5> str("abcd");
        str.ResetAt("12", 1);
        return str;
    };
    static constexpr auto constexprResetAtTest = getStringFunc();
    EXPECT_STREQ(constexprResetAtTest, "a12d");
#endif
}

TEST(util_FixedCapStr, r_operatorGtLt)
{
    //  operator<(), operator<=(), operator>(), operator>=()

    // < / <= based on content
    EXPECT_TRUE(FixedCapStr<5>("abc") < "abd");
    EXPECT_FALSE(FixedCapStr<5>("abc") < "abb");
    EXPECT_TRUE(FixedCapStr<5>("abc") <= "abd");
    EXPECT_TRUE(FixedCapStr<5>("abc") <= "abc");

    // < / <= based on length alone
    EXPECT_TRUE(FixedCapStr<5>("abc") < "abcd");
    EXPECT_FALSE(FixedCapStr<5>("abc") < "abc");
    EXPECT_TRUE(FixedCapStr<5>("abc") <= "abcd");
    EXPECT_TRUE(FixedCapStr<5>("abc") <= "abc");

    // > / >= based on content
    EXPECT_TRUE(FixedCapStr<5>("abc") > "abb");
    EXPECT_FALSE(FixedCapStr<5>("abc") > "abd");
    EXPECT_TRUE(FixedCapStr<5>("abc") >= "abb");
    EXPECT_TRUE(FixedCapStr<5>("abc") >= "abc");

    // > / >= based on length alone
    EXPECT_TRUE(FixedCapStr<5>("abcd") > "abc");
    EXPECT_FALSE(FixedCapStr<5>("abc") > "abc");
    EXPECT_TRUE(FixedCapStr<5>("abcd") >= "abc");
    EXPECT_TRUE(FixedCapStr<5>("abc") >= "abc");
}

TEST(util_FixedCapStr, s_reverseSection)
{
    // reverse section
    FixedCapStr<5> str("abcd");
    str.ReverseSection(1, 2);
    EXPECT_STREQ(str, "acbd");

    // reverse section exceeding length
    FixedCapStr<5> str2("abcd");
    str2.ReverseSection(2, 20);
    EXPECT_STREQ(str2, "abdc");

#ifdef HAS_CONSTEXPR_LAMBDA
    // should also work in a constexpr use case
    constexpr auto getStringFunc = []() {
        FixedCapStr<5> str("abcd");
        str.ReverseSection(1, 2);
        return str;
    };
    static constexpr auto constexprReverseSectionTest = getStringFunc();
    EXPECT_STREQ(constexprReverseSectionTest, "acbd");
#endif
}

TEST(util_FixedCapStr, t_appendInt)
{
    // append integer
    {
        FixedCapStr<5> str("ab");
        str.AppendInt(12);
        EXPECT_STREQ(str, "ab12");
        FixedCapStr<5> str2("ab");
        str2.AppendInt(-12);
        EXPECT_STREQ(str2, "ab-12");
    }
    // append with forced sign
    {
        FixedCapStr<5> str("ab");
        str.AppendInt(12, true);
        EXPECT_STREQ(str, "ab+12");
        FixedCapStr<5> str2("ab");
        str2.AppendInt(-12, true);
        EXPECT_STREQ(str2, "ab-12");
    }
    // append zero
    {
        FixedCapStr<5> str("ab");
        str.AppendInt(0);
        EXPECT_STREQ(str, "ab0");
        FixedCapStr<5> str2("ab");
        str2.AppendInt(0, true);
        EXPECT_STREQ(str2, "ab+0");
    }

#ifdef HAS_CONSTEXPR_LAMBDA
    // should also work in a constexpr use case
    constexpr auto getStringFunc = []() {
        FixedCapStr<5> str("ab");
        str.AppendInt(12);
        return str;
    };
    static constexpr auto constexprAppendIntTest = getStringFunc();
    EXPECT_STREQ(constexprAppendIntTest, "ab12");
#endif
}
TEST(util_FixedCapStr, u_appendFloat)
{
    // append float

    // default settings
    {
        FixedCapStr<10> str("ab");
        str.AppendFloat(12.3456789f);
        EXPECT_STREQ(str, "ab12.35"); // rounding!
    }
    // negative default
    {
        FixedCapStr<10> str("ab");
        str.AppendFloat(-12.3456789f);
        EXPECT_STREQ(str, "ab-12.35"); // rounding!
    }
    // with more digits
    {
        FixedCapStr<10> str("ab");
        str.AppendFloat(12.3456789f, 4);
        EXPECT_STREQ(str, "ab12.3457"); // rounding!
    }
    // not omitting trailing zeros by default
    {
        FixedCapStr<10> str("ab");
        str.AppendFloat(12.3000f);
        EXPECT_STREQ(str, "ab12.30");
    }
    // omitting trailing zeros manually
    {
        FixedCapStr<10> str("ab");
        str.AppendFloat(12.3000f, 2, true);
        EXPECT_STREQ(str, "ab12.3");
    }
    // force sign
    {
        FixedCapStr<10> str("ab");
        str.AppendFloat(12.3456789f, 2, false, true);
        EXPECT_STREQ(str, "ab+12.35"); // rounding!
    }
    // at least one zero before decimal point
    {
        FixedCapStr<10> str("ab");
        str.AppendFloat(0.1f);
        EXPECT_STREQ(str, "ab0.10");
    }
    // no decimal point when numDecimals == 0
    {
        FixedCapStr<10> str("ab");
        str.AppendFloat(12.3f, 0);
        EXPECT_STREQ(str, "ab12");
    }

#ifdef HAS_CONSTEXPR_LAMBDA
    // should also work in a constexpr use case
    constexpr auto getStringFunc = []() {
        FixedCapStr<10> str("ab");
        str.AppendFloat(12.3456789f);
        return str;
    };
    static constexpr auto constexprAppendFloatTest = getStringFunc();
    EXPECT_STREQ(constexprAppendFloatTest, "ab12.35");
#endif
}