#include "psdimage.hpp"

#include "gtestwrapper.h"

using namespace Exiv2;

TEST(APsdImage, isConstructed)
{
    BasicIo::AutoPtr ptr;
    PsdImage image(ptr);
}

TEST(APsdImage, setCommentThrows)
{
    BasicIo::AutoPtr ptr;
    PsdImage image(ptr);
    EXPECT_THROW(image.setComment(("blabla")), Exiv2::Error);
}

TEST(APsdImage, throwsTryingToReadMetadata)
{
    BasicIo::AutoPtr ptr;
    PsdImage image(ptr);
    EXPECT_THROW(image.readMetadata(), Exiv2::Error);
}
