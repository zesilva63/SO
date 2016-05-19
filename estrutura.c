
 #include "estrutura.h"

 Ficheiro inicia_ficheiro() {
     Ficheiro f = malloc(sizeof(struct ficheiro));
     return f;
 }


 Ficheiro altera_ficheiro(Ficheiro f, char* comando, char* ficheiro, int pid, char conteudo[], int estado, int tamanho) {
     char* hash = malloc(SIZE*sizeof(char));
     char* codigo;
     hash = get_codigo(ficheiro,hash);
     codigo = strtok(hash," ");

     f->pid_cliente = pid;
     strcpy(f->ficheiro,ficheiro);
     strcpy(f->comando,comando);
     strcpy(f->codigo,codigo);
     memcpy(f->conteudo,conteudo,tamanho);
     f->estado = estado;
     f->tamanho = tamanho;

     return f;
 }

 Ficheiro altera_ficheiro_servidor(Ficheiro f, char* comando, char* ficheiro, int pid, char conteudo[], int estado, int tamanho) {

     f->pid_cliente = pid;
     strcpy(f->ficheiro,ficheiro);
     strcpy(f->comando,comando);
     memcpy(f->conteudo,conteudo,tamanho);
     f->estado = estado;
     f->tamanho = tamanho;

     return f;
 }

 Ficheiro altera_ficheiro_cliente(Ficheiro f, char* comando, char* ficheiro, int pid, int estado) {

     f->pid_cliente = pid;
     strcpy(f->comando,comando);
     strcpy(f->ficheiro,ficheiro);
     f->estado = estado;

     return f;
 }

 char* get_codigo(char ficheiro[], char* codigo) {

     int pfd[2];
     int res_pipe, res_read;
     res_pipe = pipe(pfd);

         if(!fork()) {
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
             res_read = read(0,codigo,SIZE);
         }

     return codigo;
 }
