
#include "cliente.h"

char* ficheiro;
int vivos;


int main(int argc, char** argv) {

    int open_pipe, i, res_write, open_file, tam;
    char buffer[FILE_SIZE], pipe_path[SIZE];
    Ficheiro f = inicia_ficheiro();

    signal(SIGQUIT,falha);
    signal(SIGCHLD,morte);
    signal(SIGUSR1,copiado);
    signal(SIGUSR2,recuperado);

    char* homedir = getenv("HOME");
    sprintf(pipe_path,"%s/.Backup/sobupipe",homedir);
    open_pipe = open(pipe_path, O_WRONLY);

    if(argc == 1) printf("Nenhuma acção foi especificada\n");

    else if(argc == 2) printf("Nenhum ficheiro foi especificado\n");

    else if(strcmp(argv[1],"backup") == 0) {

        for(i = 2; i < argc; i++) {

            if(access(argv[i], F_OK ) != -1 ) {
                vivos++;

                if(!fork()) {
                    ficheiro = argv[i];
                    open_file = open(argv[i],O_RDONLY);

                    while((tam = read(open_file,buffer,FILE_SIZE)) > 0) {
                        f = altera_ficheiro(f,argv[1],argv[i],getpid(),buffer,1,tam);
                        res_write = write(open_pipe,f,sizeof(*f));
                    }

                    f = altera_ficheiro(f,argv[1],argv[i],getpid(),buffer,0,0);
                    res_write = write(open_pipe,f,sizeof(*f));
                    close(open_file);
                    pause();
                    _exit(0);
                }

            } else {
                printf("O ficheiro %s não existe\n",argv[i]);
            }

        }
        while(vivos > 0) wait(NULL);

    } else if(strcmp(argv[1],"restore") == 0) {
        // CENAS DO RESTORE
    } else {
        printf("Comando Inválido\n");
    }

    close(open_pipe);

    return 0;
}


void falha() {
    printf("Falha na operação do ficheiro %s\n",ficheiro);
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
