cmake_minimum_required(VERSION 3.14)
set(PROJECT_NAME "audio_service_smtc")
project(${PROJECT_NAME} LANGUAGES CXX)

# This value is used when generating builds using this plugin, so it must
# not be changed
set(PLUGIN_NAME "audio_service_smtc_plugin")

# Explicitly opt in to modern CMake behaviors to avoid warnings
cmake_policy(VERSION 3.14...3.25)

# Define build configuration option.
get_property(IS_MULTICONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(IS_MULTICONFIG)
  set(CMAKE_CONFIGURATION_TYPES "Debug;Profile;Release"
    CACHE STRING "" FORCE)
else()
  if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Debug" CACHE
      STRING "Flutter build mode" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
      "Debug" "Profile" "Release")
  endif()
endif()

# Use Unicode for all projects.
add_definitions(-DUNICODE -D_UNICODE)

# Compilation settings that should be applied to most targets.
function(APPLY_STANDARD_SETTINGS TARGET)
  target_compile_features(${TARGET} PUBLIC cxx_std_17)
  target_compile_options(${TARGET} PRIVATE /W4 /WX /wd"4100")
  target_compile_options(${TARGET} PRIVATE /EHsc)
  target_compile_definitions(${TARGET} PRIVATE "_HAS_EXCEPTIONS=0")
  target_compile_definitions(${TARGET} PRIVATE "$<$<CONFIG:Debug>:_DEBUG>")
endfunction()

# Plugin library
set(PLUGIN_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/smtc_windows")

add_library(${PLUGIN_NAME} SHARED
  "${PLUGIN_SOURCE_DIR}/smtc_windows.cpp"
  "${PLUGIN_SOURCE_DIR}/smtc_windows.h"
  "${PLUGIN_SOURCE_DIR}/audio_service_smtc_plugin.cpp"
  "${PLUGIN_SOURCE_DIR}/audio_service_smtc_plugin.h"
)

apply_standard_settings(${PLUGIN_NAME})

# Find Windows SDK installation path
# First, check the standard location
set(WINDOWS_SDK_LOCATION "C:/Program Files (x86)/Windows Kits/10")

# Function to find first available SDK version
function(find_winrt_sdk_version BASE_PATH OUT_VERSION)
  file(GLOB SDK_VERSIONS "${BASE_PATH}/Include/*")
  foreach(VERSION_DIR ${SDK_VERSIONS})
    if(IS_DIRECTORY "${VERSION_DIR}")
      get_filename_component(VERSION_NUMBER "${VERSION_DIR}" NAME)
      if(EXISTS "${BASE_PATH}/Include/${VERSION_NUMBER}/cppwinrt")
        set(${OUT_VERSION} "${VERSION_NUMBER}" PARENT_SCOPE)
        return()
      endif()
    endif()
  endforeach()
  set(${OUT_VERSION} "" PARENT_SCOPE)
endfunction()

# Find the SDK version
find_winrt_sdk_version(${WINDOWS_SDK_LOCATION} WINRT_SDK_VERSION)

if(WINRT_SDK_VERSION STREQUAL "")
  message(FATAL_ERROR "Could not find a valid Windows SDK. Please ensure the Windows SDK is installed.")
endif()

message(STATUS "Using Windows SDK version: ${WINRT_SDK_VERSION}")

set(WINDOWS_SDK_PATH_UNIONMETADATA "${WINDOWS_SDK_LOCATION}/UnionMetadata/${WINRT_SDK_VERSION}")
set(WINDOWS_SDK_PATH_REFERENCES "${WINDOWS_SDK_LOCATION}/References/${WINRT_SDK_VERSION}")

# Add WinRT support
target_compile_options(${PLUGIN_NAME} PRIVATE
  /await
  /std:c++17
  /bigobj
  /ZW
  /EHsc
)

# Add the Windows SDK paths
target_compile_options(${PLUGIN_NAME} PRIVATE
  "/AI${WINDOWS_SDK_PATH_UNIONMETADATA}"
  "/AI${WINDOWS_SDK_PATH_REFERENCES}"
)

# Add include paths for WinRT
target_include_directories(${PLUGIN_NAME} PRIVATE
  "${WINDOWS_SDK_LOCATION}/Include/${WINRT_SDK_VERSION}/cppwinrt"
  "${WINDOWS_SDK_LOCATION}/Include/${WINRT_SDK_VERSION}/winrt"
  "${WINDOWS_SDK_LOCATION}/Include/${WINRT_SDK_VERSION}/shared"
  "${WINDOWS_SDK_LOCATION}/Include/${WINRT_SDK_VERSION}/um"
)

# Include needed libraries
target_link_libraries(${PLUGIN_NAME} PRIVATE
  flutter
  flutter_wrapper_plugin
  WindowsApp.lib
)

# List of absolute paths to libraries that should be bundled with the plugin.
set(audio_service_smtc_bundled_libraries
  ""
  PARENT_SCOPE
)
