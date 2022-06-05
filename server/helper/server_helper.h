#ifndef SERVER_HELPER_H_   /* Include guard */
#define SERVER_HELPER_H_

char *dir_array_to_string(char **dirs);

char **list_dir_as_array(char *cwp, char *path);

int get_dir_size(char **dirs);

int folder_exists(char *path);

unsigned long hash(char *str);

void process_client(int socket2, char *current_path);

void welcomeClient(int childSocket, struct sockaddr_in *clientName, char* status);

#endif