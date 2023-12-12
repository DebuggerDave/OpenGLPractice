set(GENERATED_SPRITES
	"${CMAKE_CURRENT_BINARY_DIR}/dirt_top.png"
	"${CMAKE_CURRENT_BINARY_DIR}/dirt_side.png"
	"${CMAKE_CURRENT_BINARY_DIR}/dirt_bottom.png"
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
		COMMAND "${CMAKE_COMMAND}" -E copy "${SPRITE}" "${CMAKE_CURRENT_BINARY_DIR}"
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
