# Minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Project name
project(RefPickerProjectREFRACTORED)

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

# Instruct CMake to run moc/uic automatically when needed
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# Set the Qt6 directory
set(CMAKE_PREFIX_PATH "C:/Qt/6.6.3/msvc2019_64/lib/cmake")
list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.6.3/msvc2019_64/lib/cmake/Qt6CoreTools")
list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.6.3/msvc2019_64/lib/cmake/Qt6GuiTools")
list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.6.3/msvc2019_64/lib/cmake/Qt6Network")

# Set the OpenCV directory
set(OpenCV_DIR "C:/Codice/opencv/build")
list(APPEND CMAKE_PREFIX_PATH "C:/Codice/opencv/build")
list(APPEND CMAKE_PREFIX_PATH "C:/Codice/opencv/build/x64/vc16/lib")

# Set the LittleCMS include and library directories
set(LCMS2_INCLUDE_DIRS "C:/Codice/littleCMS/Little-CMS-master/include")
set(LCMS2_LIBRARIES "C:/Codice/littleCMS/Little-CMS-master/bin/lcms2.lib")

# Set the ImageMagick include + lib directories
set(ImageMagick_INCLUDE_DIRS 
    "C:/Codice/ImageMagick-7.1.1-Q16-HDRI/include/MagickWand"
    "C:/Codice/ImageMagick-7.1.1-Q16-HDRI/include/MagickCore"
    "C:/Codice/ImageMagick-7.1.1-Q16-HDRI/include"
)
set(ImageMagick_LIBRARIES "C:/Codice/ImageMagick-7.1.1-Q16-HDRI/lib/CORE_RL_MagickWand_.lib")

# Find the necessary packages
find_package(Qt6 COMPONENTS Core Gui Widgets Network REQUIRED)
find_package(OpenCV REQUIRED)

# Grab the location of Qt binaries
get_target_property(QT_BINARY_DIR Qt6::Core LOCATION)
get_filename_component(QT_BINARY_DIR "${QT_BINARY_DIR}" DIRECTORY)

# ----------------------------------------------------------------------------
#         ADJUST THE PATHS HERE TO MATCH YOUR SPLIT FILES
# ----------------------------------------------------------------------------
# For example, if your refactored code is all in:
#   C:\Codice\c++\Test gpt remake
# then list each .cpp (and optionally .h) below.

set(SOURCE_FILES
    "C:/Codice/c++/Test gpt remake/reference picker REFRACTORED/main.cpp"
    "C:/Codice/c++/Test gpt remake/reference picker REFRACTORED/mainwindow.cpp"
    "C:/Codice/c++/Test gpt remake/reference picker REFRACTORED/zoomablegraphicsview.cpp"
    "C:/Codice/c++/Test gpt remake/reference picker REFRACTORED/imageutils.cpp"
    "C:/Codice/c++/Test gpt remake/reference picker REFRACTORED/scheduledialog.cpp"
    "C:/Codice/c++/Test gpt remake/reference picker REFRACTORED/settingsdialog.cpp"  # Add this line
)

set(HEADER_FILES
    "C:/Codice/c++/Test gpt remake/reference picker REFRACTORED/mainwindow.h"
    "C:/Codice/c++/Test gpt remake/reference picker REFRACTORED/zoomablegraphicsview.h"
    "C:/Codice/c++/Test gpt remake/reference picker REFRACTORED/imageutils.h"
    "C:/Codice/c++/Test gpt remake/reference picker REFRACTORED/scheduledialog.h"
    "C:/Codice/c++/Test gpt remake/reference picker REFRACTORED/settingsdialog.h"
  
)

# ----------------------------------------------------------------------------
# Create the executable
# ----------------------------------------------------------------------------
add_executable(RefPickerProjectREFRACTORED WIN32
    ${SOURCE_FILES}
    ${HEADER_FILES}
)

# Include the OpenCV, LCMS2, ImageMagick directories
include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${LCMS2_INCLUDE_DIRS}
    ${ImageMagick_INCLUDE_DIRS}
)

# Link the necessary libraries
target_link_libraries(
    RefPickerProjectREFRACTORED
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    ${OpenCV_LIBS}
    ${LCMS2_LIBRARIES}
    ${ImageMagick_LIBRARIES}
)

# ----------------------------------------------------------------------------
#               DLL copying / post-build commands
# ----------------------------------------------------------------------------

# Define the directory where the DLLs will be copied
set(DLL_DIR $<TARGET_FILE_DIR:RefPickerProjectREFRACTORED>/dlls)

# Custom command to create the folder and copy the DLLs
add_custom_command(TARGET RefPickerProjectREFRACTORED POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DLL_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<$<CONFIG:Debug>:"C:/Codice/littleCMS/Little-CMS-master/bin/lcms2.dll">
        $<$<CONFIG:Debug>:"C:/Codice/ImageMagick-7.1.1-Q16-HDRI/CORE_RL_MagickWand_.dll">
        $<$<CONFIG:Debug>:"${QT_BINARY_DIR}/Qt6Cored.dll">
        $<$<CONFIG:Debug>:"${QT_BINARY_DIR}/Qt6Guid.dll">
        $<$<CONFIG:Debug>:"${QT_BINARY_DIR}/Qt6Widgetsd.dll">
        $<$<CONFIG:Release>:"C:/Codice/littleCMS/Little-CMS-master/bin/lcms2.dll">
        $<$<CONFIG:Release>:"C:/Codice/ImageMagick-7.1.1-Q16-HDRI/CORE_RL_MagickWand_.dll">
        $<$<CONFIG:Release>:"${QT_BINARY_DIR}/Qt6Core.dll">
        $<$<CONFIG:Release>:"${QT_BINARY_DIR}/Qt6Gui.dll">
        $<$<CONFIG:Release>:"${QT_BINARY_DIR}/Qt6Widgets.dll">
        $<$<CONFIG:Release>:"${QT_BINARY_DIR}/Qt6Network.dll">
        ${DLL_DIR}
)

# Update PATH in the Visual Studio debugger environment
set_target_properties(RefPickerProjectREFRACTORED PROPERTIES
    VS_DEBUGGER_ENVIRONMENT "PATH=${QT_BINARY_DIR};${DLL_DIR};$ENV{PATH}"
)

# Copy the Qt platform plugins
add_custom_command(TARGET RefPickerProjectREFRACTORED POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${QT_BINARY_DIR}/../plugins/platforms"
        "${DLL_DIR}/platforms"
)

# Copy the Qt style plugins
add_custom_command(TARGET RefPickerProjectREFRACTORED POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DLL_DIR}/styles"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${QT_BINARY_DIR}/../plugins/styles/qwindowsvistastyle.dll"
        "${DLL_DIR}/styles"
)

# Copy the LCMS2, ImageMagick, and OpenCV libraries
add_custom_command(TARGET RefPickerProjectREFRACTORED POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<$<CONFIG:Debug>:"${OpenCV_DIR}/x64/vc16/bin/opencv_world490d.dll">
        $<$<CONFIG:Release>:"${OpenCV_DIR}/x64/vc16/bin/opencv_world490.dll">
        ${DLL_DIR}
)

# Copy extra ImageMagick dependencies
add_custom_command(TARGET RefPickerProjectREFRACTORED POST_BUILD
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
        "${DLL_DIR}"
)

# Copy Visual C++ runtime libraries
add_custom_command(TARGET RefPickerProjectREFRACTORED POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Redist/MSVC/14.38.33135/x64/Microsoft.VC143.CRT/msvcp140.dll"
        "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Redist/MSVC/14.38.33135/x64/Microsoft.VC143.CRT/vcruntime140.dll"
        "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Redist/MSVC/14.38.33135/x64/Microsoft.VC143.CRT/vcruntime140_1.dll"
        "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Redist/MSVC/14.38.33135/x64/Microsoft.VC143.CRT/concrt140.dll"
        "${DLL_DIR}"
)

# Additional system libraries if needed
add_custom_command(TARGET RefPickerProjectREFRACTORED POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "C:/Windows/System32/vcomp140.dll"
        "C:/Windows/System32/msvcp140_2.dll"
        "C:/Windows/System32/msvcp140_1.dll"
        "${DLL_DIR}"
)

# Finally, ensure the PATH includes these DLLs
set_target_properties(RefPickerProjectREFRACTORED PROPERTIES
    VS_DEBUGGER_ENVIRONMENT "PATH=${QT_BINARY_DIR};$<TARGET_FILE_DIR:RefPickerProjectREFRACTORED>;\
C:/Codice/ImageMagick-7.1.1-Q16-HDRI;$ENV{PATH}"
)

# Also explicitly set PATH again if needed
set_target_properties(RefPickerProjectREFRACTORED PROPERTIES
    VS_DEBUGGER_ENVIRONMENT "PATH=${QT_BINARY_DIR};${DLL_DIR};$ENV{PATH}"
)

# Include the LCMS2 directories (for completeness, though we did include them above)
include_directories(${LCMS2_INCLUDE_DIRS})


# ----------------------------------------------------------------------------
#  CPack Configuration
# ----------------------------------------------------------------------------
set(CPACK_PACKAGE_NAME "RefPickerProjectREFRACTORED")
set(CPACK_PACKAGE_VENDOR "YourCompany")
set(CPACK_PACKAGE_CONTACT "support@yourcompany.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "RefPickerProjectREFRACTORED: An Image Processing and Reference Picker App")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "RefPickerProjectREFRACTORED")

# Optional: Specify a license file
#set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")

# Set the NSIS generator for Windows
set(CPACK_GENERATOR "NSIS")
# Configure NSIS-specific settings
set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_NAME}")
set(CPACK_NSIS_PACKAGE_NAME "${CPACK_PACKAGE_NAME} Installer")
set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
set(CPACK_NSIS_CONTACT "${CPACK_PACKAGE_CONTACT}")

# Optionally specify an icon (change path accordingly)
#set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/icon.ico")

include(CPack)
