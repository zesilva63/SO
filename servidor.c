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


void inicia_diretorias(char* homedir);
void inicia_pipe();
char* get_codigo(char* ficheiro, char* codigo);


int main() {

    int open_pipe, res_server;
    char buffer[SIZE];
    char* comando;
    char* ficheiro;
    char* codigo = malloc(SIZE*sizeof(char));

    char* homedir = getenv("HOME");

    inicia_diretorias(homedir); /* cria as pastas .Backup e data e metadata dentro desta */

    inicia_pipe();
    open_pipe = open(PIPE_PATH, O_RDONLY); /* abrir o pipe para leitura */

    while(1) {
        res_server = read(open_pipe,buffer,SIZE);
        if(res_server) {

            if(!fork()) {
                comando = strtok(buffer," \n");
                ficheiro = strtok(NULL," \n");
                codigo = get_codigo(ficheiro,codigo);
            }else {
                wait(NULL);
            }

        }
    }

    close(open_pipe);
    unlink(PIPE_PATH);

    return 0;
}


void inicia_pipe() {
    int res_pipe;
    res_pipe = mkfifo(PIPE_PATH, 0744); /* criar o pipe */
//    if (res_pipe < 0) printf("Não foi possivel criar o pipe.\n");
}


void inicia_diretorias(char* homedir) {

    char backup_folder[SIZE];
    char metadata_folder[SIZE];
    char data_folder[SIZE];

    sprintf(backup_folder,"%s/.Backup",homedir);
    mkdir(backup_folder,0700);
    sprintf(metadata_folder,"%s/.Backup/metadata",homedir);
    mkdir(metadata_folder,0700);
    sprintf(data_folder,"%s/.Backup/data",homedir);
    mkdir(data_folder,0700);
}

char* get_codigo(char* ficheiro,char* codigo) {

    int pfd[2];
    pipe(pfd);

    if(fork()==0) {

        close(pfd[0]);
        dup2(pfd[1],1);
        close(pfd[1]);
        execlp("sha1sum","sha1sum",ficheiro,NULL);
        _exit(1);

    } else {
        close(pfd[1]);
        dup2(pfd[0],0);
        close(pfd[0]);
        wait(NULL);
        read(0,codigo,SIZE);
    }

    return codigo;
}
