set(COPYABLE_SHADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/default.frag"
	"${CMAKE_CURRENT_SOURCE_DIR}/default.vert"
	"${CMAKE_CURRENT_SOURCE_DIR}/light.frag"
	"${CMAKE_CURRENT_SOURCE_DIR}/light.vert"
	"${CMAKE_CURRENT_SOURCE_DIR}/skybox.frag"
	"${CMAKE_CURRENT_SOURCE_DIR}/skybox.vert"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/shader_macros.h"
)
set(DEPS "")

target_include_directories(opengl_practice PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Copy Shaders
string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}" COPIED_SHADERS "${COPYABLE_SHADERS}")
foreach(SHADER ${COPYABLE_SHADERS})
	string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}" SHADER_DEST "${SHADER}")
	add_custom_command(
		COMMAND "${CMAKE_COMMAND}" -E copy "${SHADER}" "${SHADER_DEST}"
		OUTPUT ${SHADER_DEST}
		DEPENDS "${SHADER}"
		COMMENT "Copying shader"
		VERBATIM
	)
endforeach()
set(DEPS "${DEPS}" "${COPIED_SHADERS}")

add_custom_target(SHADERS DEPENDS "${DEPS}")
add_dependencies(opengl_practice SHADERS)