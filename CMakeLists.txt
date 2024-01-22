cmake_minimum_required(VERSION 3.0.0)
project(blockchain VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

add_executable(blockchain src/main.cpp)
target_sources(blockchain PRIVATE src/utils.cpp)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

find_package (SQLite3)

find_package(CryptoPP)

include_directories(/usr/include)
link_directories(/usr/lib)

target_link_libraries(blockchain sqlite3)

target_link_libraries(blockchain cryptopp)

include(CPack)



