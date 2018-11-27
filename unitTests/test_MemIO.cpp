#include "MemIo.hpp"
#include "error.hpp"

#include "gtestwrapper.h"

using namespace Exiv2;

struct ADefaultMemIO: public testing::Test
{
    MemIo mem;
};

TEST_F(ADefaultMemIO, initialValues)
{
    ASSERT_EQ(0ul, mem.size());
    ASSERT_EQ(0, mem.tell());
    ASSERT_TRUE(mem.isopen());
    ASSERT_FALSE(mem.error());
    ASSERT_FALSE(mem.eof());
    ASSERT_STREQ("MemIo", mem.path().c_str());
}

TEST_F(ADefaultMemIO, closeAlwaysReturn0)
{
    ASSERT_EQ(0, mem.close());
}

TEST_F(ADefaultMemIO, writeAllocatesAutomaticallyMemory)
{
    byte x[] = {1,2,3};
    ASSERT_EQ(3, mem.write(x, 3));
    ASSERT_EQ(3ul, mem.size());
    ASSERT_EQ(3, mem.tell());
}

TEST_F(ADefaultMemIO, writeWithNullPointerDoesNotThrow)
{
    ASSERT_NO_THROW(mem.write(NULL, 3));
    ASSERT_EQ(3ul, mem.size());
    ASSERT_EQ(3, mem.tell());
}
