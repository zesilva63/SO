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

#define PIPE_PATH "/tmp/sobupipe"
#define SIZE 128

int main(int argc, char** argv) {

    int open_pipe, i, wr;
    char buffer[SIZE]; /* onde colocar o comando */


    open_pipe = open(PIPE_PATH, O_WRONLY); /* abrir o pipe para escrita */

    if(argc == 1) printf("Nenhuma acção foi especificada\n");
    else if(argc == 2) printf("Nenhum ficheiro foi especificado\n");
    else {

        if(strcmp(argv[1],"exit") == 0) return 0;
        else if(strcmp(argv[1],"backup") == 0 || strcmp(argv[1],"restore") == 0) {

            for(i = 2; i < argc; i++) {
                sprintf(buffer,"%s %s",argv[1],argv[i]);
                wr = write(open_pipe,buffer,strlen(buffer)+1);
            }

        } else {
            printf("Comando Inválido\n");
        }
    }

    close(open_pipe);

    return 0;
}
