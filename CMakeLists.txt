cmake_minimum_required(VERSION 3.20)
project(TaskDevourer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  cpp_httplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG v0.15.3
)
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_Declare(
  hiredis
  GIT_REPOSITORY https://github.com/redis/hiredis.git
  GIT_TAG v1.2.0
)

set(ENABLE_SSL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(cpp_httplib nlohmann_json hiredis)

add_library(hiredis_static STATIC
  ${hiredis_SOURCE_DIR}/alloc.c
  ${hiredis_SOURCE_DIR}/async.c
  ${hiredis_SOURCE_DIR}/hiredis.c
  ${hiredis_SOURCE_DIR}/net.c
  ${hiredis_SOURCE_DIR}/read.c
  ${hiredis_SOURCE_DIR}/sds.c
  ${hiredis_SOURCE_DIR}/sockcompat.c
)
target_include_directories(hiredis_static PUBLIC ${hiredis_SOURCE_DIR})
if(WIN32)
  target_compile_definitions(hiredis_static PRIVATE _CRT_SECURE_NO_WARNINGS)
  target_link_libraries(hiredis_static PUBLIC ws2_32)
endif()

set(SOURCES
  src/main.cpp
  src/gui/MainWindow.cpp
  src/core/TaskService.cpp
  src/storage/JsonStorage.cpp
  src/timer/TimerService.cpp
  src/llm/LlmClient.cpp
  src/redis/DialogLogger.cpp
  src/log/Logger.cpp
  src/config/Config.cpp
  src/util/Paths.cpp
  src/util/StringUtil.cpp
)

add_executable(TaskDevourer WIN32 ${SOURCES})

target_include_directories(TaskDevourer PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${cpp_httplib_SOURCE_DIR}
  ${nlohmann_json_SOURCE_DIR}/include
)

target_compile_definitions(TaskDevourer PRIVATE
  WIN32_LEAN_AND_MEAN
  NOMINMAX
  _CRT_SECURE_NO_WARNINGS
)

target_compile_features(TaskDevourer PRIVATE cxx_std_17)

if(MSVC)
  set_property(TARGET TaskDevourer PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

target_link_libraries(TaskDevourer PRIVATE
  hiredis_static
  ws2_32
  comctl32
  user32
  gdi32
  shell32
)

add_custom_command(TARGET TaskDevourer POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/config.json.example"
    "$<TARGET_FILE_DIR:TaskDevourer>/config.json.example"
)
