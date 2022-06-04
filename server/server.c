// Hakan Alp - 250201056

/* File transfer server */

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

#include "../common/helper.h"

#define SERVERPORT 8888
#define MAXBUF 1024
#define MAXSTRING 128

struct stat st = {0};

char *dir_array_to_string(char **dirs) {
    char *output = malloc(sizeof(char) * MAXBUF);
    char buffer[64];
    strcpy(output, "");

    int i = 1;
    while (dirs[i - 1] != NULL) {
        sprintf(buffer, "\n%d. ", i);
        output = strcat(output, buffer);
        output = strcat(output, dirs[i - 1]);
        i++;
    }
    output = strcat(output, "\n");

    return output;
}

char **list_dir_as_array(char *cwp, char *path) {
    DIR *d;
    struct dirent *dir;
    char **files = malloc(MAXBUF * sizeof(char *));
    char *new_path = malloc(sizeof(char) * MAXBUF);
    int count = 0;

    strcpy(new_path, cwp);
    strcat(new_path, "/root/");
    strcat(new_path, path);

    d = opendir(new_path);
    if (!d) {
        printf("Failed opening as array: '%s'.\n", new_path);
        closedir(d);
        return NULL;
    }

    while ((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, "..") == 0 || strcmp(dir->d_name, ".") == 0)
        {
            continue;
        }
        files[count] = (char *)malloc(strlen(dir->d_name) + 1);
        strcpy(files[count], dir->d_name);
        count++;
    }

    closedir(d);

    return files;
}

int get_dir_size(char **dirs) {
    int i = 0;
    while (dirs[i] != NULL) {
        i++;
    }
    return i;
}

int folder_exists(char *path) {
    return stat(path, &st) == 00 && S_ISDIR(st.st_mode);
}

void process_client(int socket2, char *current_path) {
    int option;
    int file_number;
    char buf[MAXBUF];
    char username[MAXBUF];
    char password[MAXBUF];

    while (1) {
        write(socket2, "INPUT0-Please choose one below:\n0. Exit\n1. Login\n2. Register\n", 62);
        option = read_int(socket2);
        if (option < 3) {
            break;
        }
    }

    if (option == 0) {
        write(socket2, "INFO00-Goodbye.\n", 17);
        return;
    }
    if (option == 2) {
        write(socket2, "INPUT1-Please enter a username:\n", 33);
        while (1)
        {
            strcpy(username, read_str(socket2));

            strcpy(buf, "root/");
            strncat(buf, username, strlen(username));
            if (folder_exists(buf))
            {
                write(socket2, "INPUT1-Username already exists. Please provide new username.\n", 62);
                continue;
            }
            break;
        }
        mkdir(buf, 0700);
    }

    write(socket2, "INPUT1-Please enter your username:\n", 36);
    strcpy(username, read_str(socket2));

    // write(socket2, "INPUT1-Please enter your password\n", 36);
    // strcpy(password, read_str(socket2));

    char **arr = list_dir_as_array(current_path, username);
    int flag = 0;
    if (arr == NULL) {
        write(socket2, "ERROR0-There is no account with this username.\n", 48);
        return;
    }

    while (1) {
        int size = get_dir_size(arr);
        if (size == 0) {
            write(socket2, "INPUT0-You do not have any file. Press 0 to upload new file:\n", 62);
        }
        else {
            char *k = dir_array_to_string(arr);

            strcpy(buf, "INPUT0-\nHello! Here are your files!\n\nPlease give number to proceed, or 0 to upload new file:");
            write(socket2, strcat(buf, k), strlen(k) + strlen(buf));
        }

        while (1) {
            file_number = read_int(socket2);
            if (size == 0 && file_number != 0) {
                write(socket2, "INPUT0-You can only upload new files. Press 0 to upload new file:\n", 67);
                continue;
            }
            else if (file_number > size) {
                write(socket2, "INPUT0-Please select a valid file. Press 0 to upload a new file:\n", 66);
                continue;
            }
            break;
        }

        if (file_number == 0) {
            write(socket2, "INPUT2-Please enter a file name.\n", 34);
            sprintf(buf, "%s/root/%s/\0", current_path, username);
            read_file(socket2, buf);
            continue;
        }

        write(socket2, "INPUT0-Please choose one below:\n0. Exit\n1. Download\n2. Delete\n", 63);

        int file_option = -1;
        while (1) {
            file_option = read_int(socket2);
            if (file_option == 0) {
                write(socket2, "INFO00-Goodbye.\n", 17);
                break;
            }
            else if (file_option > 2) {
                write(socket2, "INPUT0-Please select a valid option.\n", 38);
                continue;
            }
            break;
        }

        if (file_option == 1) {
            sprintf(buf, "INFO01-\n", arr[file_number - 1]);
            write_str(socket2, buf);
            sprintf(buf, "%s/root/%s", current_path, username);
            write_file(socket2, buf, arr[file_number - 1]);
            continue;
        }

        else if (file_option == 2) {
            strcpy(buf, "");
            sprintf(buf, "%s/root/%s/%s\0", current_path, username, arr[file_number - 1]);
            if (remove(buf) == 0) {
                printf("%s is deleted successfully.\n", arr[file_number - 1]);
                write(socket2, "INFO00-The file is deleted successfully.\n", 42);
            }
            else {
                printf("%s is deleted successfully.\n", arr[file_number - 1]);
                write(socket2, "INFO00-The file is not deleted.\n", 33);
            }
        }
    }
}


int main() {
    int socket1, socket2;
    socklen_t addrlen;
    struct sockaddr_in xferServer, xferClient;
    int returnStatus;

    /* create a socket */
    socket1 = socket(AF_INET, SOCK_STREAM, 0);
    int true = 1;
    setsockopt(socket1, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(int));

    if (socket1 == -1)
    {
        fprintf(stderr, "Could not create socket!\n");
        exit(1);
    }

    /* bind to a socket, use INADDR_ANY for all local addresses */
    xferServer.sin_family = AF_INET;
    xferServer.sin_addr.s_addr = INADDR_ANY;
    xferServer.sin_port = htons(SERVERPORT);

    returnStatus = bind(socket1, (struct sockaddr *)&xferServer, sizeof(xferServer));

    if (returnStatus == -1)
    {
        fprintf(stderr, "Could not bind to socket!\n");
        exit(1);
    }

    returnStatus = listen(socket1, 5);

    if (returnStatus == -1)
    {
        fprintf(stderr, "Could not listen on socket!\n");
        exit(1);
    }

    /* checks & creates a root foolder */
    printf("%s \n", folder_exists("root") ? "Exists" : "Does not exist");
    if (folder_exists("root") == -1)
    {
        mkdir("root", 0700);
    }

    char current_path[MAXBUF];
    getcwd(current_path, sizeof(current_path));

    while (1)
    {
        /* wait for an incoming connection */
        addrlen = sizeof(xferClient);

        /* use accept() to handle incoming connection requests        */
        /* and free up the original socket for other requests         */
        socket2 = accept(socket1, (struct sockaddr *)&xferClient, &addrlen);

        if (socket2 == -1)
        {
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
