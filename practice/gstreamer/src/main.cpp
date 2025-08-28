#include <gst/gst.h>
#include <iostream>
#include "campix/cli.hpp"
#include "campix/recorder.hpp"

campix::Recorder* gRecorder = nullptr;   // global pointer, initially null
const campix::Options* gOpts = nullptr;   // global pointer

void sigintHandler(int sig) {
    std::cout << "\nSIGINT received!\n";

    if (gOpts) {
        switch (gOpts->type) {
            case campix::Options::Type::Record:
                std::cout << "STOP RECORD\n";
                gRecorder->requestStop();
                break;
            case campix::Options::Type::Rtp:
                std::cout << "STOP RTP\n";
                break;
            default:
                std::cout << "Unknown mode\n";
                break;
        }
    }
}
int main(int argc, char *argv[]) {
    signal(SIGINT, sigintHandler);

    campix::Cli cli(argc, argv);
    if (!cli.parse()) return 1;
    const campix::Options& opts = cli.getOptions();
    gOpts = &opts;   // now global pointer points to same object
    switch (opts.type) {
        case campix::Options::Type::Record: {
            std::cout << "[Record] dev=" << opts.record.common.device
                      << " enc=" << opts.record.common.enc
                      << " br="  << opts.record.common.bitrate
                      << " out=" << opts.record.outfile << "\n";
            campix::Recorder record(opts.record.common.device, 
                                    opts.record.common.enc,
                                    opts.record.common.bitrate,
                                    opts.record.outfile);

            gRecorder = &record;   // point global pointer to this object
            record.init();

            record.start();

            record.run();

            record.stop();
            break;
        }    
        case campix::Options::Type::Rtp: {
            std::cout << "[RTP] dev=" << opts.rtp.common.device
                      << " enc=" << opts.rtp.common.enc
                      << " br="  << opts.rtp.common.bitrate
                      << " host="<< opts.rtp.host
                      << " port="<< opts.rtp.port << "\n";
            // runRtp(opts.rtp);
            break;
        }
        default: break;
    }
    return 0;
}
