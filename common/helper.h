#ifndef HELPER_H_   /* Include guard */
#define HELPER_H_

char * read_str(int socket);

int read_int(int socket);

int read_file(int socket, char * file);

int write_str(int socket, char * data);

int write_int(int socket, int data);

int write_file(int socket, char * file);

#endif