cmake_minimum_required(VERSION 3.6)
if (POLICY CMP0054)
  cmake_policy(SET CMP0054 NEW)
endif()
if (POLICY CMP0057)
  cmake_policy(SET CMP0057 NEW)
endif()
if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif()

# CMP0075 Include file check macros honor CMAKE_REQUIRED_LIBRARIES
if(POLICY CMP0075)
  cmake_policy(SET CMP0075 NEW)
endif()
set(PACKAGE_NAME      "muduo")
if (NOT DEFINED PACKAGE_VERSION)
  set(PACKAGE_VERSION   "v2.0.4")
  set(${PACKAGE_NAME}_VERSION 2.0.4)
endif()
set(PACKAGE_STRING    "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_TARNAME   "${PACKAGE_NAME}-${PACKAGE_VERSION}")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
project(${PACKAGE_NAME} C CXX)
#enable_testing()
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)
set(INCLUDE_INSTALL_DIR include CACHE STRING
    "The subdirectory where header files should be installed")
set(LIB_INSTALL_DIR lib CACHE STRING
    "The subdirectory where libraries should be installed")
set(BIN_INSTALL_DIR bin CACHE STRING
    "The subdirectory where binaries should be installed")
set(CMAKE_INSTALL_DIR ${LIB_INSTALL_DIR}/cmake/${PROJECT_NAME} CACHE STRING
    "The subdirectory where CMake package config files should be installed")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()
# only build examples if this is the main project

if(CMAKE_PROJECT_NAME STREQUAL "muduo")
  if(DEFINED MUDUO_BUILD_EXAMPLES)
      unset(MUDUO_BUILD_EXAMPLES CACHE)
  endif()
  option(MUDUO_BUILD_EXAMPLES "Build Muduo examples" ON)
  message(STATUS "MUDUO_BUILD_EXAMPLES = ${MUDUO_BUILD_EXAMPLES}")
endif()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(MUDUO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/muduo")
set(
  MUDUO_DIR_PREFIXES
  "${CMAKE_CURRENT_SOURCE_DIR}:${CMAKE_CURRENT_BINARY_DIR}"
)
include(GNUInstallDirs)
set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
set(MUDUO_HAVE_PTHREAD "${CMAKE_USE_PTHREADS_INIT}")
list(APPEND CMAKE_REQUIRED_LIBRARIES Threads::Threads)
list(APPEND MUDUO_LINK_LIBRARIES Threads::Threads)

#如果编译器是msvc 
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
# using Clang
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CXX_FLAGS
  -g
  -DVALGRIND
  -DCHECK_PTHREAD_RETURN_VALUE
  -D_FILE_OFFSET_BITS=64
  -Wall
  -Wextra
   -Werror
  -Wconversion
  -Wno-unused-parameter
  -Wold-style-cast
  -Woverloaded-virtual
  -Wpointer-arith
  -Wshadow
  -Wwrite-strings
  -march=native
  # -MMD
  -std=c++11
  -rdynamic
  )
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
# using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(CXX_FLAGS
  #-g
  # -DVALGRIND
  -DCHECK_PTHREAD_RETURN_VALUE
  -D_FILE_OFFSET_BITS=64
  -Wall
  #-Wextra
  # -Werror
  #-Wconversion
  #-Wno-unused-parameter
  #-Wold-style-cast
  #-Woverloaded-virtual
  #-Wpointer-arith
  #-Wshadow
  #-Wwrite-strings
  #-march=native
  # -MMD
  #-std=c++11
  #-rdynamic
  )
endif()

if(CMAKE_BUILD_BITS EQUAL 32)
  list(APPEND CXX_FLAGS "-m32")
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  list(APPEND CXX_FLAGS "-Wno-null-dereference")
  list(APPEND CXX_FLAGS "-Wno-sign-conversion")
  list(APPEND CXX_FLAGS "-Wno-unused-local-typedef")
  list(APPEND CXX_FLAGS "-Wthread-safety")
  list(REMOVE_ITEM CXX_FLAGS "-rdynamic")
endif()
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

#set(CMAKE_CXX_FLAGS_DEBUG "-O0")
#set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

#likely muduo-deps.cmake
include(CheckCXXSourceCompiles)
include(CheckCXXSymbolExists)
include(CheckIncludeFileCXX)
include(CheckFunctionExists)
include(CMakePushCheckState)

set(
  BOOST_LINK_STATIC "auto"
  CACHE STRING
  "Whether to link against boost statically or dynamically."
)
set(MUDUO_BOOST_LINK_STATIC "${BOOST_LINK_STATIC}")
set(Boost_USE_STATIC_LIBS "${MUDUO_BOOST_LINK_STATIC}")
find_package(Boost 1.51.0 MODULE
  COMPONENTS
   unit_test_framework
    context
    filesystem
    program_options
    regex
    system
    thread
  REQUIRED
)

if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  list(APPEND MUDUO_LINK_LIBRARIES ${Boost_LIBRARIES})
  list(APPEND MUDUO_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS})
endif()

find_package(Protobuf)
if(Protobuf_FOUND)
  list(APPEND MUDUO_LINK_LIBRARIES ${Protobuf_LIBRARIES})
  list(APPEND MUDUO_INCLUDE_DIRECTORIES ${Protobuf_INCLUDE_DIRS})
endif()

find_package(CURL)
if(CURL_FOUND)
  list(APPEND MUDUO_LINK_LIBRARIES ${CURL_LIBRARIES})
  list(APPEND MUDUO_INCLUDE_DIRECTORIES ${CURL_INCLUDE_DIRS})
endif()

find_package(ZLIB MODULE)
set(MUDUO_HAVE_LIBZ ${ZLIB_FOUND})
if (ZLIB_FOUND)
  list(APPEND MUDUO_INCLUDE_DIRECTORIES ${ZLIB_INCLUDE_DIRS})
  list(APPEND MUDUO_LINK_LIBRARIES ${ZLIB_LIBRARIES})
  list(APPEND CMAKE_REQUIRED_LIBRARIES ${ZLIB_LIBRARIES})
endif()

find_package(c-ares CONFIG REQUIRED)
set(MUDUO_HAVE_CARES ${CARES_FOUND})
if (CARES_FOUND)
  list(APPEND MUDUO_INCLUDE_DIRECTORIES ${CARES_INCLUDE_DIR})
  list(APPEND MUDUO_LINK_LIBRARIES ${CARES_LIBRARIES})
endif()

find_package(OpenSSL 1.1.1 MODULE REQUIRED)
list(APPEND MUDUO_LINK_LIBRARIES ${OPENSSL_LIBRARIES})
list(APPEND MUDUO_INCLUDE_DIRECTORIES ${OPENSSL_INCLUDE_DIR})
list(APPEND CMAKE_REQUIRED_LIBRARIES ${OPENSSL_LIBRARIES})
list(APPEND CMAKE_REQUIRED_INCLUDES ${OPENSSL_INCLUDE_DIR})
check_function_exists(ASN1_TIME_diff MUDUO_HAVE_OPENSSL_ASN1_TIME_DIFF)
list(REMOVE_ITEM CMAKE_REQUIRED_LIBRARIES ${OPENSSL_LIBRARIES})
list(REMOVE_ITEM CMAKE_REQUIRED_INCLUDES ${OPENSSL_INCLUDE_DIR})
if (ZLIB_FOUND)
    list(REMOVE_ITEM CMAKE_REQUIRED_LIBRARIES ${ZLIB_LIBRARIES})
endif()

list(APPEND MUDUO_LINK_LIBRARIES ${CMAKE_DL_LIBS})
list(APPEND CMAKE_REQUIRED_LIBRARIES ${CMAKE_DL_LIBS})

add_library(muduo_deps INTERFACE)
find_package(fmt CONFIG)
if (NOT DEFINED fmt_CONFIG)
    # Fallback on a normal search on the current system
    find_package(Fmt MODULE REQUIRED)
endif()
target_link_libraries(muduo_deps INTERFACE fmt::fmt)

list(REMOVE_DUPLICATES MUDUO_INCLUDE_DIRECTORIES)
target_include_directories(muduo_deps INTERFACE ${MUDUO_INCLUDE_DIRECTORIES})
target_link_libraries(muduo_deps INTERFACE
  ${MUDUO_LINK_LIBRARIES}
  ${MUDUO_SHINY_DEPENDENCIES}
)
function(auto_sources RETURN_VALUE PATTERN SOURCE_SUBDIRS)
  if ("${SOURCE_SUBDIRS}" STREQUAL "RECURSE")
    SET(PATH ".")
    if (${ARGC} EQUAL 4)
      list(GET ARGV 3 PATH)
    endif ()
  endif()

  if ("${SOURCE_SUBDIRS}" STREQUAL "RECURSE")
    unset(${RETURN_VALUE})
    file(GLOB SUBDIR_FILES "${PATH}/${PATTERN}")
    list(APPEND ${RETURN_VALUE} ${SUBDIR_FILES})

    file(GLOB subdirs RELATIVE ${PATH} ${PATH}/*)

    foreach(DIR ${subdirs})
      if (IS_DIRECTORY ${PATH}/${DIR})
        if (NOT "${DIR}" STREQUAL "CMakeFiles")
          file(GLOB_RECURSE SUBDIR_FILES "${PATH}/${DIR}/${PATTERN}")
          list(APPEND ${RETURN_VALUE} ${SUBDIR_FILES})
        endif()
      endif()
    endforeach()
  else()
    file(GLOB ${RETURN_VALUE} "${PATTERN}")

    foreach (PATH ${SOURCE_SUBDIRS})
      file(GLOB SUBDIR_FILES "${PATH}/${PATTERN}")
      list(APPEND ${RETURN_VALUE} ${SUBDIR_FILES})
    endforeach()
  endif ()

  set(${RETURN_VALUE} ${${RETURN_VALUE}} PARENT_SCOPE)
endfunction(auto_sources)

auto_sources(files "*.cpp" "RECURSE" "${MUDUO_DIR}")
auto_sources(files "*.cc" "RECURSE" "${MUDUO_DIR}")
auto_sources(hfiles "*.h" "RECURSE"  "${MUDUO_DIR}")

list(APPEND muduo_base_files
  ${files} ${hfiles}
)

add_library(muduo_base OBJECT
  ${muduo_base_files}
)

# Generate pkg-config variables from muduo_deps before we add our own
# build/install-time include directory generator expressions
function(gen_pkgconfig_vars)
  if (NOT ${ARGC} EQUAL 2)
    message(FATAL_ERROR "gen_pkgconfig_vars() requires exactly 2 arguments")
  endif()
  set(var_prefix "${ARGV0}")
  set(target "${ARGV1}")

  get_target_property(target_cflags "${target}" INTERFACE_COMPILE_OPTIONS)
  if(target_cflags)
    list(APPEND cflags "${target_cflags}")
  endif()
  get_target_property(
    target_inc_dirs "${target}" INTERFACE_INCLUDE_DIRECTORIES)
  if(target_inc_dirs)
    list(APPEND include_dirs "${target_inc_dirs}")
  endif()
  get_target_property(target_defns "${target}" INTERFACE_COMPILE_DEFINITIONS)
  if(target_defns)
    list(APPEND definitions "${target_defns}")
  endif()

  # The INTERFACE_LINK_LIBRARIES list is unfortunately somewhat awkward to
  # process.  Entries in this list may be any of
  # - target names
  # - absolute paths to a library file
  # - plain library names that need "-l" prepended
  # - other linker flags starting with "-"
  #
  # Walk through each entry and transform it into the desired arguments
  get_target_property(link_libs "${target}" INTERFACE_LINK_LIBRARIES)
  if(link_libs)
    foreach(lib_arg IN LISTS link_libs)
      if(TARGET "${lib_arg}")
        # Add any compile options specified in the targets
        # INTERFACE_COMPILE_OPTIONS.  We don't need to process its
        # INTERFACE_LINK_LIBRARIES property, since our INTERFACE_LINK_LIBRARIES
        # will already include its entries transitively.
        get_target_property(lib_cflags "${lib_arg}" INTERFACE_COMPILE_OPTIONS)
        if(lib_cflags)
          list(APPEND cflags "${lib_cflags}")
        endif()
        get_target_property(lib_defs "${lib_arg}"
          INTERFACE_COMPILE_DEFINITIONS)
        if(lib_defs)
          list(APPEND definitions "${lib_defs}")
        endif()
      elseif(lib_arg MATCHES "^[-/]")
        list(APPEND private_libs "${lib_arg}")
      else()
        list(APPEND private_libs "-l${lib_arg}")
      endif()
    endforeach()
  endif()

  list(APPEND cflags "${CMAKE_REQUIRED_FLAGS}")
  if(definitions)
    list(REMOVE_DUPLICATES definitions)
    foreach(def_arg IN LISTS definitions)
      list(APPEND cflags "-D${def_arg}")
    endforeach()
  endif()
  if(include_dirs)
    list(REMOVE_DUPLICATES include_dirs)
    foreach(inc_dir IN LISTS include_dirs)
      list(APPEND cflags "-I${inc_dir}")
    endforeach()
  endif()

  # Set the output variables
  string(REPLACE ";" " " cflags "${cflags}")
  string(REPLACE ";" " " private_libs "${private_libs}")

  # Since CMake 3.18 FindThreads may include a generator expression requiring
  # a target, which gets propagated to us through INTERFACE_COMPILE_OPTIONS.
  # Before CMake 3.19 there's no way to solve this in a general way, so we
  # work around the specific case. See #1414 and CMake bug #21074.
  if(CMAKE_VERSION VERSION_LESS 3.19)
    string(REPLACE
      "<COMPILE_LANG_AND_ID:CUDA,NVIDIA>" "<COMPILE_LANGUAGE:CUDA>"
      cflags "${cflags}"
    )
  endif()

  set("${var_prefix}_CFLAGS" "${cflags}" PARENT_SCOPE)
  set("${var_prefix}_PRIVATE_LIBS" "${private_libs}" PARENT_SCOPE)
endfunction()
gen_pkgconfig_vars(MUDUO_PKGCONFIG muduo_deps)

target_include_directories(muduo_deps
  BEFORE
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

target_include_directories(muduo_deps
  INTERFACE
    $<INSTALL_INTERFACE:include>
)

target_include_directories(muduo_base
  PUBLIC
    $<TARGET_PROPERTY:muduo_deps,INTERFACE_INCLUDE_DIRECTORIES>
)
target_compile_definitions(muduo_base
  PUBLIC
    $<TARGET_PROPERTY:muduo_deps,INTERFACE_COMPILE_DEFINITIONS>
)


include(CMakePackageConfigHelpers)
set(MUDUO_INSTALL_TARGETS muduo muduo_deps)

add_library(muduo
  $<TARGET_OBJECTS:muduo_base>
)

set_property(TARGET muduo PROPERTY VERSION ${PACKAGE_VERSION})
target_compile_features(muduo INTERFACE cxx_generic_lambdas)
target_link_libraries(muduo PUBLIC muduo_deps)

function(auto_install_files rootName rootDir)
  file(TO_CMAKE_PATH "${rootDir}" rootDir)
  string(LENGTH "${rootDir}" rootDirLength)
  set(sourceGroups)
  foreach (fil ${ARGN})
    file(TO_CMAKE_PATH "${fil}" filePath)
    string(FIND "${filePath}" "/" rIdx REVERSE)
    if (rIdx EQUAL -1)
      message(FATAL_ERROR "Unable to locate the final forward slash in '${filePath}'!")
    endif()
    string(SUBSTRING "${filePath}" 0 ${rIdx} filePath)

    string(LENGTH "${filePath}" filePathLength)
    string(FIND "${filePath}" "${rootDir}" rIdx)
    if (rIdx EQUAL 0)
      math(EXPR filePathLength "${filePathLength} - ${rootDirLength}")
      string(SUBSTRING "${filePath}" ${rootDirLength} ${filePathLength} fileGroup)
      install(FILES ${fil}
              DESTINATION ${INCLUDE_INSTALL_DIR}/${rootName}${fileGroup})
    endif()
  endforeach()
endfunction()

install(TARGETS ${MUDUO_INSTALL_TARGETS}
  EXPORT muduo
  RUNTIME DESTINATION ${BIN_INSTALL_DIR}
  LIBRARY DESTINATION ${LIB_INSTALL_DIR}
  ARCHIVE DESTINATION ${LIB_INSTALL_DIR})
  auto_install_files(muduo ${MUDUO_DIR}
  ${hfiles}
)

configure_package_config_file(
  ${PROJECT_NAME}-config.cmake.in
  ${PROJECT_NAME}-config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_DIR}
  PATH_VARS
    INCLUDE_INSTALL_DIR
    CMAKE_INSTALL_DIR
)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  VERSION ${PACKAGE_NAME}_VERSION
  COMPATIBILITY AnyNewerVersion
)

install(
  FILES
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_DIR}
  COMPONENT dev
)

install(
  EXPORT ${PROJECT_NAME}
  DESTINATION ${CMAKE_INSTALL_DIR}
  NAMESPACE muduo::
  FILE ${PROJECT_NAME}-targets.cmake
  COMPONENT dev
)


#message PROJECT_BINARY_DIR
message(STATUS "PROJECT_BINARY_DIR = ${PROJECT_BINARY_DIR}")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
include_directories(${PROJECT_SOURCE_DIR})
string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
message(STATUS "CXX_FLAGS = " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_${BUILD_TYPE}})
add_subdirectory(muduo/base)
add_subdirectory(muduo/net)
if(MUDUO_BUILD_EXAMPLES)
  #add_subdirectory(contrib)
  #add_subdirectory(examples)
else()
  if(CARES_INCLUDE_DIR AND CARES_LIBRARIES)
    add_subdirectory(examples/cdns)
  endif()
endif()