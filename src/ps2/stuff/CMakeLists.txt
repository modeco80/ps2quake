add_library(stuff STATIC
		dummy.cpp
		)

target_include_directories(stuff PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# link libs?