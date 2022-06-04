#include <fcntl.h>
#include <netdb.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>

#define MAXBUF 1024
#define MAXSTRING 128

int i, readCounter, writeCounter;

char * read_str(int socket) {
    char *response = malloc(sizeof(char) * MAXBUF);

    int readCounter = read(socket, response, MAXBUF);

    if (readCounter == -1) {
        fprintf(stderr, "Could not read string data from socket!\n");
        close(socket);
    }

    return response;
}


int read_int(int socket) {
    int response = -1;

    int readCounter = read(socket, &response, sizeof(int));

    if (readCounter == -1)
    {
        fprintf(stderr, "Could not read int data from socket!\n");
        close(socket);
    }

    return response;
}


int read_file(int socket, char * filepath) {
    char buf[MAXBUF];
    int counter;
    // sprintf(buf, "%s/root/%s/%s\0", path, username, filename);
    printf("%s \n", filepath);
    
    int fd = open(filepath, O_WRONLY | O_CREAT | O_APPEND, 0666);

    if (fd == -1) {
        fprintf(stderr, "Could not open destination file, using stdout.\n");
        return -1;
    }

    /* read the file from the socket as long as there is data */
    while ((counter = read(socket, buf, MAXBUF)) > 0) {
        /* send the contents to stdout */
        write(fd, buf, counter);
    }

    if (counter == -1) {
        fprintf(stderr, "Could not read file from socket!\n");
        return -1;
    }
}


int write_str(int socket, char * data) {
    int returnStatus = write(socket, data, strlen(data)+1);

    if (returnStatus == -1) {
        fprintf(stderr, "Could not send string data to server!\n");
        return -1;
    }
}


int write_int(int socket, int data) {
    int returnStatus = write(socket, &data, sizeof(int));

    if (returnStatus == -1) {
        fprintf(stderr, "Could not send int data to server!\n");
        return -1;
    }
}


int is_filename_safe(char* path) {
    if (strstr(path, "..")) return 0;
    if (path[1] == 0 && path[0] == '.') return 0;
    if (path[0] == '/') return 0;

    const char* c = path;
    while (*c != 0) {
        if (!isalnum(*c) && *c != '.' && *c != '/') return 0;
        ++c;
    }
    if (*(c-1) == '/') return 0;

    return 1;
}


int write_file(int socket, char * filepath) {
    char * bufptr;
    char buf[MAXBUF];

    if (!is_filename_safe(filepath)) {
        fprintf(stderr, "Filename is invalid! %s\n", filepath);
        write(socket, "ERROR: Filename is invalid!\n", 29);
        return -1;
    }

    printf("Uploading '%s'\n", filepath);

    write_str(socket, filepath);
    
    /* open the file for reading */
    int fd = open(filepath, O_RDONLY);

    if (fd == -1) {
        fprintf(stderr, "Could not open file for reading!\n");
        write(socket, "ERROR: Could not open file for reading!\n", 41);
        return -1;
    }

    /* reset the read counter */
    int readCounter, writeCounter = 0;

    /* read the file, and send it to the client in chunks of size MAXBUF */
    while ((readCounter = read(fd, buf, MAXBUF)) > 0) {
        writeCounter = 0;
        bufptr = buf;

        while (writeCounter < readCounter) {
            readCounter -= writeCounter;
            bufptr += writeCounter;
            writeCounter = write(socket, bufptr, readCounter);

            if (writeCounter == -1) {
                fprintf(stderr, "Could not write file to client!\n");
                return -1;
            }
        }
    }
    return 1;
}