#include <stdio.h>
#include <string.h>
#include <libwebsockets.h>

static int was_closed;
static int deny_deflate;
static int deny_mux;

struct reason_string {
  enum libwebsocket_callback_reasons reason;
  char *reason_string;
};

static struct reason_string reason_strings[] = {
  {LWS_CALLBACK_ESTABLISHED, "LWS_CALLBACK_ESTABLISHED"},
  {LWS_CALLBACK_CLIENT_CONNECTION_ERROR,"LWS_CALLBACK_CLIENT_CONNECTION_ERROR"},
  {LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH,"LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH"},
  {LWS_CALLBACK_CLIENT_ESTABLISHED,"LWS_CALLBACK_CLIENT_ESTABLISHED"},
  {LWS_CALLBACK_CLOSED,"LWS_CALLBACK_CLOSED"},
  {LWS_CALLBACK_CLOSED_HTTP,"LWS_CALLBACK_CLOSED_HTTP"},
  {LWS_CALLBACK_RECEIVE,"LWS_CALLBACK_RECEIVE"},
  {LWS_CALLBACK_CLIENT_RECEIVE,"LWS_CALLBACK_CLIENT_RECEIVE"},
  {LWS_CALLBACK_CLIENT_RECEIVE_PONG,"LWS_CALLBACK_CLIENT_RECEIVE_PONG"},
  {LWS_CALLBACK_CLIENT_WRITEABLE,"LWS_CALLBACK_CLIENT_WRITEABLE"},
  {LWS_CALLBACK_SERVER_WRITEABLE,"LWS_CALLBACK_SERVER_WRITEABLE"},
  {LWS_CALLBACK_HTTP,"LWS_CALLBACK_HTTP"},
  {LWS_CALLBACK_HTTP_BODY,"LWS_CALLBACK_HTTP_BODY"},
  {LWS_CALLBACK_HTTP_BODY_COMPLETION,"LWS_CALLBACK_HTTP_BODY_COMPLETION"},
  {LWS_CALLBACK_HTTP_FILE_COMPLETION,"LWS_CALLBACK_HTTP_FILE_COMPLETION"},
  {LWS_CALLBACK_HTTP_WRITEABLE,"LWS_CALLBACK_HTTP_WRITEABLE"},
  {LWS_CALLBACK_FILTER_NETWORK_CONNECTION,"LWS_CALLBACK_FILTER_NETWORK_CONNECTION"},
  {LWS_CALLBACK_FILTER_HTTP_CONNECTION,"LWS_CALLBACK_FILTER_HTTP_CONNECTION"},
  {LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED,"LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED"},
  {LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION,"LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION"},
  {LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS,"LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS"},
  {LWS_CALLBACK_OPENSSL_LOAD_EXTRA_SERVER_VERIFY_CERTS,"LWS_CALLBACK_OPENSSL_LOAD_EXTRA_SERVER_VERIFY_CERTS"},
  {LWS_CALLBACK_OPENSSL_PERFORM_CLIENT_CERT_VERIFICATION,"LWS_CALLBACK_OPENSSL_PERFORM_CLIENT_CERT_VERIFICATION"},
  {LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER,"LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER"},
  {LWS_CALLBACK_CONFIRM_EXTENSION_OKAY,"LWS_CALLBACK_CONFIRM_EXTENSION_OKAY"},
  {LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED,"LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED"},
  {LWS_CALLBACK_PROTOCOL_INIT,"LWS_CALLBACK_PROTOCOL_INIT"},
  {LWS_CALLBACK_PROTOCOL_DESTROY,"WS_CALLBACK_PROTOCOL_DESTROY"},
  {LWS_CALLBACK_WSI_CREATE,"LWS_CALLBACK_WSI_CREATE"},
  {LWS_CALLBACK_WSI_DESTROY,"LWS_CALLBACK_WSI_DESTROY"},
  {LWS_CALLBACK_GET_THREAD_ID,"LWS_CALLBACK_GET_THREAD_ID"},
  {LWS_CALLBACK_ADD_POLL_FD,"LWS_CALLBACK_ADD_POLL_FD"},
  {LWS_CALLBACK_DEL_POLL_FD,"LWS_CALLBACK_DEL_POLL_FD"},
  {LWS_CALLBACK_CHANGE_MODE_POLL_FD,"LWS_CALLBACK_CHANGE_MODE_POLL_FD"},
  {LWS_CALLBACK_LOCK_POLL,"LWS_CALLBACK_LOCK_POLL"},
  {LWS_CALLBACK_UNLOCK_POLL,"LWS_CALLBACK_UNLOCK_POLL"},
  {LWS_CALLBACK_OPENSSL_CONTEXT_REQUIRES_PRIVATE_KEY,"LWS_CALLBACK_OPENSSL_CONTEXT_REQUIRES_PRIVATE_KEY"},
  {LWS_CALLBACK_USER,"LWS_CALLBACK_USE"}
};

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
      fprintf (stderr, "default case. reason = %s\n", reason_strings[reason].reason_string);
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
    0,
  },
  { NULL, NULL, 0, 0 } /* terminator */
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
                                         /* port */           port,
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

  //fprintf(stderr, "Waiting for connect...\n");

  //n = 0;
  //while (n >= 0 && !was_closed) {
  //  n = libwebsocket_service(context, 10);

  //  if (n < 0) {
  //    continue;
  //  }
  //}

  libwebsocket_context_destroy(context);
  return 0;

}
