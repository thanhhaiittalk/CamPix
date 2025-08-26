// practice/gstreamer/src/cli.cpp
#include "campix/cli.hpp"
#include <getopt.h>
#include <iostream>
using namespace campix;
Cli::Cli(int argc, char** argv)
    : argc(argc), argv(argv) {}

bool Cli::parse() {
    if (argc < 2) {
        printTopUsage();
        return false;
    }
    std::string subcommand_ = argv[1];
    int opargc = argc - 1;
    char** opargv = argv + 1;
    std::cout << "subcommand: " << subcommand_ << "\n";
    if (subcommand_ == kSubcmdRecord) {
        opts_.type = Options::Type::Record;
        return parseRecord(opargc, opargv);

    } else if (subcommand_ == kSubcmdRtp) {
        opts_.type = Options::Type::Rtp;
        return parseRtp(opargc, opargv);

    } else {
        // Unknown subcommand
        std::cerr << "Unknown subcommand: " << subcommand_ << "\n";
        printTopUsage();
        return false;
    }   

}

void Cli::printTopUsage() const {
    std::cerr
      << "Usage: " << argv[0] << " <subcommand> [options]\n\n"
      << "Subcommands:\n"
      << "  " << kSubcmdRecord << "   Record video to a file\n"
      << "  " << kSubcmdRtp    << "   Stream video over RTP\n"
      << "\nUse `" << argv[0] << " <subcommand> --help` for details.\n";
}

bool Cli::parseRecord(int sub_argc, char** sub_argv) {
    opterr = 0;
    optind = 1;  // skip sub_argv[0] (the subcommand itself)

    option longopts[] = {
        {"help",    no_argument,       nullptr, 'h'},
        {"device",  required_argument,  nullptr, 'd'},
        {"enc",     required_argument,  nullptr, 'e'},
        {"bitrate", required_argument,  nullptr, 'b'},
        {"outfile", required_argument,  nullptr, 'o'},
        {nullptr,   0,                  nullptr,  0 }
    };
    const char* optstr = "hd:e:b:o:";
     int c;
    while ((c = getopt_long(sub_argc, sub_argv, optstr, longopts, nullptr)) != -1) {
        switch (c) {
            case 'h':
                std::cout <<
                  "Usage: " << argv[0] << " record [options]\n"
                  "  -d, --device   PATH     (/dev/video3)\n"
                  "  -e, --enc      NAME     (auto)\n"
                  "  -b, --bitrate  INT      (4000000)\n"
                  "  -o, --outfile  FILE     (clip.mp4)\n";
                return false;
            case 'd': opts_.record.common.device  = optarg; break;
            case 'e': opts_.record.common.enc     = optarg; break;
            case 'b': opts_.record.common.bitrate = std::stoi(optarg); break;
            case 'o': opts_.record.outfile        = optarg; break;
            case '?':
            default:
                std::cerr << "record: invalid option\n";
                return false;
        }
    }
    return true;
}

bool Cli::parseRtp(int sub_argc, char** sub_argv) {
    std::cout << "RTP WIP\n";
    return true;
}