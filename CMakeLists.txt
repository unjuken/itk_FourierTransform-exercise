
cmake_minimum_required(VERSION 2.8)

project(FourierImage)

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})
if (ITKVtkGlue_LOADED)
  find_package(VTK REQUIRED)
  include(${VTK_USE_FILE})
endif()

add_executable(FourierTransform ImageFourierTransform.cxx)
target_link_libraries(FourierTransform ${ITK_LIBRARIES})

add_executable(FilterInFourier FilterImageInFourierDomain.cxx)
target_link_libraries(FilterInFourier ${ITK_LIBRARIES})

add_executable(FFTDirectInverse FFTDirectInverse.cxx)
target_link_libraries(FFTDirectInverse ${ITK_LIBRARIES})
