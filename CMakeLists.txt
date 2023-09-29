CMAKE_MINIMUM_REQUIRED(VERSION 3.12)
PROJECT(DW1ModelConverter)

SET(CMAKE_CXX_STANDARD 20)

include(cmake/CPM.cmake)

# --- Libraries ---
# boost
#FIND_PACKAGE(Boost 1.75 REQUIRED)
#INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

# libpng
FIND_PACKAGE(PNG REQUIRED)
INCLUDE_DIRECTORIES(${PNG_INCLUDE_DIR})

# tinyxml2
ADD_SUBDIRECTORY("libs/tinyxml2")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Dcimg_use_png")
INCLUDE_DIRECTORIES( "libs" )

#tinygltf
set(TINYGLTF_HEADER_ONLY ON CACHE INTERNAL "" FORCE)
set(TINYGLTF_INSTALL OFF CACHE INTERNAL "" FORCE)

CPMAddPackage(
  NAME tinygltf
  VERSION 2.8.9
  GITHUB_REPOSITORY "syoyo/tinygltf"
)

# --- Configuration ---

add_compile_definitions(cimg_display=0)

# --- Tools ---
SET(SOURCE_FILES ${SOURCE_FILES} "src/main.cpp" "src/TIM.cpp" "src/Animation.cpp" "src/CLUTMap.cpp" 
                                 "src/Model.cpp" "src/GLTF.cpp")

ADD_EXECUTABLE(DW1ModelConverter ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(DW1ModelConverter PUBLIC tinyxml2 PNG::PNG tinygltf)

# Copy .dll dependencies. Extremely dirty!
if(WIN32)
    FIND_FILE(PNG_FILE "bin/libpng16d.dll")
    FIND_FILE(ZLIB_FILE "bin/zlib.dll" PATHS "${ZLIB_INCLUDE_DIR}/..")

    ADD_CUSTOM_COMMAND(TARGET DW1ModelConverter POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${PNG_FILE} $<TARGET_FILE_DIR:DW1ModelConverter>
      COMMAND ${CMAKE_COMMAND} -E copy ${ZLIB_FILE} $<TARGET_FILE_DIR:DW1ModelConverter>
      COMMAND_EXPAND_LISTS
    )
endif()