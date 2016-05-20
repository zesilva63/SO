#ifndef _estrutura_h_
#define _estrutura_h_

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

#define SIZE 256
#define FILE_SIZE 4096

typedef struct ficheiro {
    pid_t pid_cliente;
    char comando[SIZE];
    char ficheiro[SIZE];
    char codigo[SIZE];
    char conteudo[FILE_SIZE];
    int estado;
    int tamanho;
} *Ficheiro;


Ficheiro inicia_ficheiro();
Ficheiro altera_ficheiro(Ficheiro f, char* comando, char* ficheiro, int pid, char conteudo[], int estado, int tamanho);
Ficheiro altera_ficheiro_cliente(Ficheiro f, char* comando, char* ficheiro, int pid, int estado);
Ficheiro altera_ficheiro_servidor(Ficheiro f, char* comando, char* ficheiro, int pid, char conteudo[], int estado, int tamanho);
Ficheiro altera_ficheiro_gc(Ficheiro f, char* comando, int pid);
char* get_codigo(char* ficheiro, char* codigo);

#endif
