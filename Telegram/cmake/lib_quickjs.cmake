# This file is part of Omnigram,
# an advanced fork of Telegram Desktop.
#
# For license and copyright information please follow this link:
# https://github.com/omnigram-app/omnigram/blob/main/LICENSE

add_library(lib_quickjs STATIC)
init_target(lib_quickjs "(external)")

add_library(desktop-app::lib_quickjs ALIAS lib_quickjs)

set(quickjs_loc ${third_party_loc}/quickjs)

nice_target_sources(lib_quickjs ${quickjs_loc}
PRIVATE
    dtoa.c
    libregexp.c
    libunicode.c
    quickjs.c
)

target_include_directories(lib_quickjs
PUBLIC
    ${quickjs_loc}
)

target_compile_definitions(lib_quickjs
PRIVATE
    QUICKJS_NG_BUILD
    _GNU_SOURCE
)

set_target_properties(lib_quickjs PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
)

if (MSVC)
    target_compile_options(lib_quickjs PRIVATE /W0 /WX-)
else()
    target_compile_options(lib_quickjs PRIVATE -w)
endif()

if (NOT WIN32)
    target_link_libraries(lib_quickjs PRIVATE m ${CMAKE_DL_LIBS})
endif()
