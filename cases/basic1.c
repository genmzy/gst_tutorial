#include "gst/gstobject.h"
#include <gst/gst.h>

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

/* stream can be file:// or http:// */
#if 0
#define STREAM_SOURCE "file:///home/waves/Workspace/clang/gst_tutorial/resources/three_season_warm.mp3"
#endif
#define STREAM_SOURCE "https://gstreamer.freedesktop.org/data/media/sintel_trailer-480p.webm"

int turtorial_main(int argc, char **argv)
{
	GstElement *pipeline;
	GstBus *bus;
	GstMessage *msg;

	/* Initialize GStreamer */
	gst_init(&argc, &argv);

	/* Build the pipeline */
	pipeline = gst_parse_launch("playbin uri=" STREAM_SOURCE, NULL);

	/* Start playing */
	gst_element_set_state(pipeline, GST_STATE_PLAYING);

	/* Wait until error or EOS */
	bus = gst_element_get_bus(pipeline);
	msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

	/* See next tutorial for proper error message handling/parsing */
	if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
		g_error("An error occurred! Re-run with the GST_DEBUG=*:WARN environment variable set for more details.");
	}

	/* Free resources */
	gst_message_unref(msg);
	gst_object_unref(bus);
	gst_element_set_state(pipeline, GST_STATE_NULL);
	gst_object_unref(pipeline);

	return 0;
}

int main(int argc, char *argv[])
{
#if defined(__APPLE__) && TARGET_OS_MAC && !TARGET_OS_IPHONE
	return gst_macos_main(turtorial_main, argc, argv, NULL);
#else
	return turtorial_main(argc, argv);
#endif
}
