
#include "servidor.h"

int filhos_vivos;


int main() {

    signal(SIGCHLD,morte);

    if(!fork()) {

        int open_pipe, open_file, res_server, res_pipe, res_pipe_cliente, res_comando, res_write;
        char buffer[SIZE], backup_folder[SIZE], metadata_folder[SIZE], data_folder[SIZE], data_path[SIZE];
        char file_coded[SIZE];
        char pipe_path[SIZE];
        Ficheiro f = inicia_ficheiro();

        char* homedir = getenv("HOME");

        sprintf(backup_folder,"%s/.Backup",homedir);
        mkdir(backup_folder,0755);
        sprintf(metadata_folder,"%s/.Backup/metadata",homedir);
        mkdir(metadata_folder,0755);
        sprintf(data_folder,"%s/.Backup/data",homedir);
        mkdir(data_folder,0755);

        sprintf(pipe_path,"%s/.Backup/sobupipe",homedir);
        res_pipe = mkfifo(pipe_path, 0744);
        open_pipe = open(pipe_path, O_RDONLY);


        while(1) {

            res_server = read(open_pipe,f,sizeof(*f));
            if(res_server) {
                if(strcmp(f->comando,"backup") == 0) {

                    sprintf(file_coded,"%s/%s",data_folder,f->codigo);

                    if(f->estado == 1 && access(file_coded, F_OK ) == -1) {

                        sprintf(data_path,"%s/%s",data_folder,f->ficheiro);
                        open_file = open(data_path, O_CREAT | O_APPEND | O_WRONLY, 0600);
                        res_write = write(open_file,f->conteudo,f->tamanho);
                        close(open_file);

                    }else {
                        if(filhos_vivos == MAX_FILHOS) pause();

                        filhos_vivos++;

                        if(!fork()) {
                            res_comando = backup(f);
                            if(!res_comando) kill(f->pid_cliente,SIGUSR1);
                            else kill(f->pid_cliente,SIGUSR2);
                            _exit(0);
                        }
                    }
                }
                else if(strcmp(f->comando,"restore") == 0) {

                    if(!fork()) {
                        res_comando = restore(f);
                        _exit(0);
                    }
                }
                else if(strcmp(f->comando,"gc") == 0){

                    if(!fork()) {
                        res_comando = gc(f);
                        if(!res_comando) kill(f->pid_cliente,SIGUSR1);
                        else kill(f->pid_cliente,SIGUSR2);
                        _exit(0);
                    }
                }
                else if(strcmp(f->comando,"delete") == 0) {
                    if(!fork()) {
                        res_comando = delete(f);
                        if(!res_comando) kill(f->pid_cliente,SIGUSR1);
                        else kill(f->pid_cliente,SIGUSR2);
                        _exit(0);
                    }
                }
                else {
                    // MAIS COMANDOS
                }
            } else {
                close(open_pipe);
                open_pipe = open(pipe_path,O_RDONLY);
            }
        }

        unlink(pipe_path);
        _exit(0);
    }
    return 0;
}


int delete(Ficheiro f) {

    int erro = 0;

    char file_metadata[SIZE];
    char* homedir = getenv("HOME");
    sprintf(file_metadata,"%s/.Backup/metadata/%s",homedir,f->ficheiro);

    if(access(file_metadata, F_OK ) != -1) {
        unlink(file_metadata);
    }else erro = 1;

    return erro;
}


int gc(Ficheiro f) {

    int erro = 0;
    int i = 0;
    int pfd[2];
    int res_pipe, res_read;
    char ls_files[MAX_SIZE], data_folder[SIZE], metadata_folder[SIZE], link_path[SIZE];
    char* token;
    char** ficheiros = malloc(100*sizeof(char*));
    char* homedir = getenv("HOME");
    char file_data[SIZE];
    sprintf(data_folder,"%s/.Backup/data",homedir);
    sprintf(metadata_folder,"%s/.Backup/metadata",homedir);

    res_pipe = pipe(pfd);

    if(!fork()) {
        close(pfd[0]);
        dup2(pfd[1],1);
        close(pfd[1]);
        execlp("ls","ls",data_folder,NULL);
        perror("Falhou a obter o código");
        erro = 1;
        _exit(0);
    } else {
        wait(NULL);
        close(pfd[1]);
        dup2(pfd[0],0);
        close(pfd[0]);
        res_read = read(0,ls_files,MAX_SIZE);
    }

    token = strtok(ls_files," \n");

    while( token != NULL ) {
      ficheiros[i] = malloc((strlen(token)+1)*sizeof(char));
      strcpy(ficheiros[i],token);
      i++;
      token = strtok(NULL," \n");
   }
    ficheiros[i] = NULL;

    int j, pfd1[2];
    res_pipe = pipe(pfd1);
    fcntl(pfd1[0], F_SETFL, O_NONBLOCK);

    for(j = 0 ; j < i; j++) {

        if(!fork()) {

            sprintf(file_data,"%s/%s",data_folder,ficheiros[j]);

            if(!fork()) {
                close(pfd1[0]);
                dup2(pfd1[1],1);
                close(pfd1[1]);
                execlp("find","find",metadata_folder,"-lname",file_data,NULL);
                perror("Falhou a obter o código.");
                erro = 1;
                _exit(1);
            } else {
                wait(NULL);
                close(pfd1[1]);
                res_read = read(pfd1[0],link_path,SIZE);
                if(strlen(link_path) == 0) {
                    if(!fork()) {
                        execlp("rm","rm",file_data,NULL);
                        perror("Não consegui remover o ficheiro.");
                        erro = 1;
                    }else wait(NULL);
                }
            }

            _exit(0);
        }else wait(NULL);

    }
    return erro;
}



int restore(Ficheiro f) {

    int size_path, erro = 0, open_file, tam, res_write, open_pipe_cliente, res_pipe_cliente;
    char metadata_folder[SIZE], data_folder[SIZE], link_path[SIZE], file_metadata[SIZE], file_compressed[SIZE], file_to_send[SIZE];
    char buffer[FILE_SIZE];
    char* homedir = getenv("HOME");
    char pipe_restore_path[SIZE];

    sprintf(pipe_restore_path,"%s/.Backup/soburestore",homedir);
    res_pipe_cliente = mkfifo(pipe_restore_path,0744);


    sprintf(metadata_folder,"%s/.Backup/metadata",homedir);
    sprintf(file_metadata,"%s/%s",metadata_folder,f->ficheiro);

    if(access(file_metadata, F_OK ) == -1) {
        kill(f->pid_cliente,SIGUSR2);
        unlink(pipe_restore_path);
        return 1;
    }else {
        kill(f->pid_cliente,SIGUSR1);
    }

    size_path = readlink(file_metadata,link_path,SIZE);
    link_path[size_path] = '\0';

    open_pipe_cliente = open(pipe_restore_path, O_WRONLY);

    sprintf(data_folder,"%s/.Backup/data",homedir);
    sprintf(file_compressed,"%s/%s.gz",data_folder,f->ficheiro);

    if(!fork()) {
        execlp("cp","cp",link_path,file_compressed,NULL);
        perror("Falha na alteração do ficheiro.");
        erro = 1;
        _exit(0);
    }else wait(NULL);

    if(!fork()) {
        execlp("gunzip","gunzip",file_compressed,NULL);
        perror("Falha na descompressão do ficheiro.");
        erro = 1;
        _exit(0);
    }else wait(NULL);

    sprintf(file_to_send,"%s/%s",data_folder,f->ficheiro);
    open_file = open(file_to_send,O_RDONLY);

    while((tam = read(open_file,buffer,FILE_SIZE)) > 0) {
        f = altera_ficheiro_servidor(f,f->comando,f->ficheiro,getpid(),buffer,1,tam);
        res_write = write(open_pipe_cliente,f,sizeof(*f));
    }
    f = altera_ficheiro_servidor(f,f->comando,f->ficheiro,getpid(),buffer,0,0);
    res_write = write(open_pipe_cliente,f,sizeof(*f));
    close(open_file);

    if(!fork()) {
        execlp("rm","rm",file_to_send,NULL);
        perror("Falha na remoção do ficheiro.");
        erro = 1;
        _exit(0);
    }else wait(NULL);

    return erro;
}



int backup(Ficheiro f) {

    int resultado = 0, mudar_nome;
    char* homedir = getenv("HOME");

    char metadata_folder[SIZE], data_folder[SIZE];
    char file_coded[SIZE], data_path[SIZE], data_encripted[SIZE];
    char file_metadata[SIZE];

    sprintf(data_folder,"%s/.Backup/data",homedir);
    sprintf(metadata_folder,"%s/.Backup/metadata",homedir);
    sprintf(file_coded,"%s/%s",data_folder,f->codigo);
    sprintf(data_path,"%s/%s",data_folder,f->ficheiro);
    sprintf(data_encripted,"%s.gz",data_path);
    sprintf(file_metadata,"%s/%s",metadata_folder,f->ficheiro);


    if(access(file_coded, F_OK ) != -1) {

        if (access(file_metadata, F_OK ) == -1) {

            if(!fork()) {
                unlink(file_metadata);
                execlp("ln","ln","-s",file_coded,file_metadata,NULL);
                resultado = 1;
                perror("Linkagem do ficheiro para metadata mal sucedida");
                _exit(0);
            }else wait(NULL);

        } else {
            return 0;
        }

    } else {

        if(!fork()) {
            execlp("gzip","gzip",data_path,NULL);
            resultado = 1;
            perror("Falha ao comprimir o ficheiro");
            _exit(0);
        } else wait(NULL);

        mudar_nome = rename(data_encripted,file_coded);
        if(mudar_nome == -1) {
            resultado = 1;
            printf("Erro ao alterar o nome do ficheiro %s\n",f->ficheiro);
        }

        if(!fork()) {
            execlp("ln","ln","-s",file_coded,file_metadata,NULL);
            resultado = 1;
            perror("Linkagem do ficheiro para metadata mal sucedida");
            _exit(0);
        } else wait(NULL);

    }

    return resultado;
}



void morte(int pid) {
    waitpid(pid, NULL, WCONTINUED);
    filhos_vivos--;
}
