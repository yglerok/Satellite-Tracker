cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

set(PROJECT_NAME SatelliteTracker)
project(${PROJECT_NAME})

add_compile_definitions(_USE_MATH_DEFINES)

add_subdirectory(third_party/SDL EXCLUDE_FROM_ALL)
add_subdirectory(third_party/glad EXCLUDE_FROM_ALL)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(${PROJECT_NAME} 
                src/main.cpp
                src/Application.h 
                src/Application.cpp 
                src/Camera.h 
                src/Camera.cpp 
                src/render/Earth.h 
                src/render/Earth.cpp
                src/render/Sun.h 
                src/render/Sun.cpp 
                src/render/Shaders.h
                src/render/Shaders.cpp 
                src/render/stb_image.h
                src/data/Database.h
                src/data/Database.cpp
                src/data/TleParser.h
                src/data/TleParser.cpp
                src/data/DataManager.h 
                src/data/DataManager.cpp
)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_include_directories(${PROJECT_NAME} PRIVATE 
third_party/glm
)

# curl
find_package(CURL REQUIRED)

if(CURL_FOUND)
    message(STATUS "Found cURL: ${CURL_VERSION}")
    target_include_directories(${PROJECT_NAME} PRIVATE ${CURL_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE CURL::libcurl)
else()
    message(FATAL_ERROR "cURL not found! Please install libcurl development package")
endif()


if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL/CMakeLists.txt")
    message(FATAL_ERROR "SDL3 submodule not found. Run: git submodule update --init --recursive")
endif()

set(SQLITE3_INCLUDES
third_party/sqlite/sqlite3.h
)

set(SQLITE3_SOURCES
third_party/sqlite/sqlite3.c
)

add_library(sqlite3 STATIC ${SQLITE3_INCLUDES} ${SQLITE3_SOURCES})

target_include_directories(sqlite3 PUBLIC third_party)
target_include_directories(sqlite3 PUBLIC third_party/sqlite)

target_compile_definitions(sqlite3 PRIVATE
    SQLITE_THREADSAFE=1
    SQLITE_ENABLE_FTS5=1
    SQLITE_ENABLE_JSON1=1
)

# Отключаем предупреждения для SQLite
if(MSVC)
    target_compile_options(sqlite3 PRIVATE /W0)
else()
    target_compile_options(sqlite3 PRIVATE -w)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE 
SDL3::SDL3 
glad 
sqlite3
)

set(IMGUI_INCLUDES 
third_party/imgui/imgui.h 
third_party/imgui/backends/imgui_impl_sdl3.h 
third_party/imgui/backends/imgui_impl_opengl3.h 
)

set(IMGUI_SOURCES 
third_party/imgui/imgui.cpp 
third_party/imgui/backends/imgui_impl_sdl3.cpp 
third_party/imgui/backends/imgui_impl_opengl3.cpp 
third_party/imgui/imgui_demo.cpp 
third_party/imgui/imgui_draw.cpp 
third_party/imgui/imgui_widgets.cpp 
third_party/imgui/imgui_tables.cpp 
)

add_library(ImGui STATIC ${IMGUI_INCLUDES} ${IMGUI_SOURCES})

target_include_directories(ImGui PUBLIC third_party)
target_include_directories(ImGui PUBLIC third_party/imgui)
target_link_libraries(ImGui PRIVATE glad SDL3::SDL3)

target_link_libraries(${PROJECT_NAME} PRIVATE ImGui)

# set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set_target_properties(${PROJECT_NAME} PROPERTIES 
VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
COMMAND ${CMAKE_COMMAND} -E copy_directory 
${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:${PROJECT_NAME}>/res)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT SatelliteTracker)
