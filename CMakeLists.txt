# CMakeList.txt : CMake project for OrdMatchingEngine, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("OrdMatchingEngine")

# Add source to this project's executable.
add_executable (OrdMatchingEngine "OrdMatchingEngine.cpp" "OrdMatchingEngine.h" "DecimalLong.h" "Defn.h" "OrdEvent.h" "Order.h" "OrdBook.h"  )

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET OrdMatchingEngine PROPERTY CXX_STANDARD 14)
endif()

# TODO: Add tests and install targets if needed.
target_compile_features(OrdMatchingEngine PUBLIC cxx_std_14)
