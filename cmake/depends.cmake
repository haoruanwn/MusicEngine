include(FetchContent)
FetchContent_Declare(
        taglib
        GIT_REPOSITORY https://github.com/taglib/taglib.git
        GIT_TAG        v2.1.1
)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Force static libraries")
set(BUILD_EXAMPLES OFF CACHE BOOL "Don't build examples")
set(BUILD_TESTING OFF CACHE BOOL "Don't build tests")
FetchContent_MakeAvailable(taglib)
FetchContent_GetProperties(taglib)