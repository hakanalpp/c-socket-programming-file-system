// Hakan Alp - 250201056

/* File transfer client */

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "../common/helper.h"

#define SERVERPORT 8888
#define MAXBUF 1024

int counter;
int sockd;
char buf[MAXBUF];
int fd;
int returnStatus;


void chop_n_chars(char *str, size_t n)
{
    size_t len = strlen(str);
    if (n > len)
        return;  // Or: n = len;
    memmove(str, str+n, len - n + 1);
}

int read_data() {
    // Read as long as there is data
    while ((counter = read(sockd, buf, MAXBUF)) > 0) {
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
        return 0;
    }
    return -1;
}


int main(int argc, char* argv[]) {
    struct sockaddr_in xferServer;
    // if (argc < 3) {
    //     fprintf(stderr, "Usage: %s <ip address> <filename> [dest filename]\n", argv[0]);
    //     exit(1);
    // }

    /* create a socket */
    sockd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockd == -1) {
        fprintf(stderr, "Could not create socket!\n");
        exit(1);
    }

    /* set up the server information */
    xferServer.sin_family = AF_INET;
    xferServer.sin_addr.s_addr = inet_addr("127.0.0.1");
    xferServer.sin_port = htons(SERVERPORT);

    /* connect to the server */
    returnStatus =
        connect(sockd, (struct sockaddr*)&xferServer, sizeof(xferServer));

    if (returnStatus == -1) {
        fprintf(stderr, "Could not connect to server!\n");
        exit(1);
    }

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
            write_str(sockd, input);
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
            write_int(sockd, num);
        }
        if (res_type == 3) {
            printf("$ \0");
            scanf("%s", input);
            write_file(sockd, input);
        }
    }

    close(sockd);
    return 0;
}
