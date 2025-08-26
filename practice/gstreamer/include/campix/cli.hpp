#pragma once 
#include <string>
#include <vector>

#define SUBCM_RECORD   "record"
#define SUBCM_RTP      "rtp"


namespace campix{
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

// struct ListCapsOptions {               // Ex2
//     CommonOptions common;
// };


// struct RecordLimitOptions {            // Ex3
//     CommonOptions common;
//     std::string outfile = "clip.mp4";
//     int duration_sec = 0;              // 0 = disabled
//     int max_mb       = 0;              // 0 = disabled
// };

// struct RecordSegOptions {              // Ex4
//     CommonOptions common;
//     std::string pattern = "clip-%05d.mp4";
//     int segment_sec = 60;              // or use max-size-bytes if you prefer
// };

struct RtpOptions {                    // Ex5
    CommonOptions common;
    std::string host = "127.0.0.1";
    int port = 5000;
    int pay_config_interval = 1;       // seconds
};

struct Options {
    enum class Type {None, Record, Rtp} type = Type::None;
    RecordOptions record;
    RtpOptions rtp;
};   
class Cli {
public:
    Cli(int argc, char** argv);
    bool parse();   //Parse arguments into options
    const Options&         getOptions()         const { return opts_; }

    void printUsage() const;
private:
    int argc;
    char** argv;

    Options opts_;
    std::string subcommand_;

    bool parseRecord(int sub_argc, char** sub_argv);
    bool parseRtp(int sub_argc, char** sub_argv);

    void printTopUsage() const;
    
    // Constant
    // Subcommand
    static inline constexpr const char* kSubcmdRecord = "record";
    static inline constexpr const char* kSubcmdRtp    = "rtp";


};    
} // namespace campix

