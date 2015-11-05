#include "gtest/gtest.h"

#include "lazy_string.h"

using namespace std_utils;
TEST(lazy_string, size_is_zero_after_default_construction){
    lazy_string str;
    ASSERT_EQ(str.size(), 0);
    ASSERT_EQ(str.capacity(), 0);
}

TEST(lazy_string, move_constructor){
    lazy_string foo = "foo";
    lazy_string bar(std::move(foo));
    ASSERT_TRUE(foo.empty());
    ASSERT_EQ(bar, "foo");
}

TEST(lazy_string, move_assignment){
    lazy_string foo = "foo";
    lazy_string bar = "bar";
    bar = std::move(foo);
    ASSERT_EQ(bar, "foo");
    ASSERT_EQ(foo, "bar");
}

TEST(lazy_string, assign_empty_string){
    lazy_string str = "foobar";
    lazy_string empty;
    str = empty;
    ASSERT_EQ(str.size(), 0);
    ASSERT_EQ(str.capacity(), 0);
}

TEST(lazy_string, addition_operator){
    lazy_string foo = "foo";
    lazy_string bar = "bar";
    ASSERT_EQ((foo+bar), "foobar");
}

TEST(lazy_string, assignment_via_non_const_indexing_doesnt_affect_shared_buffer){
    lazy_string foo = "12345";
    lazy_string bar = foo;
    foo[2] = 'x';
    ASSERT_EQ(foo, "12x45");
    ASSERT_EQ(bar, "12345");
}

TEST(lazy_string, proxy_converts_to_char){
    lazy_string foo = "12345";
    auto chr = foo[2];
    chr = 'x';
    foo += chr;
    ASSERT_EQ(foo, "12x45x");
}

TEST(lazy_string, clear_shared_creates_empty_string){
    lazy_string foo = "12345";
    lazy_string bar = foo;
    foo.clear();
    ASSERT_EQ(foo.capacity(), 0);
    ASSERT_EQ(bar, lazy_string("12345"));
}

TEST(lazy_string, clear_nonshared_adjusts_size){
    lazy_string foo = "12345";
    foo.clear();
    ASSERT_EQ(foo.capacity(), 31);
    ASSERT_TRUE(foo.empty());
}

TEST(lazy_string, swap_with_empty){
    lazy_string empty;
    lazy_string notempty = "foo";
    swap(empty, notempty);
    ASSERT_TRUE(notempty.empty());
    ASSERT_EQ(empty, "foo");
}

TEST(lazy_string, assign_emty){
    lazy_string notempty = "foo";
    notempty = lazy_string();
    ASSERT_TRUE(notempty.empty());
}

TEST(relational_ops, array_less_and_lazy_string){
    lazy_string::const_pointer str1 = "abc";
    lazy_string str2 = "abcd";
    lazy_string str3 = "aa";
    ASSERT_LT(str1, str2);
    ASSERT_LT(str3, str1);
    ASSERT_LT(str3, str2);
}

TEST(lazy_istring, test_comparison){
    lazy_istring foo = "FOO";
    ASSERT_EQ(foo, "foo");
}

TEST(lazy_string, proxy_chained_assignment){
    lazy_string foo = "foo";
    auto f = foo[0];
    lazy_istring::value_type chr = 'x';
    chr = f = 'b';
    ASSERT_EQ(chr, 'b');
    ASSERT_EQ(foo, "boo");
}

TEST(lazy_string, throws_exception_if_index_is_out_of_bounds){
    lazy_string foo = "foo";
    ASSERT_NO_THROW(foo[2]);
    ASSERT_THROW(foo[3], lazy_string::out_of_range_exception);
}


