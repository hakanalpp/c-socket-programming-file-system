#ifndef HELPER_H_   /* Include guard */
#define HELPER_H_

char * read_str(int socket);

int read_int(int socket);

int read_file(int socket, char * filepath);

int write_str(int socket, char * data);

int write_int(int socket, int data);

int write_file(int socket, char* filepath, char * filename);

void chop_n_chars(char *str, size_t n);

#endif