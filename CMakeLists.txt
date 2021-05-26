project(AoD)

add_definitions(-DMBCS)
add_definitions(-D_MBCS)

add_subdirectory(xHacking)

file(GLOB TMP_SOURCES_1 src/*.cpp)
file(GLOB TMP_SOURCES_2 src/*.c)
file(GLOB TMP_INCLUDES_1 src/*.hpp)
file(GLOB TMP_INCLUDES_2 src/*.h)

set(SRC 
	${TMP_SOURCES_1}
	${TMP_SOURCES_2}
	${TMP_INCLUDES_1}
	${TMP_INCLUDES_2}
)

include_directories(xHacking/include)
add_library(AoD SHARED ${SRC})
target_link_libraries(AoD xHacking)