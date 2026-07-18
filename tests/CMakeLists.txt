add_executable(${PMZF_TEST_TARGET}
  test_cli.cpp
  test_main.cpp
  test_pubmed_cli.cpp
  test_pubmed_mapper.cpp
  test_version.cpp
)

target_link_libraries(${PMZF_TEST_TARGET}
  PRIVATE
    ${PMZF_CORE_TARGET}
)

target_compile_features(${PMZF_TEST_TARGET} PRIVATE cxx_std_20)

target_compile_options(${PMZF_TEST_TARGET}
  PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic>
)

add_test(NAME ${PMZF_TEST_TARGET} COMMAND ${PMZF_TEST_TARGET})
