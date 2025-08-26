#pragma once
#include <gst/gst.h>
#include <string>
#include <iostream>
#include <atomic>

namespace campix {

class Recorder {
public:
    // Constructor with options
    Recorder(const std::string& device,
             const std::string& encoder,
             int bitrate,
             const std::string& outfile);

    ~Recorder();

    // Initialize pipeline
    bool init();

    // Start recording
    bool start();

    // Mainly clean resource
    void stop();

    // Send EOS to save mp4 file before stopping
    void requestStop();

    // Run loop until EOS or error
    void run();

private:
    // Helper to build pipeline
    bool buildPipeline();

private:
    std::string device_;
    std::string encoder_;
    int bitrate_;
    std::string outfile_;

    GstElement* pipeline_ = nullptr;
    GstElement* source_   = nullptr;
    GstElement* debayer_  = nullptr;
    GstElement* convert_  = nullptr;
    GstElement* enc_      = nullptr;
    GstElement* mux_      = nullptr;
    GstElement* sink_     = nullptr;

    std::atomic<bool> stopRequested_{false};
    bool eosRequested_ = false;

    GstBus* bus_ = nullptr;
};

} // namespace campix
