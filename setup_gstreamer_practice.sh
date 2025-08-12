#!/bin/bash

# Root of your repo (adjust if needed)
REPO_ROOT="$PWD"

# Practice root
PRACTICE_DIR="$REPO_ROOT/practice/gstreamer"

# Create folder structure
mkdir -p "$PRACTICE_DIR/include/campix"
mkdir -p "$PRACTICE_DIR/src"

# Create C++ header files
touch "$PRACTICE_DIR/include/campix/cli.hpp"
touch "$PRACTICE_DIR/include/campix/pipeline_builder.hpp"
touch "$PRACTICE_DIR/include/campix/recorder.hpp"
touch "$PRACTICE_DIR/include/campix/device_query.hpp"
touch "$PRACTICE_DIR/include/campix/segmenter.hpp"
touch "$PRACTICE_DIR/include/campix/rtp_streamer.hpp"

# Create C++ source files
touch "$PRACTICE_DIR/src/main.cpp"
touch "$PRACTICE_DIR/src/cli.cpp"
touch "$PRACTICE_DIR/src/pipeline_builder.cpp"
touch "$PRACTICE_DIR/src/recorder.cpp"
touch "$PRACTICE_DIR/src/device_query.cpp"
touch "$PRACTICE_DIR/src/segmenter.cpp"
touch "$PRACTICE_DIR/src/rtp_streamer.cpp"

# Create CMakeLists
touch "$PRACTICE_DIR/CMakeLists.txt"

# Create repo root files if not exist
touch "$REPO_ROOT/CMakeLists.txt"
touch "$REPO_ROOT/.gitignore"

echo "✅ Project structure for CamPix GStreamer practice created at:"
echo "$PRACTICE_DIR"
