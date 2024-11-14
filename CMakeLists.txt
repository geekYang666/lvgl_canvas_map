cmake_minimum_required(VERSION 3.10)
project(lvgl_canvas_map)


set(CMAKE_C_STANDARD 99)#C99 # lvgl officially support C99 and above
set(CMAKE_CXX_STANDARD 17)#C17
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(FREETYPE_LIBRARY "D:/msys64/mingw64/lib/libfreetype.dll.a")
set(FREETYPE_INCLUDE_DIRS "D:/msys64/mingw64/include/freetype2")



SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

find_package(SDL2 REQUIRED SDL2)
find_package(Freetype REQUIRED Freetype)

add_compile_definitions($<$<BOOL:${LV_USE_DRAW_SDL}>:LV_USE_DRAW_SDL=1>)
add_compile_definitions($<$<BOOL:${LV_USE_LIBPNG}>:LV_USE_LIBPNG=1>)
add_compile_definitions($<$<BOOL:${LV_USE_LIBJPEG_TURBO}>:LV_USE_LIBJPEG_TURBO=1>)
add_compile_definitions($<$<BOOL:${LV_USE_FFMPEG}>:LV_USE_FFMPEG=1>)

include_directories(${FREETYPE_INCLUDE_DIRS})
include_directories(
        App
        lib
        HAL
        CanvasMap
)

file(GLOB_RECURSE APP_SRC
        "App/*.*"
        "lib/*.*"
        "HAL/*.*"
        "CanvasMap/*.*"
)

add_subdirectory(lvgl)
target_include_directories(lvgl PUBLIC ${PROJECT_SOURCE_DIR} ${SDL2_INCLUDE_DIRS})

add_executable(main main.cpp ${APP_SRC})

target_compile_definitions(main PRIVATE LV_CONF_INCLUDE_SIMPLE)
target_link_libraries(main lvgl lvgl::thorvg ${SDL2_LIBRARIES} Freetype m pthread)
add_custom_target (run COMMAND ${EXECUTABLE_OUTPUT_PATH}/main DEPENDS main)

