#include "psdimage.hpp"

#include "gtestwrapper.h"

using namespace Exiv2;

struct AFakePsdImage : public testing::Test
{
    AFakePsdImage() : image(ptr)
    {
    }
    BasicIo::AutoPtr ptr;
    PsdImage image;
};

TEST_F(AFakePsdImage, isConstructed)
{
}

TEST_F(AFakePsdImage, setCommentThrows)
{
    EXPECT_THROW(image.setComment(("blabla")), Exiv2::Error);
}

TEST_F(AFakePsdImage, throwsTryingToReadMetadata)
{
    EXPECT_THROW(image.readMetadata(), Exiv2::Error);
}

TEST_F(AFakePsdImage, throwsTryingToWriteMetadata)
{
    EXPECT_THROW(image.writeMetadata(), Exiv2::Error);
}

const std::string psdPath = TEST_DATA_DIR + std::string("exiv2-photoshop.psd");

struct APsdImage : public testing::Test
{
    APsdImage() : image(ImageFactory::open(psdPath, false))
    {
    }
    Image::AutoPtr image;
};

TEST_F(APsdImage, throwsTryingToReadMetadata)
{
    EXPECT_NO_THROW(image->readMetadata());
}
