cmake_minimum_required(VERSION 3.28)
project(Server)

set(CMAKE_CXX_STANDARD 17)

# Find the packages and include directories
find_package(OpenSSL REQUIRED)
find_path(GMP_INCLUDE_DIR gmp.h)
find_library(GMP_LIBRARY gmp)
find_library(GMPXX_LIBRARY gmpxx)
find_library(JSONCPP_LIBRARY jsoncpp)

# Manually specify SQLite3 paths
set(SQLITE3_INCLUDE_DIR "/usr/include")  # Replace with the actual path
set(SQLITE3_LIBRARY "/usr/lib64/libsqlite3.so.0.8.6")  # Replace with the actual path

# Include directories
include_directories(${GMP_INCLUDE_DIR})
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(${SQLITE3_INCLUDE_DIR})

# Add executable
add_executable(Server
        main.cpp
        DB/DB.cpp
        Requests/Requests.cpp
        Server/Server.cpp
        DB/Encryption/Encryption.cpp
        DB/Operations/Admin.cpp
        DB/Operations/Client.cpp
)

# Link libraries
target_link_libraries(Server
        ${GMP_LIBRARY}
        ${GMPXX_LIBRARY}
        ${OPENSSL_LIBRARIES}
        ${JSONCPP_LIBRARY}
        ${SQLITE3_LIBRARY}
)
