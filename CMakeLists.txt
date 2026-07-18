cmake_minimum_required(VERSION 3.20)

project(
  pmzf
  VERSION 0.1.0
  LANGUAGES CXX
)

include(GNUInstallDirs)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(CURL REQUIRED)

set(PMZF_TARGET_NAME "${PROJECT_NAME}")
set(PMZF_CORE_TARGET "pmzf_core")
set(PMZF_TEST_TARGET "pmzf_tests")

option(PMZF_BUILD_TESTS "Build pmzf tests" ON)
set(PMZF_SPAGYRIST_PROVIDER
    "source"
    CACHE STRING
    "How to provide Spagyrist: auto, source, fetch, or package.")
set_property(CACHE PMZF_SPAGYRIST_PROVIDER PROPERTY STRINGS auto source fetch package)
set(PMZF_SPAGYRIST_SOURCE_DIR
    "${CMAKE_CURRENT_LIST_DIR}/reference/libspagyrist"
    CACHE PATH
    "Path to the Spagyrist source tree. Used when spagyrist is not installed.")
set(PMZF_SPAGYRIST_GIT_REPOSITORY
    "https://github.com/recelsus/libspagyrist.git"
    CACHE STRING
    "Git repository URL for Spagyrist when PMZF_SPAGYRIST_PROVIDER=fetch.")
set(PMZF_SPAGYRIST_GIT_TAG
    "master"
    CACHE STRING
    "Git tag, branch, or commit for Spagyrist when PMZF_SPAGYRIST_PROVIDER=fetch.")

if(TARGET spagyrist::spagyrist)
  # The parent project already provides Spagyrist.
elseif(PMZF_SPAGYRIST_PROVIDER STREQUAL "source")
  if(NOT EXISTS "${PMZF_SPAGYRIST_SOURCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "PMZF_SPAGYRIST_SOURCE_DIR does not contain CMakeLists.txt: ${PMZF_SPAGYRIST_SOURCE_DIR}")
  endif()
  set(SPAGYRIST_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  add_subdirectory("${PMZF_SPAGYRIST_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/_deps/spagyrist" EXCLUDE_FROM_ALL)
elseif(PMZF_SPAGYRIST_PROVIDER STREQUAL "fetch")
  if(PMZF_SPAGYRIST_GIT_REPOSITORY STREQUAL "")
    message(FATAL_ERROR "PMZF_SPAGYRIST_GIT_REPOSITORY is required when PMZF_SPAGYRIST_PROVIDER=fetch")
  endif()
  include(FetchContent)
  set(SPAGYRIST_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(
    spagyrist
    GIT_REPOSITORY "${PMZF_SPAGYRIST_GIT_REPOSITORY}"
    GIT_TAG "${PMZF_SPAGYRIST_GIT_TAG}"
  )
  FetchContent_MakeAvailable(spagyrist)
elseif(PMZF_SPAGYRIST_PROVIDER STREQUAL "package")
  find_package(spagyrist REQUIRED)
elseif(PMZF_SPAGYRIST_PROVIDER STREQUAL "auto")
  if(EXISTS "${PMZF_SPAGYRIST_SOURCE_DIR}/CMakeLists.txt")
    set(SPAGYRIST_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    add_subdirectory("${PMZF_SPAGYRIST_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/_deps/spagyrist" EXCLUDE_FROM_ALL)
  else()
    find_package(spagyrist REQUIRED)
  endif()
else()
  message(FATAL_ERROR "Unknown PMZF_SPAGYRIST_PROVIDER: ${PMZF_SPAGYRIST_PROVIDER}")
endif()

add_library(${PMZF_CORE_TARGET} STATIC
  src/app/application.cpp
  src/app/cli.cpp
  src/app/version.cpp
  src/pubmed/client.cpp
  src/pubmed/application.cpp
  src/pubmed/cli.cpp
  src/pubmed/http_client.cpp
  src/pubmed/mapper.cpp
  src/pubmed/options.cpp
  src/pubmed/provider.cpp
  src/support/json.cpp
)

target_compile_features(${PMZF_CORE_TARGET} PUBLIC cxx_std_20)

target_compile_definitions(${PMZF_CORE_TARGET}
  PUBLIC
    PMZF_VERSION="${PROJECT_VERSION}"
)

target_include_directories(${PMZF_CORE_TARGET}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PMZF_CORE_TARGET}
  PUBLIC
    spagyrist::spagyrist
    CURL::libcurl
)

target_compile_options(${PMZF_CORE_TARGET}
  PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic>
)

add_executable(${PMZF_TARGET_NAME}
  src/main.cpp
)

target_link_libraries(${PMZF_TARGET_NAME}
  PRIVATE
    ${PMZF_CORE_TARGET}
)

install(
  TARGETS ${PMZF_TARGET_NAME}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
  FILES
    LICENSE
    README.md
    README.ja.md
    THIRD_PARTY_NOTICES.md
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/doc/${PMZF_TARGET_NAME}
)

if(PMZF_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
