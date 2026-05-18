#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_CMD_LEN 1024
#define MAX_ARGS 64

// Functie pentru comanda start_monitor (Pasul 4 din plan)
void handle_start_monitor() {
    int pipe_fd[2]; 

    if (pipe(pipe_fd) == -1) {
        perror("[HUB ERR] Nu s-a putut crea pipe-ul");
        return;
    }

    pid_t hub_mon_pid = fork();

    if (hub_mon_pid < 0) {
        perror("[HUB ERR] Eroare la fork pentru hub_mon");
        return;
    }

    if (hub_mon_pid == 0) {        
        pid_t monitor_pid = fork();
        if (monitor_pid == 0) {
            
            close(pipe_fd[0]);
            
            dup2(pipe_fd[1], STDOUT_FILENO);
            close(pipe_fd[1]);

            execlp("./monitor", "./monitor", NULL);
            
            perror("[NEPOT ERR] Eroare la execlp monitor");
            exit(1);
        } else {
            
            close(pipe_fd[1]);
            
            char buffer[512];
            ssize_t bytes_read;

            printf("[hub_mon] Monitor lansat. Ascult mesaje...\n");

            while ((bytes_read = read(pipe_fd[0], buffer, sizeof(buffer) - 1)) > 0) {
                buffer[bytes_read] = '\0';
                printf("[hub_mon -> MONITOR]: %s", buffer);
            }

            printf("\n[hub_mon] Monitorul s-a oprit pentru orice motiv. Se inchide si hub_mon.\n");
            close(pipe_fd[0]);
            exit(0);
        }
    }

    close(pipe_fd[0]);
    close(pipe_fd[1]);
    
    printf("[HUB] hub_mon a fost lansat in background (PID: %d).\n", hub_mon_pid);
}

// Functie pentru comanda calculate_scores (Pasul 5 din plan)
void handle_calculate_scores(int arg_count, char *districts[]) {
    if (arg_count == 0) {
        printf("[HUB] Eroare: Trebuie sa specificati cel putin un district.\n");
        return;
    }
    printf("\n[HUB] Calculare scoruri pentru %d districte...\n\n", arg_count);
    
    for(int i = 0; i < arg_count; i++){
        int pipe_fd[2];

        if(pipe(pipe_fd) == -1){
            perror("[HUB ERR] Nu s-a putut crea pipe-ul pentru scorer");
            continue;
        }

        pid_t scorer_pid = fork();

        if(scorer_pid < 0){
            perror("[HUB ERR] Eroare la fork pentru scorer");
            continue;
        }

        if(scorer_pid == 0){
            close(pipe_fd[0]);

            dup2(pipe_fd[1], STDOUT_FILENO);

            close(pipe_fd[1]);

            execlp("./scorer", "./scorer", districts[i], NULL);

            perror("[SCORER ERR] Nu s-a putut llansa ./scorer");
            exit(1);
        }
        else{
            close(pipe_fd[1]);

            char buffer[1024];
            ssize_t bytes_read;

            while((bytes_read = read(pipe_fd[0], buffer, sizeof(buffer) - 1)) > 0){
                buffer[bytes_read] = '\0';
                printf("%s\n", buffer);
            }

            close(pipe_fd[0]);

            waitpid(scorer_pid, NULL, 0);
        }
    }

    printf("\n[HUB] Raport combinat finalizat.\n\n");
}

int main() {
    char input[MAX_CMD_LEN];
    char *args[MAX_ARGS];

    printf("========================================\n");
    printf("   CITY INFRASTRUCTURE HUB - Phase 3    \n");
    printf("========================================\n");
    printf("Comenzi disponibile: start_monitor, calculate_scores <d1> <d2>..., exit\n\n");

    while (1) {
        printf("city_hub> ");
        if (fgets(input, sizeof(input), stdin) == NULL) break;

        input[strcspn(input, "\n")] = 0;

        int i = 0;
        args[i] = strtok(input, " ");
        while (args[i] != NULL && i < MAX_ARGS - 1) {
            args[++i] = strtok(NULL, " ");
        }

        if (args[0] == NULL) continue;

        if (strcmp(args[0], "exit") == 0) {
            printf("[HUB] Se inchide consola. La revedere!\n");
            break;
        } 
        else if (strcmp(args[0], "start_monitor") == 0) {
            handle_start_monitor();
        } 
        else if (strcmp(args[0], "calculate_scores") == 0) {
            handle_calculate_scores(i - 1, &args[1]);
        } 
        else {
            printf("[HUB] Comanda necunoscuta: %s\n", args[0]);
        }
    }



    return 0;
}