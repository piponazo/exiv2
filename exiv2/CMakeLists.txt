add_executable( exiv2
     exiv2.cpp
     exiv2app.hpp
     actions.cpp actions.hpp
     getopt.cpp getopt.hpp
     utils.cpp utils.hpp
)

set_target_properties( exiv2 PROPERTIES
    COMPILE_FLAGS ${EXTRA_COMPILE_FLAGS}
    XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS[variant=Debug] "YES"
)

target_link_libraries( exiv2 PRIVATE exiv2lib )
target_include_directories(exiv2 PRIVATE ${PROJECT_SOURCE_DIR}/exiv2lib/src) # To find i18n.h

if( EXIV2_ENABLE_NLS )
    target_link_libraries(exiv2 PRIVATE ${Intl_LIBRARIES})
    target_include_directories(exiv2 PRIVATE ${Intl_INCLUDE_DIRS})
endif()

if (USING_CONAN AND WIN32 AND EXISTS ${PROJECT_BINARY_DIR}/conanDlls)
    # In case of using conan recipes with their 'shared' option turned on, we will have dlls of
    # the 3rd party dependencies in the conanDlls folder.

    # Copy 3rd party DLLs the bin folder. [build step]
    add_custom_command(TARGET exiv2 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_BINARY_DIR}/conanDlls ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

    # Copy 3rd party DLLs the bin folder. [install step]
    install(DIRECTORY ${PROJECT_BINARY_DIR}/conanDlls/ DESTINATION bin)
endif()

install(TARGETS exiv2 RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install( FILES exiv2.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 )

if (MSVC)
    set_target_properties(exiv2 PROPERTIES LINK_FLAGS "/ignore:4099")
endif()
