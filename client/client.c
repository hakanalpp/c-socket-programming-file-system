// Hakan Alp - 250201056

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>


#include "../common/helper.h"

#define SERVERPORT 8888
#define MAXBUF 1024

int counter;
int socket1;
char buf[MAXBUF];
int fd;
int returnStatus;


int read_data() {
    // Read as long as there is data
    while ((counter = read(socket1, buf, MAXBUF)) > 0) {
        char mes[7];
        strncpy(mes, buf, 6);
        chop_n_chars(buf, 7);

        printf(buf);
        if (strcmp(mes, "INPUT0\0") == 0) {    
            return 1;
        }
        if (strcmp(mes, "INPUT1\0") == 0) {
            return 2;
        }
        if (strcmp(mes, "INPUT2\0") == 0) {
            return 3;
        }
        if (strcmp(mes, "INFO01\0") == 0) {
            return 4;
        }
        return 0;
    }
    return -1;
}


int main(int argc, char* argv[]) {
    struct sockaddr_in server;

    /* create a socket */
    socket1 = socket(AF_INET, SOCK_STREAM, 0);

    if (socket1 == -1) {
        fprintf(stderr, "Could not create socket!\n");
        exit(1);
    }

    /* set up the server information */
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(SERVERPORT);

    /* connect to the server */
    returnStatus =
        connect(socket1, (struct sockaddr*)&server, sizeof(server));

    if (returnStatus == -1) {
        fprintf(stderr, "Could not connect to server!\n");
        exit(1);
    }

    char current_path[MAXBUF];
    getcwd(current_path, sizeof(current_path));

    int res_type = 0;
    char * input = malloc(sizeof (char) * MAXBUF);
    int num;
    char term;
    while(1) {
        res_type = read_data();

        if (res_type == -1) {
            break;
        }

        if (res_type == 2) {
            printf("$ \0");
            scanf("%s", input);
            printf("\n");
            write_str(socket1, input);
        }
        if (res_type == 1) {
            int num;
            char term;
            while(1) {
                printf("$ \0");
                if (scanf("%d%c", &num, &term) != 2 || term != '\n') {
                    printf("Please provide a valid integer.\n");
                    scanf("%*s");
                    continue;
                } else
                    break;
            }
            printf("\n");
            write_int(socket1, num);
        }
        if (res_type == 3) {
            printf("$ \0");
            scanf("%s", input);
            printf("\n");
            write_file(socket1, current_path, input);
        }
        if (res_type == 4) {
            read_file(socket1, current_path);
        }
    }

    close(socket1);
    return 0;
}
