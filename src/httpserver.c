#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libwebsockets.h>

static int callback_http(struct libwebsocket_context *context,
                         struct libwebsocket *wsi,
                         enum libwebsocket_callback_reasons reason,
                         void *user,
                         void *in,
                         size_t len)
{
    switch (reason) 
    {
        case LWS_CALLBACK_CLIENT_WRITEABLE:
            printf("connection established\n");
        //    break;
        case LWS_CALLBACK_HTTP:
        {
            char *requested_uri = (char *)in;
            printf("requested URI: %s\n", requested_uri);

            if (strcmp(requested_uri, "/") == 0) 
            {
                void *universal_response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\nHello, World!";
                libwebsocket_write(wsi, universal_response,
                                   strlen(universal_response), LWS_WRITE_HTTP);
            }
            else
            {
                char cwd[1024];
                char *resource_path;

                if  (getcwd(cwd, sizeof(cwd)) != NULL)
                {
                    // allocate enough memory for resource path
                    resource_path = (char *)malloc(strlen(cwd) + 
                                                   strlen(requested_uri) + 1);

                    // join current working directory to resource path
                    sprintf(resource_path, "%s%s", cwd, requested_uri);
                    printf("resource path: %s\n", resource_path);

                    char *extension = strrchr(resource_path, '.');
                    char *mime;
                    // choose mime type based on the file extension
                    if (extension == NULL)
                    {
                        mime = "text/plain";
                    }
                    else if (strcmp(extension, ".png") == 0)
                    {
                        mime = "image/png";
                    }
                    else if (strcmp(extension, ".jgp") == 0)
                    {
                        mime = "image/jpg";
                    }
                    else if (strcmp(extension, ".gif") == 0)
                    {
                        mime = "image/gif";
                    }
                    else if (strcmp(extension, ".html") == 0)
                    {
                        mime = "text/html";
                    }
                    else if (strcmp(extension, ".css") == 0)
                    {
                        mime = "text/css";
                    }
                    else
                    {
                        mime = "text/plain";
                    }

                    libwebsockets_serve_http_file(wsi, resource_path, mime);
                }
            }

            // close  connection
            libwebsocket_close_and_free_session(context, wsi, 
                                                LWS_CLOSE_STATUS_NORMAL);
            break;
        }
        default:
            printf("%d\t:unhandled callback\n", reason);
            break;
    }
    return 0;
}

// list of supported protocols and callbacks
static struct libwebsocket_protocols protocols[] = {
    // first protocol must always be HTTP handler
    {
        "http-only",        // name
        callback_http,      // callback
        0                   // per_session_data_size
    },
    {
        NULL, NULL, 0       // end of list
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

