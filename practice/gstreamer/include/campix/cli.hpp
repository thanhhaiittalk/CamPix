#pragma once 
#include <string>
#include <vector>

struct CommonOptions
{
    std::string device = "/dev/video3";
    std::string enc = "auto";
    int bitrate = 4'000'000;
};

// Subcommand-specific options
struct RecordOptions {                 // Ex1
    CommonOptions common;
    std::string outfile = "clip.mp4";
};

struct ListCapsOptions {               // Ex2
    CommonOptions common;
};


struct RecordLimitOptions {            // Ex3
    CommonOptions common;
    std::string outfile = "clip.mp4";
    int duration_sec = 0;              // 0 = disabled
    int max_mb       = 0;              // 0 = disabled
};

struct RecordSegOptions {              // Ex4
    CommonOptions common;
    std::string pattern = "clip-%05d.mp4";
    int segment_sec = 60;              // or use max-size-bytes if you prefer
};

struct RtpOptions {                    // Ex5
    CommonOptions common;
    std::string host = "127.0.0.1";
    int port = 5000;
    int pay_config_interval = 1;       // seconds
};

enum class Command {
    Unknown,
    Record,
    ListCaps,
    RecordLimit,
    RecordSeg,
    Rtp
};

class Cli {
public:
    Cli(int argc, char** argv);
    bool parse();   //Parse arguments into options
    
    Command command() const { return cmd; }

    const RecordOptions&      record()      const { return rec; }
    const ListCapsOptions&    listCaps()    const { return caps; }
    const RecordLimitOptions& recordLimit() const { return recLimit; }
    const RecordSegOptions&   recordSeg()   const { return seg; }
    const RtpOptions&         rtp()         const { return rtpOpt; }

    void printUsage() const;
private:
    int argc;
    char** argv;

    CommonOptions commonOptions;
    RecordOptions rec;
    ListCapsOptions caps;
    RecordLimitOptions recLimit;
    RecordSegOptions seg;
    RtpOptions rtpOpt;
    Command cmd = Command::Unknown;
    std::string subcommand;
};