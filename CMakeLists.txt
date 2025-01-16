cmake_minimum_required(VERSION 3.20)

project(sdl3-imgui-template LANGUAGES CXX)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_C_COMPILER clang)

# CPM
file(
        DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/latest/download/get_cpm.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
)
include (${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

set (arch x86_64-w64-mingw32)

function (download_and_extract version filename link dest folder)
        file(
                DOWNLOAD
                ${link}
                ${CMAKE_CURRENT_BINARY_DIR}/${filename}.zip
        )

        if(WIN32)
        execute_process(
                COMMAND powershell -Command "Expand-Archive -Path ${CMAKE_CURRENT_BINARY_DIR}/${filename}.zip -DestinationPath ${dest} -Force"
                OUTPUT_QUIET
        )
        execute_process(
                COMMAND powershell -Command "Start-Sleep -Seconds 1"
                OUTPUT_QUIET
        )
        else()
        execute_process(
                COMMAND unzip -o ${CMAKE_CURRENT_BINARY_DIR}/${filename}.zip -d ${dest}
                OUTPUT_QUIET
        )
        execute_process(
                COMMAND sleep 1
                OUTPUT_QUIET
        )
        endif()

        file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/${filename}.zip)
        file(RENAME ${CMAKE_CURRENT_BINARY_DIR}/${folder}-${version} ${CMAKE_CURRENT_BINARY_DIR}/${folder})
endfunction()

if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/SDL3)
        set (sdl_version 3.1.8)
        set (sdl_filename SDL3-devel-${sdl_version}-mingw)
        set (sdl_link https://github.com/libsdl-org/SDL/releases/download/preview-${sdl_version}/${sdl_filename}.zip)
        set(sdl_dest ${CMAKE_CURRENT_BINARY_DIR})
        set(sdl_folder SDL3)
        download_and_extract(${sdl_version} ${sdl_filename} ${sdl_link} ${sdl_dest} ${sdl_folder})
endif()

if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/SDL3_image)
        set (sdl_image_version 3.1.0)
        set (sdl_image_filename SDL3_image-devel-${sdl_image_version}-mingw)
        set (sdl_image_link https://github.com/libsdl-org/SDL_image/releases/download/preview-${sdl_image_version}/${sdl_image_filename}.zip)
        set(sdl_image_dest ${CMAKE_CURRENT_BINARY_DIR})
        set(sdl_image_folder SDL3_image)
        download_and_extract(${sdl_image_version} ${sdl_image_filename} ${sdl_image_link} ${sdl_image_dest} ${sdl_image_folder})
endif()

if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/SDL3_ttf)
        set (sdl_ttf_version 3.1.0)
        set (sdl_ttf_filename SDL3_ttf-devel-${sdl_ttf_version}-mingw)
        set (sdl_ttf_link https://github.com/Coltenus/sdl3-imgui-template/releases/download/${sdl_ttf_version}/${sdl_ttf_filename}.zip)
        set(sdl_ttf_dest ${CMAKE_CURRENT_BINARY_DIR})
        set(sdl_ttf_folder SDL3_ttf)
        download_and_extract(${sdl_ttf_version} ${sdl_ttf_filename} ${sdl_ttf_link} ${sdl_ttf_dest} ${sdl_ttf_folder})
endif()

set(LIBS SDL3::SDL3 SDL3_image::SDL3_image SDL3_ttf::SDL3_ttf)
cmake_policy(SET CMP0169 OLD)

set(CMAKE_FIND_USE_CMAKE_PATH TRUE)
set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_BINARY_DIR}/SDL3/${arch} ${CMAKE_CURRENT_BINARY_DIR}/SDL3_image/${arch} ${CMAKE_CURRENT_BINARY_DIR}/SDL3_ttf/${arch})
set(SDL3_DIR ${CMAKE_CURRENT_BINARY_DIR}/SDL3/${arch}/lib/cmake/SDL3)
set(SDL3_image_DIR ${CMAKE_CURRENT_BINARY_DIR}/SDL3_image/${arch}/lib/cmake/SDL3_image)
set(SDL3_ttf_DIR ${CMAKE_CURRENT_BINARY_DIR}/SDL3_ttf/${arch}/lib/cmake/SDL3_ttf)
find_package(SDL3 REQUIRED CONFIG NO_DEFAULT_PATH)
find_package(SDL3_image REQUIRED CONFIG NO_DEFAULT_PATH)
find_package(SDL3_ttf REQUIRED CONFIG NO_DEFAULT_PATH)

# ImGui
CPMAddPackage(
        NAME imgui
        GITHUB_REPOSITORY ocornut/imgui
        GIT_TAG 0f33d71
        DOWNLOAD_ONLY
)
if (imgui_ADDED)
    add_library(imgui STATIC)
    target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/SDL3/${arch}/include)
    target_link_libraries(imgui PUBLIC SDL3::SDL3)
    target_sources(imgui PRIVATE
            ${imgui_SOURCE_DIR}/imgui.cpp
            ${imgui_SOURCE_DIR}/imgui_draw.cpp
            ${imgui_SOURCE_DIR}/imgui_tables.cpp
            ${imgui_SOURCE_DIR}/imgui_widgets.cpp
            ${imgui_SOURCE_DIR}/imgui_demo.cpp
            # SDL 3
            ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
            # SDL 3 Renderer
            ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp
    )
    target_include_directories(imgui PUBLIC "${imgui_SOURCE_DIR}")
    target_include_directories(imgui PUBLIC "${imgui_SOURCE_DIR}/backends")
    list(APPEND LIBS imgui)
endif ()

add_executable(app) # WIN32 - hide the console like this add_executable(app WIN32)

target_sources(app
PRIVATE
    src/main.cpp
)

target_include_directories(app PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/SDL3/${arch}/include)
target_include_directories(app PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/SDL3_image/${arch}/include)
target_include_directories(app PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/SDL3_ttf/${arch}/include)
target_link_libraries(app PRIVATE ${LIBS})
target_compile_features(app PRIVATE cxx_std_23)
target_compile_definitions(app PRIVATE SDL_MAIN_USE_CALLBACKS)
target_link_options(app PRIVATE -static)

file(COPY assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})