cmake_minimum_required(VERSION 3.28)
project(Server)

set(CMAKE_CXX_STANDARD 17)

# Find the packages and include directories
find_package(OpenSSL REQUIRED)
find_path(GMP_INCLUDE_DIR gmp.h)
find_library(GMP_LIBRARY gmp)
find_library(GMPXX_LIBRARY gmpxx)
find_library(JSONCPP_LIBRARY jsoncpp)

# Manually specify SQLCipher paths
find_path(SQLCIPHER_INCLUDE_DIR NAMES sqlite3.h PATH_SUFFIXES sqlcipher)
find_library(SQLCIPHER_LIBRARY NAMES sqlcipher)

# Include directories
include_directories(${GMP_INCLUDE_DIR})
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(${SQLCIPHER_INCLUDE_DIR})

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
        ${SQLCIPHER_LIBRARY}
        dl  # Required for dynamic linking
        pthread  # Required for threading
)

# Check if SQLCipher paths were found
if (NOT SQLCIPHER_INCLUDE_DIR)
    message(FATAL_ERROR "SQLCipher include directory not found")
endif()

if (NOT SQLCIPHER_LIBRARY)
    message(FATAL_ERROR "SQLCipher library not found")
endif()
