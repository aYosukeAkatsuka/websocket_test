#include <stdio.h>
#include <string.h>
#include <libwebsockets.h>

static int was_closed;
static int deny_deflate;
static int deny_mux;

static int
callback_test(struct libwebsocket_context *this,
              struct libwebsocket *wsi,
              enum libwebsocket_callback_reasons reason,
              void *user,
              void *in,
              size_t len)
{
  switch (reason) {

    case LWS_CALLBACK_CLIENT_ESTABLISHED:
      fprintf(stderr, "callback_dumb_increment: LWS_CALLBACK_CLIENT_ESTABLISHED\n");
      break;

    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
      fprintf(stderr, "LWS_CALLBACK_CLIENT_CONNECTION_ERROR\n");
      was_closed = 1;
      break;

    case LWS_CALLBACK_CLOSED:
      fprintf(stderr, "LWS_CALLBACK_CLOSED\n");
      was_closed = 1;
      break;

    case LWS_CALLBACK_CLIENT_RECEIVE:
      ((char *)in)[len] = '\0';
      fprintf(stderr, "rx %d '%s'\n", (int)len, (char *)in);
      break;

      /* because we are protocols[0] ... */

    case LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED:
      if ((strcmp(in, "deflate-stream") == 0) && deny_deflate) {
        fprintf(stderr, "denied deflate-stream extension\n");
        return 1;
      }
      if ((strcmp(in, "deflate-frame") == 0) && deny_deflate) {
        fprintf(stderr, "denied deflate-frame extension\n");
        return 1;
      }
      if ((strcmp(in, "x-google-mux") == 0) && deny_mux) {
        fprintf(stderr, "denied x-google-mux extension\n");
        return 1;
      }

      break;

    default:
      break;
  }

  return 0;
}

/* list of supported protocols and callbacks */

static struct libwebsocket_protocols protocols[] = {
  {
    "test",
    callback_test,
    0,
    20,
  },
  { NULL, NULL, 0, 0 } /* end */
};

int main(int argc, char **argv)
{
  int n = 0;
  int ret = 0;
  int port = 7681;
  int use_ssl = 0;
  struct libwebsocket_context *context;
  struct libwebsocket *wsi_test;
  struct lws_context_creation_info info;

  memset(&info, 0, sizeof info);

  info.port = CONTEXT_PORT_NO_LISTEN;
  info.protocols = protocols;
  info.extensions = libwebsocket_get_internal_extensions();
  info.gid = -1;
  info.uid = -1;

  context = libwebsocket_create_context(&info);
  if (context == NULL) {
    fprintf(stderr, "Creating libwebsocket context failed\n");
    return 1;
  }

  wsi_test = libwebsocket_client_connect(/* context */        context,
                                         /* address */        "localhost",
                                         /* port */           7861,
                                         /* ssl_connection */ 0,
                                         /* path */           "/",
                                         /* host */           "localhost",
                                         /* origin */         "localhost",
                                         /* protocol */       "test",
                                         /* ietf_version_or_minus_one */ -1);

  if (wsi_test == NULL) {
    fprintf(stderr, "libwebsocket connect failed\n");
    return 1;
  }

  fprintf(stderr, "Waiting for connect...\n");

  n = 0;
  while (n >= 0 && !was_closed) {
    n = libwebsocket_service(context, 10);

    if (n < 0) {
      continue;
    }
  }

  libwebsocket_context_destroy(context);
  return 0;

}
