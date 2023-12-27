set(GENERATED_SPRITES
	"${CMAKE_CURRENT_BINARY_DIR}/grass_top.png"
	"${CMAKE_CURRENT_BINARY_DIR}/grass_side.png"
	"${CMAKE_CURRENT_BINARY_DIR}/grass_bottom.png"
	"${CMAKE_CURRENT_BINARY_DIR}/grass_top_specular.png"
	"${CMAKE_CURRENT_BINARY_DIR}/grass_side_specular.png"
	"${CMAKE_CURRENT_BINARY_DIR}/grass_bottom_specular.png"
	"${CMAKE_CURRENT_BINARY_DIR}/cobblestone_top.png"
	"${CMAKE_CURRENT_BINARY_DIR}/cobblestone_side.png"
	"${CMAKE_CURRENT_BINARY_DIR}/cobblestone_bottom.png"
	"${CMAKE_CURRENT_BINARY_DIR}/cobblestone_top_specular.png"
	"${CMAKE_CURRENT_BINARY_DIR}/cobblestone_side_specular.png"
	"${CMAKE_CURRENT_BINARY_DIR}/cobblestone_bottom_specular.png"
)
set(COPYABLE_SPRITES
	"${CMAKE_CURRENT_SOURCE_DIR}/awesomeface.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/container.jpg"
)
set(DEPS "")

# Copy Sprites
string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}" COPIED_SPRITES "${COPYABLE_SPRITES}")
foreach(SPRITE ${COPYABLE_SPRITES})
	string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}" SPRITE_DEST "${SPRITE}")
	add_custom_command(
		COMMAND "${CMAKE_COMMAND}" -E copy "${SPRITE}" "${SPRITE_DEST}"
		OUTPUT ${SPRITE_DEST}
		DEPENDS "${SPRITE}"
		COMMENT "Copying sprite"
		VERBATIM
	)
endforeach()
set(DEPS "${DEPS}" "${COPIED_SPRITES}")

# Generate Sprites
set(ASEPRITE $ENV{ASEPRITE})
if ("${ASEPRITE}" STREQUAL "")
	message(FATAL_ERROR "Aseprite executable not provided, cannot generate sprites")
else()
	add_custom_command(
		COMMAND "${ASEPRITE}" -b "${CMAKE_CURRENT_SOURCE_DIR}/texture.aseprite" --split-slices --save-as {slice}.png
		OUTPUT ${GENERATED_SPRITES}
		DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/texture.aseprite"
		COMMENT "Generating sprites"
		VERBATIM
	)
	set(DEPS "${DEPS}" "${GENERATED_SPRITES}")
endif()

add_custom_target(SPRITES DEPENDS "${DEPS}")
add_dependencies(opengl_practice SPRITES)
