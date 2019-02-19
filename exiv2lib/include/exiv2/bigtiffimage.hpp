
#include <exiv2/basicio.hpp>
#include <exiv2/image.hpp>

namespace Exiv2
{

namespace ImageType
{
    const int bigtiff = 25;
}

Image::UniquePtr newBigTiffInstance(BasicIo::UniquePtr, bool);
bool isBigTiffType(BasicIo &, bool);

}
