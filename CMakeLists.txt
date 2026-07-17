cmake_minimum_required(VERSION 3.14)
project(FileCrypto VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8 /W3)
else()
    add_compile_options(-Wall -Wextra)
endif()

add_executable(file-crypto
    src/main.cpp
    src/aes256.cpp
    src/crypto.cpp
    src/config.cpp
    src/i18n.cpp
)

target_include_directories(file-crypto PRIVATE src)

install(TARGETS file-crypto RUNTIME DESTINATION bin)
