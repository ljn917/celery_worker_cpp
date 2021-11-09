find_path(Hiredis_INCLUDE_DIRS
    NAMES hiredis/hiredis.h
    DOC "hiredis include dir"
    PATH_SUFFIXES hiredis
    HINTS
        ${HIREDIS_DIR}/include
        /usr/include
)

find_library(Hiredis_LIBRARIES
    NAMES hiredis
    DOC "hiredis library"
    HINTS
        ${HIREDIS_DIR}/lib
        /usr/lib64
        /usr/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Hiredis
    DEFAULT_MSG
    Hiredis_INCLUDE_DIRS
    Hiredis_LIBRARIES
)
mark_as_advanced(Hiredis_INCLUDE_DIRS Hiredis_LIBRARIES)

if(Hiredis_FOUND)
    add_library(Hiredis::Hiredis INTERFACE IMPORTED)
    set_target_properties(Hiredis::Hiredis
        PROPERTIES INTERFACE_INCLUDE_DIRECTORIES    "${Hiredis_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES                    "${Hiredis_LIBRARIES}"
    )
endif(Hiredis_FOUND)
