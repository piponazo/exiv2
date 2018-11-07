#include "basicio.hpp"

#include "gtestwrapper.h"

TEST(AFileIo, canBeCreatedEvenWithNonExistingFiles)
{
    ASSERT_NO_THROW(Exiv2::FileIo file("NonExistingFile.txt"));
}

TEST(AFileIo, failsTryingToOpenANonExistingFile)
{
    Exiv2::FileIo file("NonExistingFile.txt");
    ASSERT_EQ(1, file.open());
}