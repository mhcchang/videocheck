cmake_minimum_required(VERSION 3.9)

set(SDK_NAME VideoSdk)

project(${SDK_NAME} VERSION 1.0.0 LANGUAGES CXX)

#set(CMAKE_DEBUG_POSTFIX d) //-fPIC
set(PLATFORM_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -fPIC -no-pie -std=c++11 -O0 -g -ggdb ")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -fPIC -no-pie -std=c++11 -O3 -Wl,-rpath=./libs -lz")

include_directories( ./
	./rtspclient
	./util
	./ffmpeg/include
	./rapidjson
	./live555/include
)

file(GLOB_RECURSE SDK_PUBLIC_FILES
	"include/live555/include/*.h"
	"live555/include/BasicUsageEnvironment/*.hh"
	"live555/include/groupsock/*.hh"
	"live555/include/liveMedia/*.hh"
	"live555/include/UsageEnvironment/*.hh"
)

set(SDK_SOURCE_FILES
	VideoSdk.cpp
	VideoChannel.cpp
	VideoSdkImpl.cpp
	#rtspclient/live_video.cpp
	#rtspclient/live_video.h
	rtspclient/media_format.h
	rtspclient/RtspInterface.cpp
	rtspclient/RtspInterface.h
	rtspclient/RtspStream.cpp
	rtspclient/RtspStream.h
	util/defs.h
	util/lock.h
	util/MD5.cpp
	util/MD5.hpp
	util/ObjectLists.h
	util/pubutil.cpp
	util/pubutil.h
	util/rwLock.h
	util/sole.hpp
	util/sys_inc.h
	util/sys_log.cpp
	util/sys_log.h
	util/sys_os.cpp
)

# NOTE: link_directories should be placed before add_executable/add_library
link_directories(
	#${PLATFORM_DIR}/3rdparty/baselib/lib
	${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/lib
	${CMAKE_CURRENT_SOURCE_DIR}/live555/libs
)

add_library(${SDK_NAME} SHARED ${SDK_SOURCE_FILES})

target_include_directories(${SDK_NAME}
    PUBLIC 
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${PLATFORM_DIR}/
		${PLATFORM_DIR}/rtspclient
		${PLATFORM_DIR}/util
		${PLATFORM_DIR}/rapidjson
		${PLATFORM_DIR}/ffmpeg/include
		${PLATFORM_DIR}/live555/include
		${PLATFORM_DIR}/live555/BasicUsageEnvironment
		${PLATFORM_DIR}/live555/groupsock
		${PLATFORM_DIR}/live555/liveMedia
		${PLATFORM_DIR}/live555/UsageEnvironment
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/zhcv/include
)

target_sources(${SDK_NAME}
	PRIVATE 
	${SDK_PUBLIC_FILES}
	${SDK_SOURCE_FILES}
)

target_compile_definitions(${SDK_NAME} 
	PRIVATE
	#	CVCAM_API_EXPORT
	#	NO_OPENSSL=1
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_definitions(${SDK_NAME} 
		PRIVATE
			_WIN32_WINNT=0x0601
	)	
	set_source_files_properties(
		#src/ws/ws_session.cpp
		PROPERTIES COMPILE_FLAGS /bigobj
	)
endif()

target_link_libraries(${SDK_NAME}
    PRIVATE
		avcodec
		avdevice
		avfilter
		avformat
		avutil		
		swscale
		swresample
		#debug cvcam_base_d optimized cvcam_base
		#debug zhcvd optimized zhcv
		#stdc++fs
		#rt
)

if (NOT DISABLE_ENCODER)
target_link_libraries(${SDK_NAME}
    PRIVATE
		debug ${PLATFORM_DIR}/live555/libs/debug/libliveMedia.a
		debug ${PLATFORM_DIR}/live555/libs/debug/libgroupsock.a
        debug ${PLATFORM_DIR}/live555/libs/debug/libBasicUsageEnvironment.a
		debug ${PLATFORM_DIR}/live555/libs/debug/libUsageEnvironment.a
		optimized ${PLATFORM_DIR}/live555/libs/release/libliveMedia.a
		optimized ${PLATFORM_DIR}/live555/libs/release/libgroupsock.a
        optimized ${PLATFORM_DIR}/live555/libs/release/libBasicUsageEnvironment.a
		optimized ${PLATFORM_DIR}/live555/libs/release/libUsageEnvironment.a
)
endif()

#option(ENABLE_AUTH	"Enable license check" OFF)
#if (ENABLE_AUTH)
#	target_compile_definitions(${SDK_NAME} 
#		PRIVATE
#			ENABLE_AUTH=1
#	)
#	message("ENABLE_AUTH enabled")
#endif()

#if (ENABLE_AUTH)

#target_include_directories(${SDK_NAME}
#    PRIVATE
#        ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/dongle/include
#)

#if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
#
#	target_link_libraries(${SDK_NAME}
#		PRIVATE
#			debug ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/dongle/lib/VerifyDogD.lib
#			optimized ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/dongle/lib/VerifyDog.lib
#			${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/dongle/lib/U_code.lib
#	)
#	
#else() # linux
#
#	target_link_libraries(${SDK_NAME}
#		PRIVATE
#			${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/dongle/lib/libVerifyDog.so
#			${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/dongle/lib/libucode.so
#			uuid
#	)
#	
#endif()	
#	
#endif() # ENABLE_AUTH
#	


add_custom_command(TARGET ${SDK_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${SDK_NAME}> ${PLATFORM_DIR}/lib
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${SDK_NAME}> ${PLATFORM_DIR}/bin
)


# sdkdemo
add_executable(sdkdemo ../sdkdemo/sdkdemo.cpp)

target_include_directories(sdkdemo
    PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/../VideoSdk
        ${CMAKE_CURRENT_SOURCE_DIR}/../VideoSdk/rapidjson
)

set_target_properties (sdkdemo PROPERTIES DEBUG_POSTFIX d)

target_link_libraries(sdkdemo
	PRIVATE
		${SDK_NAME}
		#debug cvcam_base_d optimized cvcam_base
		pthread
)
