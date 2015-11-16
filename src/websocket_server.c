#include <stdio.h>
#include <stdlib.h>
#include <libwebsockets.h>

static int callback_http(struct libwebsocket_context *this,
                         struct libwebsocket *wsi,
                         enum libwebsocket_callback_reasons reason,
                         void *user,
                         void *in,
                         size_t len)
{
    return 0;
}

static int callback_dumb_increment(struct libwebsocket_context *this,
                                   struct libwebsocket *wsi,
                                   enum libwebsocket_callback_reasons reason,
                                   void *user,
                                   void *in,
                                   size_t len)
{
    switch (reason) 
    {
        case LWS_CALLBACK_ESTABLISHED: 
            // just log message that someone is connecting
            printf("connection established\n");
            break;
        case LWS_CALLBACK_RECEIVE:
        {
            /* create a buffer to hold our response
             * Is has to have some pre and post padding. You don't need to care
             * what comes there, libwebsockets will do everythign for you.
             */
            char *buf = (char *) malloc(LWS_SEND_BUFFER_PRE_PADDING + len
                                      + LWS_SEND_BUFFER_POST_PADDING);
            int i;
            // reverse the input
            for(i = 0; i < len; i++)
            {
                buf[LWS_SEND_BUFFER_PRE_PADDING + (len - 1) - i] 
                    = ((char *)in)[i];
            }

            printf("received data: %s\nreplying: %.*s\n", (char *)in, (int)len,
                    buf + LWS_SEND_BUFFER_PRE_PADDING);
            // send response
            libwebsocket_write(wsi, &buf[LWS_SEND_BUFFER_PRE_PADDING], len,
                               LWS_WRITE_TEXT);
            break;
        }
        default:
            printf("error: %s\n", (char *)in);
            break;
    }
    return 0;
}

static struct libwebsocket_protocols protocols[] = {
    {
        "http-only",                // name
        callback_http,              // callback
        0                           // per_session_data_size
    },
    {
        "dumb-increment-protocol",  // protocol name
        callback_dumb_increment,    // callback
        0                           // we don't use any per session data
    },
    {
        NULL, NULL, 0               // End of list
    }
};

int main(int argc, char *argv[])
{

    // server url will be http://localhost:9000
    int port = 9000;
    const char *interface = NULL;
    struct libwebsocket_context *context;
    // we're not using ssl
    const char *cert_path = NULL;
    const char *key_path = NULL;
    // no spcial optinos
    int opts = 0;

    // create libwebsocket context representing this server
    context = libwebsocket_create_context(port, interface, protocols,
                                          libwebsocket_internal_extensions,
                                          cert_path, key_path, NULL,
                                          -1, -1, opts, NULL);
    if (context == NULL) 
    {
        fprintf(stderr, "libwebsocket init failed\n");
        return -1;
    }

    printf("starting server...\n");

    // infinite loop, to end this server send SIGTERM. (CTRL-C)
    while (1)
    {
        libwebsocket_service(context, 500);
    }
    libwebsocket_context_destroy(context);

    return 0;
}


