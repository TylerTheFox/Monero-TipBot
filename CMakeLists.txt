cmake_minimum_required(VERSION 3.2)
project (TIPBOT)
set (CMAKE_CXX_STANDARD 11)

file(GLOB_RECURSE sources "src/*.cpp")

include_directories(
	extern/sleepy-discord/include
	extern/poco-1.9.0/Foundation/include
	extern/poco-1.9.0/JSON/include
	extern/poco-1.9.0/Net/include
	extern/poco-1.9.0/Util/include
    )

if (MSVC)
  link_directories(
	extern/sleepy-discord/lib/win32
	extern/poco-1.9.0/lib/win32
    )

	add_executable(TIPBOT ${sources})

	target_link_libraries(TIPBOT
		debug "PocoJSON.lib"			optimized	"PocoJSON.lib"
		debug "PocoUtil.lib"			optimized	"PocoUtil.lib"
		debug "PocoNet.lib"				optimized	"PocoNet.lib"	
		debug "PocoFoundation.lib"		optimized	"PocoFoundation.lib"
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
	link_libraries(			      			"libPocoJSON.a"
											"libPocoUtil.a"
											"libPocoNet.a"
											"libPocoFoundation.a"
											"libsleepy_discord.a"
											"pthread"
											"curl"
											"ssl"
											"crypto")
endif()