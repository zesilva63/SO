
#include "cliente.h"

char* ficheiro;
int vivos;
int erro;


int main(int argc, char** argv) {

    int open_pipe, open_pipe_cliente, i,res_pipe_cliente, res_write, res_read, open_file, tam, pid;
    char buffer[FILE_SIZE], pipe_path[SIZE], pipe_restore_path[SIZE];
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
                    if(erro == 1) printf("Falha no backup do ficheiro %s\n",ficheiro);
                    else printf("%s: copiado\n",ficheiro);
                    _exit(0);
                }else wait(NULL);

            } else {
                printf("O ficheiro %s não existe\n",argv[i]);
            }
        }

    }
    else if(strcmp(argv[1],"restore") == 0) {

        sprintf(pipe_restore_path,"%s/.Backup/soburestore",homedir);
        res_pipe_cliente = mkfifo(pipe_restore_path,0744);

        for(i = 2; i < argc; i++) {

            vivos++;

                ficheiro = argv[i];
                pid = getpid();
                f = altera_ficheiro_cliente(f,argv[1],argv[i],getpid(),0);
                res_write = write(open_pipe,f,sizeof(*f));

                open_pipe_cliente = open(pipe_restore_path, O_RDONLY);

                f->estado = 1;
                while(f->estado == 1) { // confirmar que funciona
                    res_read = read(open_pipe_cliente,f,sizeof(*f));
                    if(res_read) {
                        open_file = open(f->ficheiro, O_CREAT | O_APPEND | O_WRONLY, 0600);
                        res_write = write(open_file,f->conteudo,f->tamanho);
                        close(open_file);
                    }
                }
        }
        close(open_pipe_cliente);
        unlink(pipe_restore_path);

        printf("%s: recuperado\n",f->ficheiro);

    }
    else {
        printf("Comando Inválido\n");
    }

    close(open_pipe_cliente);
    close(open_pipe);

    return 0;
}


void falha() {
    erro = 1;
}

void copiado() {
    erro = 0;
}


void recuperado() {
    erro = 0;
}

void morte(int pid) {
    waitpid(pid, NULL, WCONTINUED);
    vivos--;
}
