cmake_minimum_required(VERSION 3.15)

project("tgbot-ai")

# Вказуємо шлях до vcpkg явно (замініть на свій шлях, якщо він інший)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)
find_package(TgBot REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.hpp")

add_executable(${PROJECT_NAME} "main.cpp" ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE include)
target_precompile_headers(${PROJECT_NAME} PRIVATE "include/pch.hpp")

target_link_libraries(${PROJECT_NAME} PRIVATE TgBot::TgBot)
target_link_libraries(${PROJECT_NAME} PRIVATE CURL::libcurl)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} PRIVATE unofficial::sqlite3::sqlite3)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/config.env"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/config.env"
    COMMENT "Copying config.env to build directory..."
)