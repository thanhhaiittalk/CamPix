#include "campix/recorder.hpp"

namespace campix {

// Constructor
Recorder::Recorder(const std::string& device,
                   const std::string& encoder,
                   int bitrate,
                   const std::string& outfile)
    : device_(device),
      encoder_(encoder),
      bitrate_(bitrate),
      outfile_(outfile) {
    // TODO: constructor logic if needed
}

// Destructor
Recorder::~Recorder() {
    // TODO: cleanup resources
}

// Initialize pipeline
bool Recorder::init() {
    // Initialize 
    gst_init(nullptr, nullptr);

    // Build the pipeline
    if (buildPipeline())
        return true;
    return false;
}

// Start recording
bool Recorder::start() {
    GstStateChangeReturn ret = gst_element_set_state(pipeline_, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Failed to set pipeline to PLAYING\n");
        return false;
    }
    return true;
}

// Stop recording
void Recorder::stop() {
    if (!pipeline_) {
        g_printerr("Recorder::stop() called but pipeline_ is null\n");
        return;
    }

    // Step 1: stop the pipeline
    gst_element_set_state(pipeline_, GST_STATE_NULL);

    // Step 2: optional — block until state change completes
    GstStateChangeReturn ret = gst_element_get_state(pipeline_, nullptr, nullptr, GST_SECOND);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Failed to set pipeline to NULL cleanly\n");
    }

    // Step 3: clean up bus
    if (bus_) {
        gst_object_unref(bus_);
        bus_ = nullptr;
    }

    // Step 4: unref pipeline (this also unrefs all contained elements)
    gst_object_unref(pipeline_);
    pipeline_ = nullptr;

    g_print("Saved to %s\n", outfile_.c_str());
}


void Recorder::requestStop() {
    stopRequested_ = true;
}

// Run loop until EOS or error
void Recorder::run() {
    const GstClockTime timeout = GST_MSECOND * 100;
    while (true)
    {
        // Check for Ctrl-C
        if (stopRequested_ && !eosRequested_) {
            gst_element_send_event(pipeline_, gst_event_new_eos());
            eosRequested_ = true;
            g_print("[RUN] EOS requested\n");
        }
        GstMessage *msg = gst_bus_timed_pop_filtered(
            bus_, timeout, (GstMessageType)(GST_MESSAGE_EOS | GST_MESSAGE_ERROR));
        if (!msg) continue;
        if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_EOS) {
            g_print("[RUN] Message EOS\n");
            gst_message_unref(msg);
            return;
        } else if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
            GError *err;
            gchar *dbg;
            gst_message_parse_error(msg, &err, &dbg);
            std::cerr << "Error: " << err->message << "\n";
            g_error_free(err);
            g_free(dbg);
            gst_message_unref(msg);
            break;
        }
    }
    
}

// Build pipeline
bool Recorder::buildPipeline() {
    /*Chain concept
    v4l2src
  → (caps: video/x-bayer,format=rggb,1920x1080)
  → bayer2rgb
  → videoconvert
  → (caps: video/x-raw,format=NV12)
  → {v4l2h264enc | x264enc}
  → h264parse
  → mp4mux
  → filesink
    */
 // ---------------------------
    // Step 1) Create elements
    // ---------------------------
    pipeline_ = gst_pipeline_new("campix-rec");
    if (!pipeline_) { g_printerr("Failed to create pipeline\n"); return false; }
    std::cout <<"Debug buildPipeline: gst_pipeline_new completed\n";
    source_   = gst_element_factory_make("v4l2src",      "source");
    debayer_  = gst_element_factory_make("bayer2rgb",    "debayer");
    if (!debayer_) {
        // Fallback name on some systems
        debayer_ = gst_element_factory_make("debayer", "debayer");
    }
    convert_  = gst_element_factory_make("videoconvert", "convert");

    // Encoder: use class member encoder_ ("v4l2h264enc" or "x264enc")
    enc_      = gst_element_factory_make(encoder_.c_str(), "enc");

    GstElement* parse = gst_element_factory_make("h264parse", "parse");
    mux_      = gst_element_factory_make("mp4mux",      "mux");
    sink_     = gst_element_factory_make("filesink",    "sink");

    if (!source_ || !debayer_ || !convert_ || !enc_ || !parse || !mux_ || !sink_) {
        g_printerr("Missing elements: %s%s%s%s%s%s%s\n",
                   source_ ? "" : "v4l2src ",
                   debayer_ ? "" : "bayer2rgb/debayer ",
                   convert_ ? "" : "videoconvert ",
                   enc_ ? "" : encoder_.c_str(),
                   parse ? "" : "h264parse ",
                   mux_ ? "" : "mp4mux ",
                   sink_ ? "" : "filesink ");
        return false;
    }
    std::cout <<"Debug buildPipeline: create elements completed\n";

    // ---------------------------
    // Step 2) Configure properties
    // ---------------------------

    // v4l2src
    g_object_set(G_OBJECT(source_),
                 "device", device_.c_str(),
                 // "num-buffers", 300,     // <-- keep commented for continuous recording
                 NULL);

    // Encoder properties
    if (encoder_ == "v4l2h264enc") {
        std::cout <<"Debug buildPipeline: set encoder v4l2h264enc\n";
        GstStructure *s = gst_structure_new(
        "controls",
        "video_bitrate", G_TYPE_INT, bitrate_,
        NULL);

        g_object_set(G_OBJECT(enc_), "extra-controls", s, NULL);
        gst_structure_free(s); // free after set
    } else if (encoder_ == "x264enc") {
        // x264enc 'bitrate' is in kbps
        int kbps = (bitrate_ > 0) ? (bitrate_ / 1000) : 4000;
        g_object_set(G_OBJECT(enc_),
                     "bitrate", kbps,
                     "tune", 4,           // 4 = zerolatency (handy if you later stream)
                     "speed-preset", 1,   // ultrafast
                     NULL);
    }
    std::cout <<"Debug buildPipeline: set encoder completed\n";
    // mp4mux: optional but useful
    g_object_set(G_OBJECT(mux_), "faststart", TRUE, NULL);

    // filesink
    g_object_set(G_OBJECT(sink_), "location", outfile_.c_str(), NULL);

    // ---------------------------
    // Step 3) Add to pipeline
    // ---------------------------
    gst_bin_add_many(GST_BIN(pipeline_),
                     source_, debayer_, convert_, enc_, parse, mux_, sink_,
                     NULL);

    // ---------------------------
    // Step 4) Link elements
    // ---------------------------

    // 4a) source → debayer with Bayer caps
    GstCaps* caps_bayer = gst_caps_new_simple(
        "video/x-bayer",
        "format", G_TYPE_STRING, "rggb",
        "width",  G_TYPE_INT,    1920,
        "height", G_TYPE_INT,    1080,
        NULL);

    if (!gst_element_link_filtered(source_, debayer_, caps_bayer)) {
        g_printerr("Link failed: source → debayer with Bayer caps\n");
        gst_caps_unref(caps_bayer);
        return false;
    }
    std::cout <<"Debug buildPipeline: gst_element_link_filtered(source_, debayer_, caps_bayer) completed\n";
    gst_caps_unref(caps_bayer);

    // 4b) debayer → videoconvert
    if (!gst_element_link(debayer_, convert_)) {
        g_printerr("Link failed: debayer → videoconvert\n");
        return false;
    }
    std::cout <<"Debug buildPipeline: gst_element_link(debayer_, convert_) completed\n";

    // 4c) videoconvert → encoder with NV12 caps
    GstCaps* caps_nv12 = gst_caps_new_simple("video/x-raw",
                                             "format", G_TYPE_STRING, "NV12",
                                             NULL);
    if (!gst_element_link_filtered(convert_, enc_, caps_nv12)) {
        g_printerr("Link failed: videoconvert → enc with NV12 caps\n");
        gst_caps_unref(caps_nv12);
        return false;
    }
    std::cout <<"Debug buildPipeline: gst_element_link_filtered(convert_, enc_, caps_nv12) completed\n";
    gst_caps_unref(caps_nv12);

    // 4d) enc → h264parse
    if (!gst_element_link(enc_, parse)) {
        g_printerr("Link failed: enc → h264parse\n");
        return false;
    }
    std::cout <<"Debug buildPipeline: gst_element_link(enc_, parse) completed\n";

    // 4e) h264parse → mp4mux
    if (!gst_element_link(parse, mux_)) {
        g_printerr("Link failed: h264parse → mp4mux\n");
        return false;
    }
    std::cout <<"Debug buildPipeline: gst_element_link(parse, mux_) completed\n";
    // 4f) mp4mux → filesink
    if (!gst_element_link(mux_, sink_)) {
        g_printerr("Link failed: mp4mux → filesink\n");
        return false;
    }
    std::cout <<"Debug buildPipeline: gst_element_link(mux_, sink_) completed\n";

    // ---------------------------
    // Step 5) Get bus
    // ---------------------------
    bus_ = gst_element_get_bus(pipeline_);
    if (!bus_) {
        g_printerr("Failed to get bus from pipeline\n");
        return false;
    }

    return true;
}
} // namespace campix
