set(COPYABLE_MESHES
	"${CMAKE_CURRENT_SOURCE_DIR}/cube.obj"
)
set(DEPS "")

# Copy Meshes
string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}" COPIED_MESHES "${COPYABLE_MESHES}")
foreach(MESH ${COPYABLE_MESHES})
	string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}" MESH_DEST "${MESH}")
	add_custom_command(
		COMMAND "${CMAKE_COMMAND}" -E copy "${MESH}" "${MESH_DEST}"
		OUTPUT ${MESH_DEST}
		DEPENDS "${MESH}"
		COMMENT "Copying mesh"
		VERBATIM
	)
endforeach()
set(DEPS "${DEPS}" "${COPIED_MESHES}")

add_custom_target(MESHES DEPENDS "${DEPS}")
add_dependencies(opengl_practice MESHES)