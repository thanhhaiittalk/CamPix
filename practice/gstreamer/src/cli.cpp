// practice/gstreamer/src/cli.cpp
#include "campix/cli.hpp"
#include <getopt.h>
#include <iostream>

Cli::Cli(int argc, char** argv)
    : argc(argc), argv(argv) {}

bool Cli::parse() {
    // TODO: implement CLI parsing for subcommands and options
    return false;
}

void Cli::printUsage() const {
    // TODO: implement usage/help text
}
