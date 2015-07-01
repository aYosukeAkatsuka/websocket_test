
#include <libwebsockets.h>

static int
callback_test (struct libwebsocket_context *context,
               struct libwebsocket *wsi,
               enum libwebsocket_callback_reasons reason,
               void *user,
               void *in,
               size_t len)
{
  return 0;
}

struct per_session_data {
  int number;
};

static struct libwebsocket_protocols protocols[] = {
  {
    "test",
    callback_test,
    sizeof (struct per_session_data),
    0,
  },
  { NULL, NULL, 0, 0 } /* terminator */
};

int
main ()
{
  struct lws_context_creation_info info;
  struct libwebsocket_context *context;
  const char *interface = NULL;
  int n;

  memset(&info, 0, sizeof info);
  info.port = 7681;
  info.iface = interface;
  info.protocols = protocols;
  info.extensions = libwebsocket_get_internal_extensions();
  info.ssl_cert_filepath = NULL;
  info.ssl_private_key_filepath = NULL;
  info.gid = -1;
  info.uid = -1;
  info.options = 0;

  context = libwebsocket_create_context(&info);

  n = 0;
  while (n >= 0) {
    unsigned int ms, oldms = 0;
    struct timeval tv;

    ms = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
    if ((ms - oldms) > 50) {
      libwebsocket_callback_on_writable_all_protocol(&protocols[0]);
      oldms = ms;
    }

    n = libwebsocket_service(context, 50);
  }

  libwebsocket_context_destroy(context);
  return 0;
}

