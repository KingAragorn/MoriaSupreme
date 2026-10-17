cmake_minimum_required(VERSION 3.10)
project(MoriaSupreme C)

include(GNUInstallDirs)

if(NOT EMSCRIPTEN)
  set(CURSES_NEED_NCURSES TRUE)
  find_package(Curses REQUIRED)
endif()

set(PMORIA_SOURCES
  source/main.c
  source/misc1.c
  source/misc2.c
  source/misc3.c
  source/misc4.c
  source/store1.c
  source/files.c
  source/io.c
  source/create.c
  source/desc.c
  source/generate.c
  source/sets.c
  source/dungeon.c
  source/creature.c
  source/death.c
  source/eat.c
  source/help.c
  source/magic.c
  source/potions.c
  source/prayer.c
  source/save.c
  source/staffs.c
  source/wands.c
  source/scrolls.c
  source/spells.c
  source/wizard.c
  source/store2.c
  source/signals.c
  source/moria1.c
  source/moria2.c
  source/moria3.c
  source/moria4.c
  source/monsters.c
  source/treasure.c
  source/variable.c
  source/rnd.c
  source/recall.c
  source/unix.c
  source/player.c
  source/tables.c
  source/incant.c
  source/townlife.c
)

add_executable(moriasupreme ${PMORIA_SOURCES})
target_include_directories(moriasupreme PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/source
)
target_compile_definitions(moriasupreme PRIVATE unix)
if(EMSCRIPTEN)
  target_sources(moriasupreme PRIVATE web/curses_shim.c web/persist.c)
  target_include_directories(moriasupreme BEFORE PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/web
  )
  target_compile_definitions(moriasupreme PRIVATE
    "MORIA_LIB_SOURCE=\"/files\""
    "MORIA_LIB_DEFAULT=\"/files\""
  )
  # wasm32 time_t is long long. The K&R sources pass long* where a POSIX
  # prototype now says time_t*. NULL and the values are unchanged.
  target_compile_options(moriasupreme PRIVATE -Wno-incompatible-pointer-types)
  set_target_properties(moriasupreme PROPERTIES
    LINK_FLAGS "-sASYNCIFY -sASYNCIFY_STACK_SIZE=1048576 -sSTACK_SIZE=1048576 -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=33554432 -sFORCE_FILESYSTEM=1 -sINVOKE_RUN=0 -sEXIT_RUNTIME=1 -sEXPORTED_FUNCTIONS=_main,_moria_push_key -sEXPORTED_RUNTIME_METHODS=FS,callMain,ccall -lidbfs.js --preload-file ${CMAKE_SOURCE_DIR}/files@/files"
  )
else()
  target_include_directories(moriasupreme PRIVATE ${CURSES_INCLUDE_DIRS})
  target_compile_definitions(moriasupreme PRIVATE
    "MORIA_LIB_SOURCE=\"${CMAKE_SOURCE_DIR}/files\""
    "MORIA_LIB_DEFAULT=\"${CMAKE_INSTALL_FULL_DATADIR}/moriasupreme\""
  )
  target_link_libraries(moriasupreme PRIVATE ${CURSES_LIBRARIES})
  add_custom_target(web
    COMMAND /bin/sh ${CMAKE_SOURCE_DIR}/web/build.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    USES_TERMINAL
  )
endif()

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
  target_compile_options(moriasupreme PRIVATE
    -std=gnu17
    -fcommon
    -Wall
    -Wno-unknown-warning-option
    -Wno-unused-parameter
    -Wno-unused-variable
    -Wno-unused-function
    -Wno-unused-but-set-variable
    -Wno-pointer-sign
    -Wno-format
    -Wno-unused-result
    -Wno-deprecated-declarations
    -Wno-old-style-definition
    -Wno-deprecated-non-prototype
    -Wno-strict-prototypes
    -Wno-missing-braces
    -Wno-implicit-fallthrough
    -Wno-sign-compare
    -Wno-parentheses
    -Wno-char-subscripts
    -Wno-int-to-pointer-cast
    -Wno-pointer-to-int-cast
    -Wno-missing-field-initializers
  )
endif()

install(TARGETS moriasupreme RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES
  files/hours
  files/news
  files/origcmds.hlp
  files/owizcmds.hlp
  files/roglcmds.hlp
  files/rwizcmds.hlp
  files/version.hlp
  files/welcome.hlp
  DESTINATION ${CMAKE_INSTALL_DATADIR}/moriasupreme
)
