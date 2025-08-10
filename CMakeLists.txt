cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

set(PROJECT_NAME SatelliteTracker)
project(${PROJECT_NAME})

add_subdirectory(third_party/SDL)
add_subdirectory(third_party/glad)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(${PROJECT_NAME} 
                src/main.cpp
                src/render/Earth.h
                src/render/Earth.cpp
                src/render/Shaders.h
                src/render/Shaders.cpp
)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_include_directories(${PROJECT_NAME} PRIVATE third_party/glm)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL/CMakeLists.txt")
    message(FATAL_ERROR "SDL3 submodule not found. Run: git submodule update --init --recursive")
endif()


target_link_libraries(${PROJECT_NAME} PRIVATE 
SDL3::SDL3 
glad)

# set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set_target_properties(${PROJECT_NAME} PROPERTIES 
VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
COMMAND ${CMAKE_COMMAND} -E copy_directory 
${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:${PROJECT_NAME}>/res)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT SatelliteTracker)
