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
#define SIZE 256

void copiado();
void recuperado();
void morte(int pid);

char* ficheiro;
int vivos;

int main(int argc, char** argv) {

    int open_pipe, i, res_write;
    char buffer[SIZE];

    signal(SIGCHLD,morte);
    signal(SIGUSR1,copiado);
    signal(SIGUSR2,recuperado);

    open_pipe = open(PIPE_PATH, O_WRONLY); /* abrir o pipe para escrita */

    if(argc == 1) printf("Nenhuma acção foi especificada\n");
    else if(argc == 2) printf("Nenhum ficheiro foi especificado\n");
    else {

        if(strcmp(argv[1],"exit") == 0) return 0;
        else if(strcmp(argv[1],"backup") == 0 || strcmp(argv[1],"restore") == 0) {

            for(i = 2; i < argc; i++) {
                vivos++;
                if(!fork()) {
                    sprintf(buffer,"%s %s %d",argv[1],argv[i],getpid());
                    res_write = write(open_pipe,buffer,strlen(buffer)+1);
                    ficheiro = argv[i];
                    pause();
                    _exit(0);
                }
            }
            while(vivos > 0) wait(NULL);

        } else {
            printf("Comando Inválido\n");
        }
    }

    close(open_pipe);

    return 0;
}


void copiado() {
    printf("%s: copiado\n",ficheiro);
}


void recuperado() {
    printf("%s: recuperado\n",ficheiro);
}

void morte(int pid) {
    waitpid(pid, NULL, WCONTINUED);
    vivos--;
}
