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

int main() {
    int socket1, socket2;
    socklen_t addrlen;
    struct sockaddr_in server_address, client_address;
    int returnStatus;

    /* create a socket */
    socket1 = socket(AF_INET, SOCK_STREAM, 0);
    int true = 1;
    setsockopt(socket1, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(int));

    if (socket1 == -1) {
        fprintf(stderr, "Could not create socket!\n");
        exit(1);
    }
    fprintf(stdout, "Socket created!\n");


    /* bind to a socket, use INADDR_ANY for all local addresses */
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(SERVERPORT);

    returnStatus = bind(socket1, (struct sockaddr *)&server_address, sizeof(server_address));

    if (returnStatus == -1) {
        fprintf(stderr, "Could not bind to socket!\n");
        exit(1);
    }
    fprintf(stdout, "Bind Completed!\n");


    returnStatus = listen(socket1, 5);

    if (returnStatus == -1) {
        fprintf(stderr, "Could not listen on socket!\n");
        exit(1);
    }

    int len = sizeof(server_address);
    getsockname(socket1, (struct sockaddr *)&server_address, &len);
    int port = ntohs(server_address.sin_port);
    fprintf(stderr, "Listening port %d\n", port);
    
    /* checks & creates a root foolder */
    printf("%s \n\n", folder_exists("root") ? "Root folder exists." : "Root folder does not exist. Created.");
    if (folder_exists("root") == 0) {
        mkdir("root", 0777);
    }

    char current_path[MAXBUF];
    getcwd(current_path, sizeof(current_path));

    while (1) {
        /* wait for an incoming connection */
        addrlen = sizeof(client_address);

        /* use accept() to handle incoming connection requests        */
        /* and free up the original socket for other requests         */
        socket2 = accept(socket1, (struct sockaddr *)&client_address, &addrlen);

        if (socket2 == -1) {
            fprintf(stderr, "Could not accept connection!\n");
            exit(1);
        }

        welcomeClient(socket1, &client_address, "connected.\n\n");
        process_client(socket2, current_path);
        
        welcomeClient(socket1, &client_address, "disconnected.\n\n");
        
        shutdown(socket2, SHUT_RDWR);
        close(socket2);
    }
    shutdown(socket1, SHUT_RDWR);
    close(socket1);

    return 0;
}