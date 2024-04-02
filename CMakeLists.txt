cmake_minimum_required(VERSION 3.10)
project(RefPickerProject)

set(CMAKE_CXX_STANDARD 17)

# Add the /Zc:__cplusplus and /permissive- compiler flags
add_compile_options(/Zc:__cplusplus)
add_compile_options(/permissive-)

# Add NOMINMAX definition
add_definitions(-DNOMINMAX)

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
set(LCMS2_INCLUDE_DIRS "C:/Codice/littleCMS/Little-CMS-master/Little-CMS-master/include")
set(LCMS2_LIBRARIES "C:/Codice/littleCMS/Little-CMS-master/Little-CMS-master/bin/lcms2.lib")

# Set the ImageMagick include directories
set(ImageMagick_INCLUDE_DIRS "C:/Codice/ImageMagick-7.1.1-Q16-HDRI/include/MagickWand")
list(APPEND ImageMagick_INCLUDE_DIRS "C:/Codice/ImageMagick-7.1.1-Q16-HDRI/include/MagickCore")
list(APPEND ImageMagick_INCLUDE_DIRS "C:/Codice/ImageMagick-7.1.1-Q16-HDRI/include")
set(ImageMagick_LIBRARIES "C:/Codice/ImageMagick-7.1.1-Q16-HDRI/lib/CORE_RL_MagickWand_.lib")


# Find the necessary packages
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
find_package(OpenCV REQUIRED)

# Add the executable
add_executable(RefPickerProject refPC.cpp mainwindow.cpp)

# Include the OpenCV, LCMS2, and ImageMagick directories
include_directories(${OpenCV_INCLUDE_DIRS} ${LCMS2_INCLUDE_DIRS} ${ImageMagick_INCLUDE_DIRS})  # Add ${ImageMagick_INCLUDE_DIRS}

# Link the necessary libraries
target_link_libraries(RefPickerProject Qt6::Core Qt6::Gui Qt6::Widgets ${OpenCV_LIBS} ${LCMS2_LIBRARIES} ${ImageMagick_LIBRARIES})

# Include the LCMS2 directories
include_directories(${LCMS2_INCLUDE_DIRS})