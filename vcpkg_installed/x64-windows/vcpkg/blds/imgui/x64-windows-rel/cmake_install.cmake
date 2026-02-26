# Install script for directory: C:/Users/42077/source/repos/Project1/vcpkg_installed/x64-windows/vcpkg/blds/imgui/src/v1.91.9-afb09617a6.clean

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Users/42077/source/repos/Project1/vcpkg_installed/x64-windows/vcpkg/pkgs/imgui_x64-windows")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "OFF")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/42077/source/repos/Project1/vcpkg_installed/x64-windows/vcpkg/blds/imgui/x64-windows-rel/imgui.lib")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "C:/Users/42077/source/repos/Project1/vcpkg_installed/x64-windows/vcpkg/blds/imgui/src/v1.91.9-afb09617a6.clean/imgui.h"
    "C:/Users/42077/source/repos/Project1/vcpkg_installed/x64-windows/vcpkg/blds/imgui/src/v1.91.9-afb09617a6.clean/imconfig.h"
    "C:/Users/42077/source/repos/Project1/vcpkg_installed/x64-windows/vcpkg/blds/imgui/src/v1.91.9-afb09617a6.clean/imgui_internal.h"
    "C:/Users/42077/source/repos/Project1/vcpkg_installed/x64-windows/vcpkg/blds/imgui/src/v1.91.9-afb09617a6.clean/imstb_textedit.h"
    "C:/Users/42077/source/repos/Project1/vcpkg_installed/x64-windows/vcpkg/blds/imgui/src/v1.91.9-afb09617a6.clean/imstb_rectpack.h"
    "C:/Users/42077/source/repos/Project1/vcpkg_installed/x64-windows/vcpkg/blds/imgui/src/v1.91.9-afb09617a6.clean/imstb_truetype.h"
    "C:/Users/42077/source/repos/Project1/vcpkg_installed/x64-windows/vcpkg/blds/imgui/src/v1.91.9-afb09617a6.clean/misc/cpp/imgui_stdlib.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES "C:/Users/42077/source/repos/Project1/vcpkg_installed/x64-windows/vcpkg/blds/imgui/src/v1.91.9-afb09617a6.clean/backends/imgui_impl_glfw.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "C:/Users/42077/source/repos/Project1/vcpkg_installed/x64-windows/vcpkg/blds/imgui/src/v1.91.9-afb09617a6.clean/backends/imgui_impl_opengl3.h"
    "C:/Users/42077/source/repos/Project1/vcpkg_installed/x64-windows/vcpkg/blds/imgui/src/v1.91.9-afb09617a6.clean/backends/imgui_impl_opengl3_loader.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/imgui" TYPE FILE FILES "C:/Users/42077/source/repos/Project1/vcpkg_installed/x64-windows/vcpkg/blds/imgui/x64-windows-rel/imgui-config.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/imgui/imgui-targets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/imgui/imgui-targets.cmake"
         "C:/Users/42077/source/repos/Project1/vcpkg_installed/x64-windows/vcpkg/blds/imgui/x64-windows-rel/CMakeFiles/Export/791eb3786aa5a66c7063a763bc360501/imgui-targets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/imgui/imgui-targets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/imgui/imgui-targets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/imgui" TYPE FILE FILES "C:/Users/42077/source/repos/Project1/vcpkg_installed/x64-windows/vcpkg/blds/imgui/x64-windows-rel/CMakeFiles/Export/791eb3786aa5a66c7063a763bc360501/imgui-targets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/imgui" TYPE FILE FILES "C:/Users/42077/source/repos/Project1/vcpkg_installed/x64-windows/vcpkg/blds/imgui/x64-windows-rel/CMakeFiles/Export/791eb3786aa5a66c7063a763bc360501/imgui-targets-release.cmake")
  endif()
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "C:/Users/42077/source/repos/Project1/vcpkg_installed/x64-windows/vcpkg/blds/imgui/x64-windows-rel/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
if(CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_COMPONENT MATCHES "^[a-zA-Z0-9_.+-]+$")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
  else()
    string(MD5 CMAKE_INST_COMP_HASH "${CMAKE_INSTALL_COMPONENT}")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INST_COMP_HASH}.txt")
    unset(CMAKE_INST_COMP_HASH)
  endif()
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "C:/Users/42077/source/repos/Project1/vcpkg_installed/x64-windows/vcpkg/blds/imgui/x64-windows-rel/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
