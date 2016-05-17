#ifndef _cliente_h_
#define _cliente_h_

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "estrutura.h"

#define PIPE_PATH "/tmp/sobupipe"
#define SIZE 256

int main(int argc, char** argv);
void copiado();
void recuperado();
void morte(int pid);
void falha();

#endif
