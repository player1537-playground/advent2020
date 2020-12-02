cmake_minimum_required(VERSION 3.16.0)
project(Advent)
include(FetchContent)

set(FC_OSPRay OSPRay)
FetchContent_Declare(
	${FC_OSPRay}

	URL https://github.com/ospray/OSPRay/releases/download/v2.4.0/ospray-2.4.0.x86_64.linux.tar.gz

	SOURCE_DIR ospray
)
FetchContent_MakeAvailable(${FC_OSPRay})
set(ospray_LINK_DIR ${ospray_SOURCE_DIR}/lib)
set(ospray_LIBRARIES
	${ospray_LINK_DIR}/libospray.so
)
set(ospray_INCLUDE_DIR ${ospray_SOURCE_DIR}/include)

add_executable(day01 day01.c)
target_link_libraries(day01 ${ospray_LIBRARIES})
target_include_directories(day01 PUBLIC ${ospray_INCLUDE_DIR})
