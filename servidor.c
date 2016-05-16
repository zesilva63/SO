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
#define MAX_FILHOS 5

/* ESTRUTURA */

typedef struct ficheiro {
    pid_t pid_cliente;
    char* codigo;
    char* ficheiro;
    char* comando;
}*Ficheiro;

int filhos_vivos;


/* FUNÇÕES */

void inicia_pipe();
char* get_codigo(char* ficheiro, char* codigo);
Ficheiro altera_ficheiro(Ficheiro f,char buffer[]);
Ficheiro inicia_ficheiro();
int backup(Ficheiro f);
void restore(Ficheiro f);
void morte(int pid);

int main() {

    if(!fork()) {

        signal(SIGCHLD,morte);

        int open_pipe, res_server, res_pipe, res_comando;
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

                if(filhos_vivos == MAX_FILHOS) pause();

                filhos_vivos++;
                f = altera_ficheiro(f,buffer);

                if(!fork()) {
                    if(strcmp(f->comando,"backup") == 0) {
                        res_comando = backup(f);
                        if(res_comando) kill(f->pid_cliente,SIGUSR1);
                        else kill(f->pid_cliente,SIGINT);
                    }
                    else if(strcmp(f->comando,"restore") == 0) {
                        restore(f);
                        kill(f->pid_cliente,SIGUSR2);
                    }
                    _exit(0);
                }
            }
        }

        close(open_pipe);
        unlink(PIPE_PATH);

    }
    return 0;
}



void morte(int pid) {
    waitpid(pid, NULL, WCONTINUED);
    filhos_vivos--;
}



void restore(Ficheiro f) {
    printf("restaurado\n");
}



int backup(Ficheiro f) {
    int mudar_nome;
    int resultado = 0;
    int pfd[2];
    pipe(pfd);

    char* homedir = getenv("HOME");

    char backup_folder[SIZE];
    char metadata_folder[SIZE];
    char data_folder[SIZE];
    char file_coded[SIZE];
    char data_path[SIZE];
    char data_encripted[SIZE];
    char file_metadata[SIZE];

    sprintf(backup_folder,"%s/.Backup",homedir);
    sprintf(metadata_folder,"%s/.Backup/metadata",homedir);
    sprintf(data_folder,"%s/.Backup/data",homedir);
    sprintf(file_coded,"%s/%s",data_folder,f->codigo);
    sprintf(data_path,"%s/%s",data_folder,f->ficheiro);
    sprintf(data_encripted,"%s.gz",data_path);
    sprintf(file_metadata,"%s/%s",metadata_folder,f->ficheiro);

    if(fork()==0) {
        execlp("cp","cp",f->ficheiro,data_folder,NULL);
        perror("Falha na cópia do ficheiro");
        resultado = 1;
        _exit(0);
    } else {
        wait(NULL);
        if(!fork()) {
            execlp("gzip","gzip",data_path,NULL);
            resultado = 1;
            perror("Falha ao comprimir o ficheiro");
            _exit(0);
        }else {
            wait(NULL);
        }
    }

    mudar_nome = rename(data_encripted,file_coded);
    if(mudar_nome == -1) {
         printf("Erro ao alterar o nome do ficheiro %s\n",f->ficheiro);
         resultado = 1;
    }

    if(!fork()) {
        execlp("ln","ln","-s",file_coded,file_metadata,NULL);
        resultado = 1;
        perror("Linkagem do ficheiro para metadata mal sucedida");
        _exit(0);
    }else {
        wait(NULL);
    }


    return resultado;
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
            perror("Falhou a obter o código");
            _exit(1);

        } else {
            wait(NULL);
            close(pfd[1]);
            dup2(pfd[0],0);
            close(pfd[0]);
            read(0,codigo,SIZE);
        }

    return codigo;
}
