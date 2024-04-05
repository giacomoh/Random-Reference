# Minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Project name
project(RefPickerProject)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Compiler flags
add_compile_options(/Zc:__cplusplus)
add_compile_options(/permissive-)

# Definitions
add_definitions(-DNOMINMAX)

# Statically link the Visual C++ runtime
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)

# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

# Set the Qt6 directory
set(CMAKE_PREFIX_PATH "C:/Qt/6.6.3/msvc2019_64/lib/cmake")
list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.6.3/msvc2019_64/lib/cmake/Qt6CoreTools")
list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.6.3/msvc2019_64/lib/cmake/Qt6GuiTools")

# Set the OpenCV directory
set(OpenCV_DIR "C:/Codice/opencv/build")
list(APPEND CMAKE_PREFIX_PATH "C:/Codice/opencv/build")
list(APPEND CMAKE_PREFIX_PATH "C:/Codice/opencv/build/x64/vc16/lib")

# Set the LittleCMS include and library directories
set(LCMS2_INCLUDE_DIRS "C:/Codice/littleCMS/Little-CMS-master//include")
set(LCMS2_LIBRARIES "C:/Codice/littleCMS/Little-CMS-master//bin/lcms2.lib")  # Change .dll back to .lib

# Set the ImageMagick include directories
set(ImageMagick_INCLUDE_DIRS "C:/Codice/ImageMagick-7.1.1-Q16-HDRI/include/MagickWand")
list(APPEND ImageMagick_INCLUDE_DIRS "C:/Codice/ImageMagick-7.1.1-Q16-HDRI/include/MagickCore")
list(APPEND ImageMagick_INCLUDE_DIRS "C:/Codice/ImageMagick-7.1.1-Q16-HDRI/include")
set(ImageMagick_LIBRARIES "C:/Codice/ImageMagick-7.1.1-Q16-HDRI/lib/CORE_RL_MagickWand_.lib")  # Change .dll back to .lib

# Find the necessary packages
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
find_package(OpenCV REQUIRED)

# Get the location of the Qt binaries
get_target_property(QT_BINARY_DIR Qt6::Core LOCATION)
get_filename_component(QT_BINARY_DIR "${QT_BINARY_DIR}" DIRECTORY)

# Add the executable
add_executable(RefPickerProject WIN32 refPC.cpp mainwindow.cpp)

# Include the OpenCV, LCMS2, and ImageMagick directories
include_directories(${OpenCV_INCLUDE_DIRS} ${LCMS2_INCLUDE_DIRS} ${ImageMagick_INCLUDE_DIRS})  # Add ${ImageMagick_INCLUDE_DIRS}

# Link the necessary libraries
target_link_libraries(RefPickerProject Qt6::Core Qt6::Gui Qt6::Widgets ${OpenCV_LIBS} ${LCMS2_LIBRARIES} ${ImageMagick_LIBRARIES})

# Define the directory where the DLLs will be copied
set(DLL_DIR $<TARGET_FILE_DIR:RefPickerProject>/dlls)

# Custom command to copy the DLLs
add_custom_command(TARGET RefPickerProject POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DLL_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<$<CONFIG:Debug>:"C:/Codice/littleCMS/Little-CMS-master//bin/lcms2.dll">
        $<$<CONFIG:Debug>:"C:/Codice/ImageMagick-7.1.1-Q16-HDRI/CORE_RL_MagickWand_.dll">
        $<$<CONFIG:Debug>:"${QT_BINARY_DIR}/Qt6Cored.dll">
        $<$<CONFIG:Debug>:"${QT_BINARY_DIR}/Qt6Guid.dll">
        $<$<CONFIG:Debug>:"${QT_BINARY_DIR}/Qt6Widgetsd.dll">
        $<$<CONFIG:Release>:"C:/Codice/littleCMS/Little-CMS-master//bin/lcms2.dll">
        $<$<CONFIG:Release>:"C:/Codice/ImageMagick-7.1.1-Q16-HDRI/CORE_RL_MagickWand_.dll">
        $<$<CONFIG:Release>:"${QT_BINARY_DIR}/Qt6Core.dll">
        $<$<CONFIG:Release>:"${QT_BINARY_DIR}/Qt6Gui.dll">
        $<$<CONFIG:Release>:"${QT_BINARY_DIR}/Qt6Widgets.dll">
        ${DLL_DIR}
)

# Update the PATH environment variable to include the directory containing the DLLs
set_target_properties(RefPickerProject PROPERTIES
    VS_DEBUGGER_ENVIRONMENT "PATH=${QT_BINARY_DIR};${DLL_DIR};$ENV{PATH}"
)
# Custom command to copy the platform plugins
add_custom_command(TARGET RefPickerProject POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${QT_BINARY_DIR}/../plugins/platforms"
        ${DLL_DIR}/platforms
)

# Custom command to copy the style plugins
add_custom_command(TARGET RefPickerProject POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${DLL_DIR}/styles
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${QT_BINARY_DIR}/../plugins/styles/qwindowsvistastyle.dll"
        ${DLL_DIR}/styles
)

# Custom commands to copy the LCMS2, ImageMagick, and OpenCV libraries
add_custom_command(TARGET RefPickerProject POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<$<CONFIG:Debug>:"${LCMS2_LIBRARIES_DEBUG}">
        $<$<CONFIG:Debug>:"${ImageMagick_LIBRARIES_DEBUG}">
        $<$<CONFIG:Debug>:"${OpenCV_DIR}/x64/vc16/bin/opencv_world490d.dll">
        $<$<CONFIG:Release>:"${LCMS2_LIBRARIES_RELEASE}">
        $<$<CONFIG:Release>:"${ImageMagick_LIBRARIES_RELEASE}">
        $<$<CONFIG:Release>:"${OpenCV_DIR}/x64/vc16/bin/opencv_world490.dll">
        ${DLL_DIR}
)

# Custom command to copy the ImageMagick core library
add_custom_command(TARGET RefPickerProject POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "C:/Codice/ImageMagick-7.1.1-Q16-HDRI/CORE_RL_magickcore_.dll"
        "C:/Codice/ImageMagick-7.1.1-Q16-HDRI/CORE_RL_bzlib_.dll"
        "C:/Codice/ImageMagick-7.1.1-Q16-HDRI/CORE_RL_freetype_.dll"
        "C:/Codice/ImageMagick-7.1.1-Q16-HDRI/CORE_RL_lcms_.dll"
        "C:/Codice/ImageMagick-7.1.1-Q16-HDRI/CORE_RL_lqr_.dll"
        "C:/Codice/ImageMagick-7.1.1-Q16-HDRI/CORE_RL_xml_.dll"
        "C:/Codice/ImageMagick-7.1.1-Q16-HDRI/CORE_RL_zlib_.dll"
        "C:/Codice/ImageMagick-7.1.1-Q16-HDRI/CORE_RL_fribidi_.dll"
        "C:/Codice/ImageMagick-7.1.1-Q16-HDRI/CORE_RL_harfbuzz_.dll"
        "C:/Codice/ImageMagick-7.1.1-Q16-HDRI/CORE_RL_glib_.dll"
        ${DLL_DIR}
)

# Custom command to copy the Visual C++ runtime libraries
add_custom_command(TARGET RefPickerProject POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Redist/MSVC/14.38.33135/x64/Microsoft.VC143.CRT/msvcp140.dll"
        "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Redist/MSVC/14.38.33135/x64/Microsoft.VC143.CRT/vcruntime140.dll"
        "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Redist/MSVC/14.38.33135/x64/Microsoft.VC143.CRT/vcruntime140_1.dll"
        "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Redist/MSVC/14.38.33135/x64/Microsoft.VC143.CRT/concrt140.dll"
        ${DLL_DIR}
)

# Custom command to copy the Visual C++ runtime libraries
add_custom_command(TARGET RefPickerProject POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "C:/Windows/System32/vcomp140.dll"
        "C:/Windows/System32/msvcp140_2.dll"
        "C:/Windows/System32/msvcp140_1.dll"
        ${DLL_DIR}
)

# Add the Qt binaries to the PATH environment variable
set_target_properties(RefPickerProject PROPERTIES
    VS_DEBUGGER_ENVIRONMENT "PATH=${QT_BINARY_DIR};$<TARGET_FILE_DIR:RefPickerProject>;${CMAKE_PREFIX_PATH}/bin;$ENV{PATH}"
)

# Update the PATH environment variable to include the directory containing the DLLs
set_target_properties(RefPickerProject PROPERTIES
    VS_DEBUGGER_ENVIRONMENT "PATH=${QT_BINARY_DIR};$<TARGET_FILE_DIR:RefPickerProject>;C:/Path/To/DLLs/;$ENV{PATH}"
)
# Include the LCMS2 directories
include_directories(${LCMS2_INCLUDE_DIRS})
