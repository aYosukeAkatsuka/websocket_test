#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libwebsockets.h>

static int callback_http(struct libwebsocket_context *context,
                         struct libwebsocket *wsi,
                         enum libwebsocket_callback_reasons reason,
                         void *user, void *in, size_t len)
{
  return 0;
}


static int
callback_test (struct libwebsocket_context *context,
               struct libwebsocket *wsi,
               enum libwebsocket_callback_reasons reason,
               void *user, void *in, size_t len)
{
  switch (reason) {
    case LWS_CALLBACK_ESTABLISHED: // just log message that someone is connecting
      printf("connection established\n");
      break;
    case LWS_CALLBACK_RECEIVE: {
      // the funny part
      // create a buffer to hold our response
      // it has to have some pre and post padding. You don't need to care
      // what comes there, libwebsockets will do everything for you. For more info see
      // http://git.warmcat.com/cgi-bin/cgit/libwebsockets/tree/lib/libwebsockets.h#n597
      unsigned char *buf = (unsigned char*) malloc(LWS_SEND_BUFFER_PRE_PADDING + len + LWS_SEND_BUFFER_POST_PADDING);
      int i;

      // pointer to `void *in` holds the incomming request
      // we're just going to put it in reverse order and put it in `buf` with
      // correct offset. `len` holds length of the request.
      for (i=0; i < len; i++) {
        buf[LWS_SEND_BUFFER_PRE_PADDING + (len - 1) - i ] = ((char *) in)[i];
      }

      // log what we recieved and what we're going to send as a response.
      // that disco syntax `%.*s` is used to print just a part of our buffer
      // http://stackoverflow.com/questions/5189071/print-part-of-char-array
      printf("received data: %s, replying: %.*s\n", (char *) in, (int) len,
          buf + LWS_SEND_BUFFER_PRE_PADDING);

      // send response
      // just notice that we have to tell where exactly our response starts. That's
      // why there's `buf[LWS_SEND_BUFFER_PRE_PADDING]` and how long it is.
      // we know that our response has the same length as request because
      // it's the same message in reverse order.
      libwebsocket_write(wsi, &buf[LWS_SEND_BUFFER_PRE_PADDING], len, LWS_WRITE_TEXT);

      // release memory back into the wild
      free(buf);
      break;
    }
    default:
      break;
  }
  return 0;
}

static struct libwebsocket_protocols protocols[] = {
  {
    "http-only",
    callback_http,
    0,
    0
  },
  {
    "test",
    callback_test,
    0,
    0
  },
  { NULL, NULL, 0, 0 } /* terminator */
};

int
main ()
{
  struct libwebsocket_context *context;
  struct lws_context_creation_info info;
  memset(&info, 0, sizeof info);

  info.port = 7681;
  info.iface = NULL;
  info.protocols = protocols;
  info.extensions = libwebsocket_get_internal_extensions();
  info.ssl_cert_filepath = NULL;
  info.ssl_private_key_filepath = NULL;
  info.gid = -1;
  info.uid = -1;
  info.options = 0;

  context = libwebsocket_create_context(&info);

  printf ("starting server.\n");

  while (1) {
    libwebsocket_service(context, 50);
  }

  libwebsocket_context_destroy(context);
  return 0;
}

