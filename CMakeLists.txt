CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(DW1ModelConverter)

SET(CMAKE_CXX_STANDARD 20)

# --- Libraries ---
# tinyxml2
ADD_SUBDIRECTORY("libs/tinyxml2")

INCLUDE_DIRECTORIES( "libs" )

# boost
#FIND_PACKAGE(Boost 1.75 REQUIRED)
#INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

# --- Tools ---

SET(SOURCE_FILES ${SOURCE_FILES} "src/main.cpp" "src/TIM.cpp" "src/Collada.cpp" "src/Animation.cpp" "src/CLUTMap.cpp" 
                                 "src/Model.cpp")

ADD_EXECUTABLE(DW1ModelConverter ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(DW1ModelConverter tinyxml2)
