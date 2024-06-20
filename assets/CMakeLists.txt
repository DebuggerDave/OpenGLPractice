function(COPY_TO_BINARY DATA DEPS)
	foreach(DATUM ${DATA})
		string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}" DATUM_DEST "${DATUM}")
		add_custom_command(
			COMMAND "${CMAKE_COMMAND}" -E copy "${DATUM}" "${DATUM_DEST}"
			OUTPUT ${DATUM_DEST}
			DEPENDS "${DATUM}"
			COMMENT "Copying ${DATUM}"
			VERBATIM
		)
		set(LOCAL_DEPS "${LOCAL_DEPS}" "${DATUM_DEST}")
	endforeach()
	set(DEPS "${DEPS}" "${LOCAL_DEPS}" PARENT_SCOPE)
endfunction()

set(3D_OBJECTS
	"${CMAKE_CURRENT_SOURCE_DIR}/other_3d/cube.obj"
	"${CMAKE_CURRENT_SOURCE_DIR}/other_3d/grass.obj"
	"${CMAKE_CURRENT_SOURCE_DIR}/other_3d/grass.mtl"
	"${CMAKE_CURRENT_SOURCE_DIR}/other_3d/dirt.obj"
	"${CMAKE_CURRENT_SOURCE_DIR}/other_3d/dirt.mtl"
	"${CMAKE_CURRENT_SOURCE_DIR}/other_3d/cobblestone.obj"
	"${CMAKE_CURRENT_SOURCE_DIR}/other_3d/cobblestone.mtl"
)

set(GENERATED_SPRITES
	"${CMAKE_CURRENT_SOURCE_DIR}/generated/grass.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/generated/grass_specular.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/generated/dirt.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/generated/dirt_specular.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/generated/cobblestone.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/generated/cobblestone_specular.png"
)
set(STATIC_SPRITES
	"${CMAKE_CURRENT_SOURCE_DIR}/skybox/front.jpg"
	"${CMAKE_CURRENT_SOURCE_DIR}/skybox/back.jpg"
	"${CMAKE_CURRENT_SOURCE_DIR}/skybox/left.jpg"
	"${CMAKE_CURRENT_SOURCE_DIR}/skybox/right.jpg"
	"${CMAKE_CURRENT_SOURCE_DIR}/skybox/top.jpg"
	"${CMAKE_CURRENT_SOURCE_DIR}/skybox/bottom.jpg"
)
set(DEPS "")


# Generate Sprites
set(ASEPRITE $ENV{ASEPRITE})
if ("${ASEPRITE}" STREQUAL "")
	message(WARNING "Aseprite executable not provided, cannot generate sprites")
else()
	add_custom_command(
		COMMAND "${ASEPRITE}" -b "${CMAKE_CURRENT_SOURCE_DIR}/texture.aseprite" --split-slices --save-as ./generated/{slice}.png
		OUTPUT ${GENERATED_SPRITES}
		DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/texture.aseprite"
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
		COMMENT "Generating sprites"
		VERBATIM
	)
endif()

COPY_TO_BINARY("${3D_OBJECTS}" "${DEPS}")
COPY_TO_BINARY("${GENERATED_SPRITES}" "${DEPS}")
COPY_TO_BINARY("${STATIC_SPRITES}" "${DEPS}")

add_custom_target(SPRITES DEPENDS "${DEPS}")
add_dependencies(opengl_practice SPRITES)
