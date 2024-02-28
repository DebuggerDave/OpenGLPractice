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
	"${CMAKE_CURRENT_SOURCE_DIR}/other_3d/floor.obj"
	"${CMAKE_CURRENT_SOURCE_DIR}/other_3d/floor.mtl"
	"${CMAKE_CURRENT_SOURCE_DIR}/other_3d/cube.obj"
	"${CMAKE_CURRENT_SOURCE_DIR}/other_3d/grass.obj"
	"${CMAKE_CURRENT_SOURCE_DIR}/other_3d/grass.mtl"
	"${CMAKE_CURRENT_SOURCE_DIR}/backpack/backpack.obj"
	"${CMAKE_CURRENT_SOURCE_DIR}/backpack/backpack.mtl"
)

set(GENERATED_SPRITES
	"${CMAKE_CURRENT_SOURCE_DIR}/generated/grass_top.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/generated/grass_side.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/generated/grass_bottom.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/generated/grass_top_specular.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/generated/grass_side_specular.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/generated/grass_bottom_specular.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/generated/cobblestone_top.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/generated/cobblestone_side.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/generated/cobblestone_bottom.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/generated/cobblestone_top_specular.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/generated/cobblestone_side_specular.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/generated/cobblestone_bottom_specular.png"
)
set(STATIC_SPRITES
	"${CMAKE_CURRENT_SOURCE_DIR}/other/wood.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/other/awesomeface.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/other/container.jpg"
	"${CMAKE_CURRENT_SOURCE_DIR}/other/container2.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/other/container2_specular.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/backpack/diffuse.jpg"
	"${CMAKE_CURRENT_SOURCE_DIR}/backpack/ao.jpg"
	"${CMAKE_CURRENT_SOURCE_DIR}/backpack/specular.jpg"
	"${CMAKE_CURRENT_SOURCE_DIR}/backpack/normal.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/backpack/roughness.jpg"
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
