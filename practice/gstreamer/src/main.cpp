#include <gst/gst.h>
#include <iostream>

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);

    const char *pipeline_desc =
    "v4l2src device=/dev/video3 num-buffers=300 ! "
    "video/x-bayer,format=rggb,width=1920,height=1080 ! "
    "bayer2rgb ! videoconvert ! video/x-raw,format=NV12 ! "
    "v4l2h264enc extra-controls=\"controls,video_bitrate=4000000;\" ! "
    "h264parse ! mp4mux ! filesink location=video_10s_pro.mp4";

    GError *error = nullptr;
    GstElement *pipeline = gst_parse_launch(pipeline_desc, &error);
    if (!pipeline) {
        std::cerr << "Failed to create pipeline: " << (error ? error->message : "Unknown error") << "\n";
        if (error) g_error_free(error);
        return -1;
    }

    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    g_print("Recording video for 10 seconds...\n");

    // Wait for 10 seconds of running
    g_usleep(10 * G_USEC_PER_SEC);

    // Send EOS to finish file properly
    gst_element_send_event(pipeline, gst_event_new_eos());

    // Wait for EOS message (ensures MP4 is finalized)
    GstBus *bus = gst_element_get_bus(pipeline);
    bool eos_received = false;
    while (!eos_received) {
        GstMessage *msg = gst_bus_timed_pop_filtered(
            bus, GST_CLOCK_TIME_NONE, (GstMessageType)(GST_MESSAGE_EOS | GST_MESSAGE_ERROR));
        if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_EOS)
            eos_received = true;
        else if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
            GError *err;
            gchar *dbg;
            gst_message_parse_error(msg, &err, &dbg);
            std::cerr << "Error: " << err->message << "\n";
            g_error_free(err);
            g_free(dbg);
            break;
        }
        gst_message_unref(msg);
    }
    gst_object_unref(bus);

    // Stop and clean up
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    g_print("Saved to video_10s_pro.mp4\n");
    return 0;
}
