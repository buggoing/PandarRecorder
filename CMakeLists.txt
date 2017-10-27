cmake_minimum_required (VERSION 2.8) 

project (pandar_recorder)

if(UNIX)
	set (CMAKE_CXX_FLAGS "-std=c++11")
endif()

set (CMAKE_INCLUDE_PATH "${PROJECT_SOURCE_DIR}")
set (CMAKE_LIBRARY_PATH "${PROJECT_SOURCE_DIR}")
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set (THREADS_USE_PTHREADS_WIN32 true)
#set (BUILD_WIN64 true)

if(WIN32)
	find_package(WinPcap REQUIRED)
	find_package(Threads REQUIRED)
endif() 

find_package (Qt4 REQUIRED)


include_directories (
	.
	${CMAKE_BINARY_DIR}	
	${WINPCAP_INCLUDE_DIR}
	${THREADS_PTHREADS_INCLUDE_DIR}
)

INCLUDE         (${QT_USE_FILE})
ADD_DEFINITIONS (${QT_DEFINITIONS})

add_library(pandar_source pandarSource.cpp)

if (WIN32)
	TARGET_LINK_LIBRARIES(pandar_source ${WINPCAP_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} )
else()
	TARGET_LINK_LIBRARIES(pandar_source pthread pcap )
endif()

ADD_EXECUTABLE  (pandar_recorder main.cpp)

TARGET_LINK_LIBRARIES (pandar_recorder ${QT_LIBRARIES} pandar_source)

# generate proper GUI program on specified platform
if(WIN32) # Check if we are on Windows
	if(MSVC) # Check if we are using the Visual Studio compiler
		set_target_properties(${PROJECT_NAME} PROPERTIES
			WIN32_EXECUTABLE YES
			LINK_FLAGS "/ENTRY:mainCRTStartup"
		)
	elseif(CMAKE_COMPILER_IS_GNUCXX)
			# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows") # Not tested
	else()
		message(SEND_ERROR "You are using an unsupported Windows compiler! (Not MSVC or GCC)")
	endif(MSVC)
elseif(APPLE)
	set_target_properties(${PROJECT_NAME} PROPERTIES
			MACOSX_BUNDLE YES
	)
elseif(UNIX)
	# Nothing special required
else()
	message(SEND_ERROR "You are on an unsupported platform! (Not Win32, Mac OS X or Unix)")
endif(WIN32)