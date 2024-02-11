cmake_minimum_required(VERSION 3.0.0)
project(blockchain VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED On)

add_executable(blockchain src/main.cpp)
target_sources(blockchain PRIVATE src/utils.cpp)

add_subdirectory(include)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

include_directories(/usr/include)
link_directories(/usr/lib)

find_package (SQLite3)

find_package(CryptoPP)

find_package(SqliteOrm)

target_link_libraries(blockchain sqlite3)

target_link_libraries(blockchain cryptopp)

target_link_libraries(blockchain sqlite_orm::sqlite_orm)

include(CPack)



