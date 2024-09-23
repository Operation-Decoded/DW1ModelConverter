cmake_minimum_required(VERSION 3.12)
project(DW1ModelConverter VERSION 1.2.0)

include(cmake/CPM.cmake)

# --- Libraries ---

# disable deprecation warning for libraries
set(CMAKE_WARN_DEPRECATED FALSE CACHE BOOL "" FORCE)

#tinygltf
set(TINYGLTF_HEADER_ONLY ON CACHE INTERNAL "" FORCE)
set(TINYGLTF_INSTALL OFF CACHE INTERNAL "" FORCE)
set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF CACHE INTERNAL "" FORCE)

CPMAddPackage(
  NAME tinygltf
  VERSION 2.8.17
  GITHUB_REPOSITORY "syoyo/tinygltf"
)

# zlib
set(SKIP_INSTALL_ALL ON)

CPMAddPackage(
  NAME zlib
  VERSION 1.3
  GITHUB_REPOSITORY "madler/zlib"
)

# pass zlib information to libpng
set(ZLIB_LIBRARIES zlibstatic)
set(ZLIB_INCLUDE_DIRS ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})

# libpng
set(PNG_TESTS OFF)
set(PNG_EXECUTABLES OFF)
set(PNG_SHARED OFF)
set(PNG_FRAMEWORK OFF)
set(PNG_BUILD_ZLIB ON)

CPMAddPackage(
  NAME libpng
  VERSION 1.6.40
  GITHUB_REPOSITORY "glennrp/libpng"
)

# cimg
CPMAddPackage(
  NAME cimg
  VERSION .3.3.0
  GITHUB_REPOSITORY "GreycLab/CImg"
)

# nlohmann_json
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_MultipleHeaders FALSE CACHE BOOL "" FORCE)

CPMAddPackage(
  NAME nlohmann_json
  VERSION 3.11.3
  GITHUB_REPOSITORY "nlohmann/json"
)

# re-enable deprecation warning
set(CMAKE_WARN_DEPRECATED TRUE CACHE BOOL "" FORCE)

# --- Target ---
set(SOURCE_FILES ${SOURCE_FILES} "src/main.cpp" "src/TIM.cpp" "src/Animation.cpp" "src/CLUTMap.cpp" 
                                 "src/Model.cpp" "src/GLTF.cpp" "src/MAP.cpp" "src/GameData.cpp")

add_executable(DW1ModelConverter ${SOURCE_FILES})

set_target_properties(DW1ModelConverter PROPERTIES CXX_STANDARD 20)
target_include_directories(DW1ModelConverter PRIVATE ${cimg_SOURCE_DIR} ${libpng_SOURCE_DIR} ${libpng_BINARY_DIR} nlohmann_json::nlohmann_json)
target_link_libraries(DW1ModelConverter PRIVATE png_static tinygltf nlohmann_json::nlohmann_json)

target_compile_definitions(DW1ModelConverter PRIVATE cimg_display=0)
target_compile_definitions(DW1ModelConverter PRIVATE cimg_use_png)
target_compile_definitions(DW1ModelConverter PRIVATE PROJECT_NAME="${PROJECT_NAME}")
target_compile_definitions(DW1ModelConverter PRIVATE PROJECT_VERSION="v${PROJECT_VERSION}")
target_compile_definitions(DW1ModelConverter PRIVATE PROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR})
target_compile_definitions(DW1ModelConverter PRIVATE PROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR})
target_compile_definitions(DW1ModelConverter PRIVATE PROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH})

install(TARGETS DW1ModelConverter)