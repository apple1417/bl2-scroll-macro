cmake_minimum_required(VERSION 3.24)

project(bl2-scroll-macro)
add_library(bl2-scroll-macro SHARED)

target_compile_features(bl2-scroll-macro PUBLIC cxx_std_20)
set_target_properties(bl2-scroll-macro PROPERTIES
    EXPORT_COMPILE_COMMANDS True
    COMPILE_WARNING_AS_ERROR True
    PREFIX ""
    OUTPUT_NAME "ddraw"
)

if(MSVC)
    target_compile_options(bl2-scroll-macro PRIVATE /W4)
else()
    target_compile_options(bl2-scroll-macro PRIVATE -Wall -Wextra -Wpedantic)
endif()

set(UNREALSDK_ARCH x86)
set(UNREALSDK_UE_VERSION UE3)
add_subdirectory(libs/unrealsdk)


file(GLOB_RECURSE sources CONFIGURE_DEPENDS "src/*.cpp" "src/*.h")

target_sources(bl2-scroll-macro PRIVATE ${sources})

target_include_directories(bl2-scroll-macro PUBLIC "src")
target_link_libraries(bl2-scroll-macro PUBLIC unrealsdk)

target_precompile_headers(bl2-scroll-macro PUBLIC "src/pch.h")


if(MSVC)
    # Enable Edit and Continue
    string(REPLACE "/Zi" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    string(REPLACE "/Zi" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REPLACE "/Zi" "" CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
    string(REPLACE "/Zi" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")

    target_compile_options(bl2-scroll-macro PUBLIC "$<$<CONFIG:DEBUG>:/ZI>")
    target_link_options(bl2-scroll-macro PUBLIC "/INCREMENTAL")

    # UTF-8 encoded source files
    target_compile_options(bl2-scroll-macro PUBLIC "/utf-8")
endif()

if (MINGW)
    # Want to link statically into a single file
    target_link_options(bl2-scroll-macro PRIVATE "-static")
endif()

# Postbuild
set(POSTBUILD_SCRIPT "postbuild")
if(CMAKE_HOST_WIN32)
    set(POSTBUILD_SCRIPT "${POSTBUILD_SCRIPT}.bat")
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${POSTBUILD_SCRIPT}")
    add_custom_command(
        TARGET bl2-scroll-macro
        POST_BUILD
        COMMAND ${POSTBUILD_SCRIPT} "\"$<SHELL_PATH:$<TARGET_FILE:bl2-scroll-macro>>\""
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()
