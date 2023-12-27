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


set(GENERATED_SPRITES
	"${CMAKE_CURRENT_SOURCE_DIR}/grass_top.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/grass_side.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/grass_bottom.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/grass_top_specular.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/grass_side_specular.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/grass_bottom_specular.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/cobblestone_top.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/cobblestone_side.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/cobblestone_bottom.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/cobblestone_top_specular.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/cobblestone_side_specular.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/cobblestone_bottom_specular.png"
)
set(STATIC_SPRITES
	"${CMAKE_CURRENT_SOURCE_DIR}/awesomeface.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/container.jpg"
)
set(DEPS "")


# Generate Sprites
set(ASEPRITE $ENV{ASEPRITE})
if ("${ASEPRITE}" STREQUAL "")
	message(WARNING "Aseprite executable not provided, cannot generate sprites")
else()
	add_custom_command(
		COMMAND "${ASEPRITE}" -b "${CMAKE_CURRENT_SOURCE_DIR}/texture.aseprite" --split-slices --save-as {slice}.png
		OUTPUT ${GENERATED_SPRITES}
		DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/texture.aseprite"
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
		COMMENT "Generating sprites"
		VERBATIM
	)
endif()

COPY_TO_BINARY("${GENERATED_SPRITES}" "${DEPS}")
COPY_TO_BINARY("${STATIC_SPRITES}" "${DEPS}")

add_custom_target(SPRITES DEPENDS "${DEPS}")
add_dependencies(opengl_practice SPRITES)
