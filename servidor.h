#ifndef _servidor_h_
#define _servidor_h_

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pwd.h>
#include <string.h>

#include "estrutura.h"

#define FILE_SIZE 4096
#define MAX_SIZE 1024
#define SIZE 256
#define MAX_FILHOS 5

int main();
void inicia_pipe();
int backup(Ficheiro f);
int restore(Ficheiro f, int open_pipe_cliente);
void morte(int pid);

#endif
