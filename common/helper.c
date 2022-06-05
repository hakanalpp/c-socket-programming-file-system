// Hakan Alp - 250201056

#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define MAXBUF 1024
#define MAXSTRING 128

int i, readCounter, writeCounter;

/* Gathered from https://stackoverflow.com/a/4761840/11107343 */
void chop_n_chars(char *str, size_t n)
{
    size_t len = strlen(str);
    if (n > len)
        return;  // Or: n = len;
    memmove(str, str+n, len - n + 1);
}


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
    char filename[MAXSTRING];
    int counter;

    strcpy(filename, read_str(socket));
    if (strncmp(filename, "ERROR", 5) == 0) {
        chop_n_chars(filename, 7);
        printf("%s \n", filename);
        return -1;
    }

    sprintf(buf, "%s/%s\0", filepath, filename);

    printf("Downloading '%s' \n", buf);

    remove(buf);
    FILE *file = fopen(buf, "w+");

    if (file == NULL) {
        fprintf(stderr, "Could not open destination file, using stdout.\n");
        return -1;
    }

    read(socket, buf, MAXBUF);
    fprintf(file, buf);
    
    fclose(file);
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

    const char* c = path;
    if (*(c-1) == '/') return 0;

    return 1;
}


int write_file(int socket, char* filepath, char * filename) {
    char * bufptr;
    char buf[MAXBUF];
    
    sprintf(buf, "%s/%s\0", filepath, filename);

    if (!is_filename_safe(buf)) {
        fprintf(stderr, "Filename is invalid! '%s'\n", buf);
        write_str(socket, "ERROR0-Filename is invalid!\n");
        return -1;
    }

    printf("Reading '%s'\n", buf);

    
    /* open the file for reading */
    FILE *file = fopen(buf, "r");

    if (file == NULL) {
        fprintf(stderr, "Could not open file for reading!\n");
        write_str(socket, "ERROR0-Could not open file for reading!\n");
        return -1;
    }
    write_str(socket, filename);


    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    fread(buf, fsize, 1, file);
    fclose(file);

    buf[fsize] = '\0';

    write(socket, buf, fsize+1);
    return 1;
}
