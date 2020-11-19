#include <stdio.h>
#include <signal.h>
#include <glib.h>
#include <libsoup/soup.h>

GMainLoop *loop = NULL;

void signalHandler()
{
    if (loop) {
    	g_main_loop_quit(loop);
    }
}


// another task that's running alongside websocket

gboolean anotherTaskCallback()
{
	printf("another task is running\n");
	return TRUE;
}


// websocket management

void messageCallback(SoupWebsocketConnection *ws, SoupWebsocketDataType type, GBytes *message)
{
	gsize dataSize;
    const char* data = g_bytes_get_data(message, &dataSize);
    printf("got message: %.*s\n", (int) dataSize, data);
}

void connectCallback(GObject* session, GAsyncResult* result, gpointer userData)
{
	GError *error = NULL;
	SoupWebsocketConnection *connection = soup_session_websocket_connect_finish (SOUP_SESSION (session), result, &error);
	if (error) {
		fprintf(stderr, "websocket failed to connect, error: %d\n", error->code);
		return;
	}

	printf("websocket connected\n");
	g_signal_connect (connection, "message", G_CALLBACK (messageCallback), NULL);
}


int main(int argc, char *argv[])
{
	char *url;
	GMainContext *context; // has to be the same context that libsoup attaches websocket source to
	GSource *anotherTaskSource; // another task task runs alongside websocket

	if (argc < 2) {
		fprintf(stderr, "must provide URL as the first argument\n");
		return 1;
	}

	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);

	url = argv[1];

	context = g_main_context_ref_thread_default();
	anotherTaskSource = g_timeout_source_new(1000);
	loop = g_main_loop_new(context, FALSE);

	g_source_set_callback(anotherTaskSource, anotherTaskCallback, loop, NULL);
	g_source_set_priority(anotherTaskSource, 10);
	g_source_attach(anotherTaskSource, context);

	SoupSession *session = soup_session_new();
	SoupMessage *msg = soup_message_new("GET", url);
	soup_session_websocket_connect_async(session, msg, NULL, NULL, NULL, connectCallback, NULL);

	g_main_loop_run(loop);
	g_main_loop_unref(loop);

	printf("Done!\n");

	return 0;
}
