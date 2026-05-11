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
    printf("[HUB] Se pregateste lansarea monitorului prin hub_mon...\n");
    
    // Aici va veni logica de: 
    // fork() -> hub_mon 
    //    pipe() 
    //    fork() -> exec(monitor)
    //    hub_mon citeste din pipe si afiseaza
}

// Functie pentru comanda calculate_scores (Pasul 5 din plan)
void handle_calculate_scores(int arg_count, char *districts[]) {
    if (arg_count == 0) {
        printf("[HUB] Eroare: Trebuie sa specificati cel putin un district.\n");
        return;
    }
    printf("[HUB] Calculare scoruri pentru %d districte...\n", arg_count);
    
    // Aici va veni logica de:
    // for fiecare district:
    //    pipe() -> fork() -> exec(scorer) -> dup2()
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

        // Eliminam newline-ul de la final
        input[strcspn(input, "\n")] = 0;

        // Parsam comanda si argumentele
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
            // Trimitem lista de districte (incepand cu args[1])
            handle_calculate_scores(i - 1, &args[1]);
        } 
        else {
            printf("[HUB] Comanda necunoscuta: %s\n", args[0]);
        }
    }

    return 0;
}