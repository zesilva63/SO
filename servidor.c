
    #include "servidor.h"

    int filhos_vivos;

    int main() {

        signal(SIGCHLD,morte);

        if(!fork()) {


        int open_pipe, res_server, res_pipe, res_comando;
        char buffer[SIZE];
        Ficheiro f = inicia_ficheiro();

        char* homedir = getenv("HOME");

        char backup_folder[SIZE], metadata_folder[SIZE], data_folder[SIZE];
        sprintf(backup_folder,"%s/.Backup",homedir);
        mkdir(backup_folder,0755);
        sprintf(metadata_folder,"%s/.Backup/metadata",homedir);
        mkdir(metadata_folder,0755);
        sprintf(data_folder,"%s/.Backup/data",homedir);
        mkdir(data_folder,0755);

        res_pipe = mkfifo(PIPE_PATH, 0744);
        open_pipe = open(PIPE_PATH, O_RDONLY);

        int open_file;
        char data_path[SIZE];

            while(1) {

                res_server = read(open_pipe,f,sizeof(*f));
                if(res_server) {
                    if(strcmp(f->comando,"backup") == 0) {
                        if(f->estado == 1) {
                            sprintf(data_path,"%s/%s",data_folder,f->ficheiro);
                            open_file = open(data_path, O_CREAT | O_APPEND | O_WRONLY, 0600);
                            write(open_file,f->conteudo,f->tamanho);
                            close(open_file);
                        }else {
                            if(filhos_vivos == MAX_FILHOS) pause();

                            filhos_vivos++;

                            if(!fork()) {
                                res_comando = backup(f);
                                if(!res_comando) kill(f->pid_cliente,SIGUSR1);
                                else kill(f->pid_cliente,SIGQUIT);
                                _exit(0);
                            } // fork
                        } // else
                    } // if strcmp == backup
                } // res_server
            } // while 1

        close(open_pipe);
        unlink(PIPE_PATH);
        _exit(0);
    } // fork
    return 0;
} // main


int restore(Ficheiro f) {



    return 1;
}



int backup(Ficheiro f) {
    int mudar_nome;
    int resultado = 0;
    int pfd[2];
    pipe(pfd);

    char* homedir = getenv("HOME");

    char backup_folder[SIZE], metadata_folder[SIZE], data_folder[SIZE];
    char file_coded[SIZE], data_path[SIZE], data_encripted[SIZE];
    char file_metadata[SIZE];

    sprintf(backup_folder,"%s/.Backup",homedir);
    sprintf(metadata_folder,"%s/.Backup/metadata",homedir);
    sprintf(data_folder,"%s/.Backup/data",homedir);
    sprintf(file_coded,"%s/%s",data_folder,f->codigo);
    sprintf(data_path,"%s/%s",data_folder,f->ficheiro);
    sprintf(data_encripted,"%s.gz",data_path);
    sprintf(file_metadata,"%s/%s",metadata_folder,f->ficheiro);



    if(!fork()) {
        execlp("gzip","gzip",data_path,NULL);
        resultado = 1;
        perror("Falha ao comprimir o ficheiro");
        _exit(0);
    }else {
        wait(NULL);
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




void morte(int pid) {
    waitpid(pid, NULL, WCONTINUED);
    filhos_vivos--;
}
