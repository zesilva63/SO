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


#define PIPE_PATH "/tmp/sobupip"
#define SIZE 256
#define MAX_FILHOS 5


typedef struct ficheiro {
    pid_t pid_cliente;
    char* codigo;
    char* ficheiro;
    char* comando;
}*Ficheiro;



void inicia_pipe();
char* get_codigo(char* ficheiro, char* codigo);
Ficheiro altera_ficheiro(Ficheiro f,char buffer[]);
Ficheiro inicia_ficheiro();
void backup(Ficheiro f);
void restore(Ficheiro f);


int filhos_vivos;


int main() {

    if(!fork()) {

        int open_pipe, res_server, res_pipe;
        char buffer[SIZE];

        char* homedir = getenv("HOME");
        char backup_folder[SIZE];
        char metadata_folder[SIZE];
        char data_folder[SIZE];

        sprintf(backup_folder,"%s/.Backup",homedir);
        mkdir(backup_folder,0777);
        sprintf(metadata_folder,"%s/.Backup/metadata",homedir);
        mkdir(metadata_folder,0777);
        sprintf(data_folder,"%s/.Backup/data",homedir);
        mkdir(data_folder,0777);

        res_pipe = mkfifo(PIPE_PATH, 0744);
        open_pipe = open(PIPE_PATH, O_RDONLY);

        Ficheiro f = inicia_ficheiro();

        while(1) {

            res_server = read(open_pipe,buffer,SIZE);
            if(res_server) {

                f = altera_ficheiro(f,buffer);
                if(strcmp(f->comando,"backup") == 0) {
                    backup(f);
                    kill(f->pid_cliente,SIGINT);
                }
                else if(strcmp(f->comando,"restore") == 0) {
                    restore(f);
                    kill(f->pid_cliente,SIGINT); // mudar sinal, este é da copia
                }
            }
        }

        close(open_pipe);
        unlink(PIPE_PATH);

    }
    return 0;
}


void restore(Ficheiro f) {
    printf("restaurado\n");
}


void backup(Ficheiro f) {

    int pfd[2];
    pipe(pfd);

    char* homedir = getenv("HOME");

    char backup_folder[SIZE];
    char metadata_folder[SIZE];
    char data_folder[SIZE];
    char file_coded[SIZE];
    char data_path[SIZE];
    char data_encripted[SIZE];

    sprintf(backup_folder,"%s/.Backup",homedir);
    sprintf(metadata_folder,"%s/.Backup/metadata",homedir);
    sprintf(data_folder,"%s/.Backup/data",homedir);
    sprintf(file_coded,"%s/%s",data_folder,f->codigo);
    sprintf(data_path,"%s/%s",data_folder,f->ficheiro);
    sprintf(data_encripted,"%s.gz",data_path);

    if(fork()==0) {
        execlp("cp","cp",f->ficheiro,data_folder,NULL);
    } else {
        wait(NULL);
        if(!fork()) {
            execlp("gzip","gzip",data_path,NULL);
        }else {
            wait(NULL);
            if(!fork()) {
                execlp("mv","mv",data_encripted,file_coded,NULL);
            }else {
                wait(NULL);
            }
        }
    }
}




Ficheiro inicia_ficheiro() {
    Ficheiro f = malloc(sizeof(struct ficheiro));
    f->codigo = malloc(SIZE*sizeof(char));
    return f;
}


Ficheiro altera_ficheiro(Ficheiro f,char buffer[]) {
    char* hash = malloc(SIZE*sizeof(char));
    f->comando = strtok(buffer," \n");
    f->ficheiro = strtok(NULL," \n");
    f->pid_cliente =atoi(strtok(NULL," \n"));
    hash = get_codigo(f->ficheiro,hash);
    f->codigo = strtok(hash," ");
    return f;
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
