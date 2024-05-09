#include "gst/gstbus.h"
#include "gst/gstcaps.h"
#include "gst/gstclock.h"
#include "gst/gstelement.h"
#include "gst/gstelementfactory.h"
#include "gst/gstmessage.h"
#include "gst/gstobject.h"
#include "gst/gstpad.h"
#include "gst/gstpipeline.h"
#include "gst/gststructure.h"
#include "gst/gstutils.h"
#include <assert.h>
#include <gst/gst.h>

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

/* Structure to contain all our information, so we can pass it to callbacks */
typedef struct _CustomData {
	GstElement *pipeline;
	GstElement *source;
	GstElement *convert;
	GstElement *resample;
	GstElement *sink;
} CustomData;

/* Handler for the pad-added signal */
static void pad_added_handler(GstElement *src, GstPad *pad, CustomData *data);

int tutorial_main(int argc, char **argv) {
	CustomData data = { 0 };
	GstBus *bus;
	GstMessage *msg;
	GstStateChangeReturn ret;
	gboolean terminate = FALSE;

	/* Initialize Gstreamer */
	gst_init(&argc, &argv);

	/* Create the elements */
	data.source = gst_element_factory_make("uridecodebin", "source");
	data.convert = gst_element_factory_make("audioconvert", "convert");
	data.resample = gst_element_factory_make("audioresample", "resample");
	data.sink = gst_element_factory_make("autoaudiosink", "sink");
	/* create the empty pipeline */
	data.pipeline = gst_pipeline_new("test-pipeline");
	if (!data.pipeline || !data.source || !data.convert || !data.resample || !data.sink) {
		g_printerr("Not all elements could be created\n");
		return -1;
	}

	/* Set the URI to play */
	g_object_set(data.source, "uri", "https://gstreamer.freedesktop.org/data/media/sintel_trailer-480p.webm", NULL);

	/* Connect to the pad-added signal */
	g_signal_connect(data.source, "pad-added", G_CALLBACK(pad_added_handler), &data);

	/* Start playing */
	ret = gst_element_set_state(data.pipeline, GST_STATE_PLAYING);
	if (ret == GST_STATE_CHANGE_FAILURE) {
		g_printerr("Unable to set the pipeline to the playing state.\n");
		gst_object_unref(data.pipeline);
		return -1;
	}

	/* Listen to the bus */
	bus = gst_element_get_bus(data.pipeline);
	do {
		msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
								   GST_MESSAGE_STATE_CHANGED|GST_MESSAGE_ERROR|GST_MESSAGE_EOS);
		if (msg) {
			GError *err;
			gchar *debug_info;

			switch (GST_MESSAGE_TYPE(msg)) {
			case GST_MESSAGE_ERROR:
				gst_message_parse_error(msg, &err, &debug_info);
				break;
			case GST_MESSAGE_EOS:
				g_print("End-Of-Stream reached.\n");
				terminate = TRUE;
				break;
			case GST_MESSAGE_STATE_CHANGED:
				/* We are only interested in state-changed messsages from the pipeline */
				if (GST_MESSAGE_SRC(msg) == GST_OBJECT(data.pipeline)) {
					GstState old_state, new_state, pending_state;
					gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
					g_print("Pipeline state changed from %s to %s:\n", gst_element_state_get_name(old_state),
							gst_element_state_get_name(new_state));
				}
				break;
			default:
				g_printerr("Unexpected message received.\n");
				break;
			}
			gst_message_unref(msg);
		}
	} while(!terminate);

	/* Free resources */
	gst_object_unref(bus);
	gst_element_set_state(data.pipeline, GST_STATE_NULL);
	gst_object_unref(data.pipeline);

	return 0;
}

int main(int argc, char **argv)
{
#if defined(__APPLE__) && TARGET_OS_MAC && !TARGET_OS_IPHONE
	return gst_macos_main(tutorial_main, argc, argv, NULL);
#else
	return tutorial_main(argc, argv);
#endif
}

static void pad_added_handler(GstElement *src, GstPad *new_pad, CustomData *data)
{
	GstPad *sink_pad = gst_element_get_static_pad(data->convert, "sink");
	GstPadLinkReturn ret;
	GstCaps *new_pad_caps = NULL;
	GstStructure *new_pad_struct = NULL;
	const gchar *new_pad_type = NULL;

	g_print("Received new pad '%s' from '%s':\n", GST_PAD_NAME(new_pad), GST_ELEMENT_NAME(src));

	/* If our converter is already linked, we have noting to do here */
	if (gst_pad_is_linked(sink_pad)) {
		g_print("we are already linked. Ignoring.\n");
		goto exit;
	}
	new_pad_caps = gst_pad_get_current_caps(new_pad);
	new_pad_struct = gst_caps_get_structure(new_pad_caps, 0);
	new_pad_type = gst_structure_get_name(new_pad_struct);
	if (!g_str_has_prefix(new_pad_type, "audio/x-raw")) {
		g_print("It has type '%s' which is raw audio. Ignoring.\n", new_pad_type);
		goto exit;
	}

	/* Attempt the link */
	ret = gst_pad_link(new_pad, sink_pad);
	if (GST_PAD_LINK_FAILED(ret)) {
		g_print("Type is '%s' but link failed.\n", new_pad_type);
	} else {
		g_print("Link succeeded (type '%s').\n", new_pad_type);
	}

exit:
	if (new_pad_caps != NULL) {
		gst_caps_unref(new_pad_caps);
	}
	gst_object_unref(sink_pad);
	return;
}
