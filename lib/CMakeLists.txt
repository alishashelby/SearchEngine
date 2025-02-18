add_executable(index index/main.cpp index/index.cpp trie/trie.cpp)
add_executable(search search/main.cpp search/search.cpp search/parsing.cpp trie/trie.cpp)

add_executable(test lib/tests/tests.cpp index/index.cpp trie/trie.cpp search/search.cpp search/parsing.cpp trie/trie.cpp)
target_link_libraries(test PRIVATE gtest_main)
target_include_directories(test PRIVATE ${googletest_SOURCE_DIR}/googletest/include)
include(GoogleTest)
gtest_discover_tests(test)