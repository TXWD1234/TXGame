function(tx_add_txlib)
	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../TXLib/CMakeLists.txt")
	    message(STATUS "Using local TXLib")
	    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../TXLib" "${CMAKE_BINARY_DIR}/libs/TXLib")
		set(TXLib "${CMAKE_CURRENT_SOURCE_DIR}/../TXLib" PARENT_SCOPE)
	else()
	    message(STATUS "Fetching TXLib from GitHub")

	    include(FetchContent)

	    FetchContent_Declare(
	        TXLib # declared name (you define it)
	        GIT_REPOSITORY https://github.com/TXWD1234/TXLib.git # url of the targeting repo
	        GIT_TAG main # set the version of the targeting repo
	    )

	    FetchContent_MakeAvailable(TXLib) # git clone + add_subdirector
		set(TXLib "${txlib_SOURCE_DIR}" PARENT_SCOPE)
	endif()
endfunction()

function(tx_add_txlib_version libVersion)
	if(EXISTS "${CMAKE_SOURCE_DIR}/../TXLib/CMakeLists.txt")
	    message(STATUS "Using local TXLib")
	    add_subdirectory(../libs/TXLib "${CMAKE_BINARY_DIR}/libs/TXLib")
		set(TXLib "${CMAKE_SOURCE_DIR}/../libs/TXLib" PARENT_SCOPE)
	else()
	    message(STATUS "Fetching TXLib from GitHub")

	    include(FetchContent)

	    FetchContent_Declare(
	        TXLib # declared name (you define it)
	        GIT_REPOSITORY https://github.com/TXWD1234/TXLib.git # url of the targeting repo
	        GIT_TAG ${libVersion} # set the version of the targeting repo
	    )

	    FetchContent_MakeAvailable(TXLib) # git clone + add_subdirector
		set(TXLib "${txlib_SOURCE_DIR}" PARENT_SCOPE)
	endif()
endfunction()
