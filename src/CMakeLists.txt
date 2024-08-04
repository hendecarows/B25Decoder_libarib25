# cmake version
cmake_minimum_required(VERSION 3.5)

# C++
enable_language(CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# no prefix "lib" library
set(CMAKE_SHARED_LIBRARY_PREFIX_CXX "")

# library file
add_library(
	${PROJECT_NAME}
	SHARED
	B25Decoder.cpp
)

add_executable(
	b25test
	b25test.cpp
)

# compiler option
set(COMPILER_FLAGS_GNU -Wall -Wextra -pedantic -Wno-psabi -Wno-unused-parameter -Wno-ignored-qualifiers)
set(COMPILER_FLAGS_CLANG -Wall -Wextra -Wno-unused-parameter -Wno-ignored-qualifiers)

target_compile_options(
	${PROJECT_NAME}
	PRIVATE	$<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:GNU>>:${COMPILER_FLAGS_GNU}>
	PRIVATE $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU>>:${COMPILER_FLAGS_GNU}>
)

# include directory
target_include_directories(
	${PROJECT_NAME}
	PRIVATE /usr/local/include/arib25
)

# linker option
target_link_libraries(
	${PROJECT_NAME}
	PRIVATE arib25
)

# install
install(TARGETS ${PROJECT_NAME} DESTINATION lib/edcb)
