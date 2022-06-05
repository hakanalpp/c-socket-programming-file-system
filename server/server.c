// Hakan Alp - 250201056

#include <netdb.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "../common/helper.h"
#include "helper/server_helper.h"

#define SERVERPORT 8888
#define MAXBUF 1024
#define MAXSTRING 128

int main() {
    int socket1, socket2;
    socklen_t addrlen;
    struct sockaddr_in xferServer, xferClient;
    int returnStatus;

    /* create a socket */
    socket1 = socket(AF_INET, SOCK_STREAM, 0);
    int true = 1;
    setsockopt(socket1, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(int));

    if (socket1 == -1) {
        fprintf(stderr, "Could not create socket!\n");
        exit(1);
    }

    /* bind to a socket, use INADDR_ANY for all local addresses */
    xferServer.sin_family = AF_INET;
    xferServer.sin_addr.s_addr = INADDR_ANY;
    xferServer.sin_port = htons(SERVERPORT);

    returnStatus = bind(socket1, (struct sockaddr *)&xferServer, sizeof(xferServer));

    if (returnStatus == -1) {
        fprintf(stderr, "Could not bind to socket!\n");
        exit(1);
    }

    returnStatus = listen(socket1, 5);

    if (returnStatus == -1) {
        fprintf(stderr, "Could not listen on socket!\n");
        exit(1);
    }

    /* checks & creates a root foolder */
    printf("%s \n", folder_exists("root") ? "Exists" : "Does not exist");
    if (folder_exists("root") == 0) {
        mkdir("root", 0777);
    }

    char current_path[MAXBUF];
    getcwd(current_path, sizeof(current_path));

    while (1) {
        /* wait for an incoming connection */
        addrlen = sizeof(xferClient);

        /* use accept() to handle incoming connection requests        */
        /* and free up the original socket for other requests         */
        socket2 = accept(socket1, (struct sockaddr *)&xferClient, &addrlen);

        if (socket2 == -1) {
            fprintf(stderr, "Could not accept connection!\n");
            exit(1);
        }

        process_client(socket2, current_path);

        shutdown(socket2, SHUT_RDWR);
        close(socket2);
    }
    shutdown(socket1, SHUT_RDWR);
    close(socket1);

    return 0;
}
