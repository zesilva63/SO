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

#define FILE_SIZE 4096
#define SIZE 256

int main(int argc, char** argv);
void correto();
void morte(int pid);
void falha();

#endif
