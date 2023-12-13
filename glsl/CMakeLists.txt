set(COPYABLE_SHADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/default_frag.glsl"
	"${CMAKE_CURRENT_SOURCE_DIR}/default_vert.glsl"
	"${CMAKE_CURRENT_SOURCE_DIR}/light_frag.glsl"
	"${CMAKE_CURRENT_SOURCE_DIR}/light_vert.glsl"
)
set(DEPS "")

# Copy Shaders
string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}" COPIED_SHADERS "${COPYABLE_SHADERS}")
foreach(SHADER ${COPYABLE_SHADERS})
	string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}" SHADER_DEST "${SHADER}")
	add_custom_command(
		COMMAND "${CMAKE_COMMAND}" -E copy "${SHADER}" "${CMAKE_CURRENT_BINARY_DIR}"
		OUTPUT ${SHADER_DEST}
		DEPENDS "${SHADER}"
		COMMENT "Copying shader"
		VERBATIM
	)
endforeach()
set(DEPS "${DEPS}" "${COPIED_SHADERS}")

add_custom_target(SHADERS DEPENDS "${DEPS}")
add_dependencies(opengl_practice SHADERS)