set(project_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/chatserver.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/packethandler.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/session.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin")

# chat server
add_executable(${PROJECT_NAME} ${project_SRCS})
target_link_libraries(
	${PROJECT_NAME}
	${APR_LIBRARY}
	${LZ4_LIBRARY}
	${JSON_LIBRARY}
	${LOG4CPP_LIBRARY}
)
