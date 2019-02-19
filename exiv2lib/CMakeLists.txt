# Note that this is a hack for testing the internals of the library. If EXIV2_BUILD_UNIT_TESTS==OFF
# Then we only export the symbols that are explicitly exported
if( EXIV2_BUILD_UNIT_TESTS )
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON) # Requires CMake 3.3.3
endif()

include(CMakePackageConfigHelpers)

add_library(exiv2lib_int OBJECT
    src/canonmn_int.cpp         src/canonmn_int.hpp
    src/casiomn_int.cpp         src/casiomn_int.hpp
    src/cr2header_int.cpp       src/cr2header_int.hpp
    src/crwimage_int.cpp        src/crwimage_int.hpp
    src/fujimn_int.cpp          src/fujimn_int.hpp
    src/helper_functions.cpp    src/helper_functions.hpp
    src/image_int.cpp           src/image_int.hpp
    src/makernote_int.cpp       src/makernote_int.hpp
    src/minoltamn_int.cpp       src/minoltamn_int.hpp
    src/nikonmn_int.cpp         src/nikonmn_int.hpp
    src/olympusmn_int.cpp       src/olympusmn_int.hpp
    src/orfimage_int.cpp        src/orfimage_int.hpp
    src/panasonicmn_int.cpp     src/panasonicmn_int.hpp
    src/pentaxmn_int.cpp        src/pentaxmn_int.hpp
    src/rw2image_int.cpp        src/rw2image_int.hpp
    src/safe_op.hpp
    src/samsungmn_int.cpp       src/samsungmn_int.hpp
    src/sigmamn_int.cpp         src/sigmamn_int.hpp
    src/sonymn_int.cpp          src/sonymn_int.hpp
    src/tags_int.cpp            src/tags_int.hpp
    src/tiffcomposite_int.cpp   src/tiffcomposite_int.hpp
    src/tiffimage_int.cpp       src/tiffimage_int.hpp
    src/tiffvisitor_int.cpp     src/tiffvisitor_int.hpp
    src/tifffwd_int.hpp
    src/timegm.h
    src/unused.h
)

add_library(exiv2lib
                            include/exiv2/config.h
                            include/exiv2/exiv2.hpp
                            include/exiv2/slice.hpp
    src/basicio.cpp         include/exiv2/basicio.hpp
    src/bigtiffimage.cpp
    src/bmpimage.cpp        include/exiv2/bmpimage.hpp
    src/convert.cpp         include/exiv2/convert.hpp
    src/cr2image.cpp        include/exiv2/cr2image.hpp
    src/crwimage.cpp        include/exiv2/crwimage.hpp
    src/datasets.cpp        include/exiv2/datasets.hpp
    src/easyaccess.cpp      include/exiv2/easyaccess.hpp
    src/error.cpp           include/exiv2/error.hpp
    src/exif.cpp            include/exiv2/exif.hpp
    src/futils.cpp          include/exiv2/futils.hpp
    src/gifimage.cpp        include/exiv2/gifimage.hpp
    src/http.cpp            include/exiv2/http.hpp
    src/image.cpp           include/exiv2/image.hpp
    src/ini.cpp             include/exiv2/ini.hpp
    src/iptc.cpp            include/exiv2/iptc.hpp
    src/jp2image.cpp        include/exiv2/jp2image.hpp
    src/jpgimage.cpp        include/exiv2/jpgimage.hpp
    src/metadatum.cpp       include/exiv2/metadatum.hpp
    src/mrwimage.cpp        include/exiv2/mrwimage.hpp
    src/orfimage.cpp        include/exiv2/orfimage.hpp
    src/pgfimage.cpp        include/exiv2/pgfimage.hpp
    src/preview.cpp         include/exiv2/preview.hpp
    src/properties.cpp      include/exiv2/properties.hpp
    src/psdimage.cpp        include/exiv2/psdimage.hpp
    src/rafimage.cpp        include/exiv2/rafimage.hpp
    src/rw2image.cpp        include/exiv2/rw2image.hpp
    src/tags.cpp            include/exiv2/tags.hpp
    src/tgaimage.cpp        include/exiv2/tgaimage.hpp
    src/tiffimage.cpp       include/exiv2/tiffimage.hpp
    src/types.cpp           include/exiv2/types.hpp
    src/value.cpp           include/exiv2/value.hpp
    src/version.cpp         include/exiv2/version.hpp
    src/webpimage.cpp       include/exiv2/webpimage.hpp
    src/xmp.cpp             include/exiv2/xmp_exiv2.hpp
    src/xmpsidecar.cpp      include/exiv2/xmpsidecar.hpp
    $<TARGET_OBJECTS:exiv2lib_int>
)

generate_export_header(exiv2lib
    EXPORT_MACRO_NAME EXIV2API
    EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/exiv2lib_export.h
    STATIC_DEFINE exiv2lib_STATIC
)

# Conditional addition of sources to library targets
# ---------------------------------------------------------

if(MINGW OR MSYS)
    target_sources(exiv2lib_int PRIVATE localtime.c)
endif()

if( EXIV2_ENABLE_WEBREADY )
    if( EXIV2_ENABLE_CURL)
        target_sources(exiv2lib PRIVATE src/easyaccess.cpp include/exiv2/easyaccess.hpp)
    endif()
endif()

if( EXIV2_ENABLE_PNG )
    target_sources(exiv2lib_int PRIVATE src/pngchunk_int.cpp)
    target_sources(exiv2lib PRIVATE src/pngimage.cpp include/exiv2/pngimage.hpp)
endif()

# Other library target properties
# ---------------------------------------------------------

if (${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
    # Do not check format overflows on this file, to skip a false positive warning
    set_source_files_properties(value.cpp PROPERTIES COMPILE_FLAGS -Wno-format-overflow)
endif()

set_target_properties( exiv2lib PROPERTIES
    VERSION       ${PROJECT_VERSION}
    SOVERSION     ${PROJECT_VERSION_MINOR}
    OUTPUT_NAME   exiv2
    PDB_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMPILE_FLAGS ${EXTRA_COMPILE_FLAGS}
)

if (MSVC)
    set_target_properties(exiv2lib PROPERTIES LINK_FLAGS "/ignore:4099")
endif()

set_target_properties(exiv2lib_int PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    COMPILE_DEFINITIONS exiv2lib_STATIC
)


target_include_directories(exiv2lib_int PRIVATE ${ZLIB_INCLUDE_DIR})
target_include_directories(exiv2lib PRIVATE ${ZLIB_INCLUDE_DIR})

if (EXIV2_ENABLE_XMP)
    target_link_libraries(exiv2lib PRIVATE exiv2-xmp)
elseif(EXIV2_ENABLE_EXTERNAL_XMP)
    target_link_libraries(exiv2lib PUBLIC ${XMPSDK_LIBRARY})
    target_include_directories(exiv2lib PUBLIC ${XMPSDK_INCLUDE_DIR})
    target_compile_definitions(exiv2lib PUBLIC EXV_ADOBE_XMPSDK=2016)
endif()

# Use include/exiv2 to build libary code
target_include_directories(exiv2lib PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/exiv2lib/include>
    $<INSTALL_INTERFACE:include>
)

target_include_directories(exiv2lib_int PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/exiv2lib/include>
)

if (EXIV2_ENABLE_WEBREADY)
    if( EXIV2_ENABLE_CURL )
        target_include_directories(exiv2lib SYSTEM PUBLIC ${CURL_INCLUDE_DIR} )
        target_link_libraries(exiv2lib PUBLIC ${CURL_LIBRARIES})
        if (USING_CONAN)
            target_compile_definitions(exiv2lib PUBLIC ${CONAN_COMPILE_DEFINITIONS_LIBCURL})
            target_link_libraries(exiv2lib PUBLIC ${CONAN_EXE_LINKER_FLAGS_LIBCURL})
            if (NOT APPLE)
                target_link_libraries(exiv2lib PUBLIC CONAN_PKG::OpenSSL)
            endif()
        endif()
    endif()

endif()

if (WIN32)
    target_compile_definitions(exiv2lib PRIVATE PSAPI_VERSION=1) # to be compatible with <= WinVista (#905)
endif()

if (NOT MSVC)
    if ( UNIX AND NOT CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" )
        target_link_libraries( exiv2lib PRIVATE ${CMAKE_DL_LIBS}) # -ldl = dynamic loader used by src/version.cpp
    endif()

    if (CYGWIN OR MINGW OR MSYS)
        target_link_libraries( exiv2lib PRIVATE psapi ws2_32 )
    endif()

    target_link_libraries( exiv2lib PRIVATE Threads::Threads)
else()
    target_link_libraries( exiv2lib PRIVATE psapi ws2_32)
endif()

if( EXIV2_ENABLE_PNG )
    target_link_libraries( exiv2lib PRIVATE ${ZLIB_LIBRARIES})
endif()

if( EXIV2_ENABLE_NLS )
    target_link_libraries(exiv2lib PRIVATE ${Intl_LIBRARIES})
    target_include_directories(exiv2lib PRIVATE ${Intl_INCLUDE_DIRS})
    target_include_directories(exiv2lib_int PRIVATE ${Intl_INCLUDE_DIRS})
    # Definition needed for translations
    target_compile_definitions(exiv2lib PUBLIC EXV_LOCALEDIR="/../${CMAKE_INSTALL_LOCALEDIR}")
endif()

if( ICONV_FOUND )
    target_link_libraries( exiv2lib PRIVATE Iconv::Iconv)
endif()

write_basic_package_version_file(exiv2ConfigVersion.cmake COMPATIBILITY ExactVersion)

install(TARGETS exiv2lib EXPORT exiv2Config
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES
    ${CMAKE_BINARY_DIR}/exv_conf.h
    ${CMAKE_BINARY_DIR}/exiv2lib_export.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/exiv2)

install(EXPORT exiv2Config DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/exiv2")

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/exiv2ConfigVersion.cmake DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/exiv2")

