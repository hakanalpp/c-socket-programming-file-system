// Hakan Alp - 250201056

#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <netdb.h>

#include "../../common/helper.h"
#include "server_helper.h"

#define SERVERPORT 8888
#define MAXBUF 1024
#define MAXSTRING 128

struct stat st = {0};

char *dir_array_to_string(char **dirs) {
    char *output = malloc(sizeof(char) * MAXBUF);
    char buffer[64];
    strcpy(output, "");

    int i = 1;
    while (strcmp(dirs[i - 1],"") != 0 && dirs[i-1] != NULL) {
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
        printf("There is no such user named: '%s'.\n", path);
        closedir(d);
        return NULL;
    }

    while ((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, "..") == 0 || strcmp(dir->d_name, ".") == 0 || strncmp(dir->d_name, ".", 1) == 0) {
            continue;
        }
        files[count] = (char *)malloc(strlen(dir->d_name));
        strcpy(files[count], dir->d_name);
        count++;
    }
    files[count] = (char *)malloc(strlen(""));
    strcpy(files[count], "");

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

/* Gathered from: https://stackoverflow.com/a/7666577/11107343 */
unsigned long hash(char *str) {
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

void welcomeClient(int childSocket, struct sockaddr_in *clientName, char* status) {
    int childPort = ntohs(clientName->sin_port);

    fprintf(stdout, "Client (127.0.0.1:%d) %s", childPort, status);
}

void process_client(int socket2, char *current_path) {
    int option;
    int file_number;
    char buf[MAXBUF];
    char buf2[MAXBUF];
    char username[MAXBUF];
    char password[MAXBUF];

    while (1) {
        write_str(socket2, "INPUT0-Please choose one below:\n0. Exit\n1. Login\n2. Register\n");
        option = read_int(socket2);
        if (option == -1) return;
        
        if (option < 3) {
            break;
        }
    }

    if (option == 0) {
        write_str(socket2, "INFO00-Goodbye.\n");
        return;
    }
    if (option == 2) {
        write_str(socket2, "INPUT1-Please enter a username:\n");
        while (1) {
            strcpy(username, read_str(socket2));
            if(strcmp(username, "") == 0)
                return;

            strcpy(buf, "root/");
            strncat(buf, username, strlen(username));
            if (folder_exists(buf)) {
                write_str(socket2, "INPUT1-Username already exists. Please provide new username.\n");
                continue;
            }
            break;
        }
        write_str(socket2, "INPUT1-Please enter a password:\n");
        strcpy(password, read_str(socket2));
        if(strcmp(password, "") == 0)
            return;

        mkdir(buf, 0777);
        unsigned long hashed_pw = hash(password);
        sprintf(buf2, "%s/.%lu", buf, hashed_pw);
        int fd = open(buf2, O_RDWR | O_CREAT, 0777);
        close(fd);

        printf("%s registered succesfully.\n\n", username);
    }

    write_str(socket2, "INPUT1-Please enter your username:\n");
    strcpy(username, read_str(socket2));
    if(strcmp(username, "") == 0)
        return;
    
    char **arr = list_dir_as_array(current_path, username);
    if (arr == NULL) {
        write_str(socket2, "ERROR0-There is no account with this username.\n");
        return;
    }

    write_str(socket2, "INPUT1-Please enter your password:\n");
    while (1) {
        strcpy(password, read_str(socket2));
        if(strcmp(password, "") == 0)
            return;

        sprintf(buf2, "root/%s/.%lu\0", username, hash(password));

        if(access(buf2, F_OK) == 0) {
            break;
        } else {
            write_str(socket2, "INPUT1-You enter your password wrong. Please enter again.\n");
        }
    }

    printf("%s logined succesfully.\n\n", username);
    while (1) {
        arr = list_dir_as_array(current_path, username);

        int size = get_dir_size(arr);
        if (size == 0) {
            write_str(socket2, "INPUT0-You do not have any file. Press 0 to upload new file:\n");
        }
        else {
            char *k = dir_array_to_string(arr);

            strcpy(buf, "INPUT0-Hello! Here are your files!\n\nPlease give number to proceed, or 0 to upload new file:");
            write_str(socket2, strcat(buf, k));
        }

        while (1) {
            file_number = read_int(socket2);
            if (file_number == -1) return;

            if (size == 0 && file_number != 0) {
                write_str(socket2, "INPUT0-You can only upload new files. Press 0 to upload new file:\n");
                continue;
            }
            else if (file_number >= size) {
                write_str(socket2, "INPUT0-Please select a valid file. Press 0 to upload a new file:\n");
                continue;
            }
            break;
        }

        if (file_number == 0) {
            write_str(socket2, "INPUT2-Please enter a file name.\n");
            sprintf(buf, "%s/root/%s", current_path, username);
            if (read_file(socket2, buf) == -1) return;
            continue;
        }

        write_str(socket2, "INPUT0-Please choose one below:\n0. Exit\n1. Download\n2. Delete\n");

        int file_option = -1;
        while (1) {
            file_option = read_int(socket2);
            if (file_option == -1) return;

            if (file_option == 0) {
                write_str(socket2, "INFO00-Goodbye.\n");
                break;
            }
            else if (file_option > 2) {
                write_str(socket2, "INPUT0-Please select a valid option.\n");
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
                printf("'%s' is deleted successfully.\n\n", arr[file_number - 1]);
                write_str(socket2, "INFO00-The file is deleted successfully.\n\n");
            }
            else {
                printf("'%s' is could not deleted.\n\n", arr[file_number - 1]);
                write_str(socket2, "ERROR0-The file could not deleted.\n\n");
            }
        }
    }
}
