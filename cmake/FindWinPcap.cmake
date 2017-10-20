# - Try to find WinPcap
#
# Sets the following variables:
#
#  WINPCAP_FOUND - if package found
#  WINPCAP_INCLUDE_DIRS - include directories
#  WINPCAP_LIBRARIES    - libraries
#

if (BUILD_WIN64)
	set (WPDPACKLIB WpdPack/lib/x64)
else()
	set (WPDPACKLIB WpdPack/lib)
endif()

find_path(WINPCAP_INCLUDE_DIR NAMES pcap.h
	PATH_SUFFIXES 
		WpdPack/Include
)

find_library(WINPCAP_LIBRARY_PACKET Packet
	PATH_SUFFIXES 
		${WPDPACKLIB}
)

find_library(WINPCAP_LIBRARY_WPCAP wpcap
	PATH_SUFFIXES 
		${WPDPACKLIB}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WinPcap DEFAULT_MSG 
	WINPCAP_INCLUDE_DIR
	WINPCAP_LIBRARY_PACKET
	WINPCAP_LIBRARY_WPCAP
)

if(WINPCAP_FOUND)
	set(WINPCAP_INCLUDE_DIRS ${WINPCAP_INCLUDE_DIR})
	set(WINPCAP_LIBRARIES ${WINPCAP_LIBRARY_PACKET} ${WINPCAP_LIBRARY_WPCAP})
else()
	set(WINPCAP_INCLUDE_DIRS)
	set(WINPCAP_LIBRARIES)
endif()

mark_as_advanced(WINPCAP_LIBRARIES WINPCAP_INCLUDE_DIRS)