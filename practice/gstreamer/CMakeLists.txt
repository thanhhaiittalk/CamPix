cmake_minimum_required(VERSION 3.16)
project(CamPix LANGUAGES C CXX)

# ---- Build options ----
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Sources / Includes ----
file(GLOB CAMPIX_SOURCES
  "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

add_executable(campix ${CAMPIX_SOURCES})
target_include_directories(campix PRIVATE "${CMAKE_SOURCE_DIR}/include")

# ---- Dependencies (GStreamer + GLib) via pkg-config ----
find_package(PkgConfig REQUIRED)

# --- Add for sysroot cross-compilation ---
if(DEFINED ARM_SYSROOT)
  # Force host pkg-config, but make it read ONLY from the target sysroot
  set(PKG_CONFIG_EXECUTABLE "/usr/bin/pkg-config" CACHE FILEPATH "")

  set(ENV{PKG_CONFIG_SYSROOT_DIR} "${ARM_SYSROOT}")
  set(ENV{PKG_CONFIG_LIBDIR}
      "${ARM_SYSROOT}/usr/lib/aarch64-linux-gnu/pkgconfig:"
      "${ARM_SYSROOT}/usr/lib/pkgconfig:"
      "${ARM_SYSROOT}/usr/share/pkgconfig")
  # IMPORTANT: Clear PATH-based fallbacks (prevents host leakage)
  set(ENV{PKG_CONFIG_PATH} "")
endif()

# Core GStreamer + common libs; add or remove as your code needs
pkg_check_modules(GSTREAMER REQUIRED IMPORTED_TARGET gstreamer-1.0)
pkg_check_modules(GSTREAMER_BASE REQUIRED IMPORTED_TARGET gstreamer-base-1.0)
pkg_check_modules(GSTREAMER_VIDEO REQUIRED IMPORTED_TARGET gstreamer-video-1.0)
pkg_check_modules(GSTREAMER_APP REQUIRED IMPORTED_TARGET gstreamer-app-1.0)
pkg_check_modules(GOBJECT REQUIRED IMPORTED_TARGET gobject-2.0)
pkg_check_modules(GLIB REQUIRED IMPORTED_TARGET glib-2.0)




# Make sure headers from pkg-config are visible
target_link_libraries(campix PRIVATE
  PkgConfig::GSTREAMER
  PkgConfig::GSTREAMER_BASE
  PkgConfig::GSTREAMER_VIDEO
  PkgConfig::GSTREAMER_APP
  PkgConfig::GOBJECT
  PkgConfig::GLIB
)

# Optional: install target (to copy onto the board later)
install(TARGETS campix RUNTIME DESTINATION bin)
