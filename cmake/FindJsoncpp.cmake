# From drogon project https://github.com/drogonframework/drogon/blob/master/cmake_modules/FindJsoncpp.cmake

find_path(Jsoncpp_INCLUDE_DIRS
    NAMES json/json.h
    DOC "jsoncpp include dir"
    PATH_SUFFIXES jsoncpp
)

find_library(Jsoncpp_LIBRARIES NAMES jsoncpp DOC "jsoncpp library")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Jsoncpp
                                  DEFAULT_MSG
                                  Jsoncpp_INCLUDE_DIRS
                                  Jsoncpp_LIBRARIES)
mark_as_advanced(Jsoncpp_INCLUDE_DIRS Jsoncpp_LIBRARIES)

if(Jsoncpp_FOUND)
  if(NOT EXISTS ${Jsoncpp_INCLUDE_DIRS}/json/version.h)
    message(FATAL_ERROR "Error: jsoncpp lib is too old.....stop")
  endif()
  if(NOT WIN32)
    exec_program(
      cat
      ARGS
      "${Jsoncpp_INCLUDE_DIRS}/json/version.h |grep JSONCPP_VERSION_STRING|sed s'/.*define/define/'|awk '{printf $3}'|sed s'/\"//g'"
      OUTPUT_VARIABLE
      jsoncpp_ver)
    message(STATUS "jsoncpp verson:" ${jsoncpp_ver})
    if(jsoncpp_ver LESS 1.7)
      message(
        FATAL_ERROR
          "jsoncpp lib is too old, please get new version from https://github.com/open-source-parsers/jsoncpp"
        )
    endif(jsoncpp_ver LESS 1.7)
  endif()
  add_library(Jsoncpp::Jsoncpp INTERFACE IMPORTED)
  set_target_properties(Jsoncpp::Jsoncpp
                        PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                                   "${Jsoncpp_INCLUDE_DIRS}"
                                   INTERFACE_LINK_LIBRARIES
                                   "${Jsoncpp_LIBRARIES}")

endif(Jsoncpp_FOUND)
