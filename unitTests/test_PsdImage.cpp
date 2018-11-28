#include "psdimage.hpp"

#include "gtestwrapper.h"

struct AFakePsdImage : public testing::Test
{
    AFakePsdImage() : image(ptr)
    {
    }

    Exiv2::BasicIo::AutoPtr ptr;
    Exiv2::PsdImage image;
};

TEST_F(AFakePsdImage, isConstructed)
{
}

TEST_F(AFakePsdImage, setCommentThrows)
{
    EXPECT_THROW(image.setComment(("blabla")), Exiv2::AnyError);
}

TEST_F(AFakePsdImage, throwsTryingToReadMetadata)
{
    EXPECT_THROW(image.readMetadata(), Exiv2::AnyError);
}

TEST_F(AFakePsdImage, throwsTryingToWriteMetadata)
{
    EXPECT_THROW(image.writeMetadata(), Exiv2::AnyError);
}

const std::string psdPath = TEST_DATA_DIR + std::string("exiv2-photoshop.psd");

struct APsdImage : public testing::Test
{
    APsdImage() : image(Exiv2::ImageFactory::open(psdPath, false))
    {
    }
    Exiv2::Image::AutoPtr image;
};

TEST_F(APsdImage, throwsTryingToReadMetadata)
{
    EXPECT_NO_THROW(image->readMetadata());
}
