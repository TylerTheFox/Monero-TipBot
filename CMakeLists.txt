cmake_minimum_required(VERSION 3.2)
project (TIPBOT)
set (CMAKE_CXX_STANDARD 14)

file(GLOB_RECURSE sources "src/*.cpp")
  
include_directories(
	extern/sleepy-discord/include
	extern/poco-1.9.0/Foundation/include
	extern/poco-1.9.0/JSON/include
	extern/poco-1.9.0/Net/include
	extern/poco-1.9.0/Util/include
	extern/cereal-1.2.2/include 
    extern/ChaiScript-6.1.0/include 
    )

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release) 
endif()

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPOCO_NO_UNWINDOWS /Zi")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
    set(CMAKE_CXX_FLAGS_MINSIZEREL, "-Os -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE, "-O4 -DNDEBUG")
endif()
	
if (MSVC) 
  link_directories( 
	extern/sleepy-discord/lib/win32
	extern/poco-1.9.0/lib/win32
    )

	add_executable(TIPBOT ${sources})

	target_link_libraries(TIPBOT
		debug "PocoJSONd.lib"			optimized	"PocoJSON.lib"
		debug "PocoUtild.lib"			optimized	"PocoUtil.lib"
		debug "PocoNetd.lib"			optimized	"PocoNet.lib"	
		debug "PocoFoundationd.lib"		optimized	"PocoFoundation.lib"
		debug "sleepy_discord_d.lib"	optimized	"sleepy_discord.lib"
		debug "libcurl_a.lib"			optimized	"libcurl_a.lib"
		debug "libcryptoMTd"			optimized	"libcryptoMT"
		debug "libsslMTd.lib"			optimized	"libsslMT.lib"
	)
else()
  find_package(Threads REQUIRED)
  link_directories(
	extern/sleepy-discord/lib/linux
	extern/poco-1.9.0/lib/linux
    )

	add_executable(TIPBOT ${sources})

	target_link_libraries(TIPBOT
		debug "PocoJSONd"		optimized	"PocoJSON"
		debug "PocoUtild"		optimized	"PocoUtil"
		debug "PocoNetd"		optimized	"PocoNet"
		debug "PocoFoundationd"	optimized	"PocoFoundation"
		debug "sleepy_discord"	optimized	"sleepy_discord"
		debug "pthread"			optimized	"pthread"
		debug "curl"			optimized	"curl"
		debug "ssl"				optimized	"ssl"
		debug "crypto"			optimized	"crypto")

endif()
