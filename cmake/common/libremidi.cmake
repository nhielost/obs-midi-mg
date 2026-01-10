set(CMAKE_CXX_STANDARD 20)

# For rogue CPack inclusion by readerwriterqueue
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/fake/CPack.cmake" "")
set(LIBREMIDI_MODULE_PATH ${CMAKE_MODULE_PATH})
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_BINARY_DIR}/fake" ${CMAKE_MODULE_PATH})
add_subdirectory(deps/libremidi EXCLUDE_FROM_ALL)
set(CMAKE_MODULE_PATH ${LIBREMIDI_MODULE_PATH})

target_include_directories(${CMAKE_PROJECT_NAME} SYSTEM PRIVATE deps/libremidi/include)
if (WIN32)
  target_compile_options(libremidi PRIVATE -W2)
endif()
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE libremidi)
