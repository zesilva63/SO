
#include "cliente.h"

char* ficheiro;
int erro;


int main(int argc, char** argv) {

    int open_pipe, open_pipe_cliente, i,res_pipe_cliente, res_write, res_read, open_file, tam, pid;
    char buffer[FILE_SIZE], pipe_path[SIZE], pipe_restore_path[SIZE];
    Ficheiro f = inicia_ficheiro();

    signal(SIGUSR1,correto);
    signal(SIGUSR2,falha);

    char* homedir = getenv("HOME");
    sprintf(pipe_path,"%s/.Backup/sobupipe",homedir);
    open_pipe = open(pipe_path, O_WRONLY);


    if(argc == 1) printf("Nenhuma acção foi especificada\n");

    else if(strcmp(argv[1],"backup") == 0) {

        for(i = 2; i < argc; i++) {

            if(access(argv[i], F_OK ) != -1 ) {

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
                }

            } else {
                printf("O ficheiro %s não existe\n",argv[i]);
            }
        }
        while(vivos > 0) wait(NULL);
    }
    else if(strcmp(argv[1],"restore") == 0) {

        sprintf(pipe_restore_path,"%s/.Backup/soburestore",homedir);

        for(i = 2; i < argc; i++) {

            ficheiro = argv[i];
            f = altera_ficheiro_cliente(f,argv[1],argv[i],getpid(),0);
            res_write = write(open_pipe,f,sizeof(*f));

            pause();

            if(erro == 1) {
                printf("O ficheiro %s não existe\n", ficheiro);
                continue;
            }else {

                open_pipe_cliente = open(pipe_restore_path, O_RDONLY);

                f->estado = 1;
                while(f->estado == 1) {
                    res_read = read(open_pipe_cliente,f,sizeof(*f));
                    if(res_read) {
                        open_file = open(f->ficheiro, O_CREAT | O_APPEND | O_WRONLY, 0600);
                        res_write = write(open_file,f->conteudo,f->tamanho);
                        close(open_file);
                    }
                }
                printf("%s: recuperado\n",f->ficheiro);
                close(open_pipe_cliente);
            }
        }
    }
    else if(strcmp(argv[1],"gc") == 0) {
        if(!fork()) {
            f = altera_ficheiro_gc(f,argv[1],getpid());
            res_write = write(open_pipe,f,sizeof(*f));
            pause();
            if(erro == 1) printf("Falha na limpeza de ficheiros\n");
            else printf("Ficheiros não usados limpos corretamente\n");
            _exit(0);
        }else wait(NULL);
    }
    else if(strcmp(argv[1],"delete") == 0 && argc == 3) {
        if(!fork()) {
            ficheiro = argv[2];
            f = altera_ficheiro_cliente(f,argv[1],argv[2],getpid(),1);
            res_write = write(open_pipe,f,sizeof(*f));
            pause();
            if(erro == 1) printf("Falha na remoção do ficheiro %s",ficheiro);
            else printf("%s: removido",ficheiro);
            _exit(0);
        }else wait(NULL);
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

void correto() {
    erro = 0;
}
