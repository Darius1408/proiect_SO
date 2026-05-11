#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

// 1. Aceeasi structura ca in city_manager, ca sa citim corect binarul
typedef struct Report {
    int id;
    char inspector[32];
    float latitude;
    float longitude;
    char category[32];
    int severity;
    time_t timestamp;
    char description[256];
} Report;

// 2. O structura mica doar pentru a tine minte scorul fiecarui inspector
typedef struct {
    char inspector_name[32];
    int total_score;
} InspectorScore;

int main(int argc, char *argv[]) {
    // Verificam daca a primit districtul ca argument
    if (argc != 2) {
        printf("[SCORER ERR] Utilizare: %s <district>\n", argv[0]);
        return 1;
    }

    const char *district = argv[1];
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/reports.dat", district);

    // Incercam sa deschidem fisierul
    int fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        // Daca nu exista fisierul, printam un mesaj si iesim fara eroare fatala
        printf("[District: %s] Nu exista rapoarte sau districtul este invalid.\n", district);
        return 0; 
    }

    // Un array in care vom memora scorurile. 
    // Presupunem ca nu avem mai mult de 100 de inspectori unici intr-un district.
    InspectorScore scores[100];
    int num_unique_inspectors = 0;
    Report r;

    // Citim raport cu raport
    while (read(fd, &r, sizeof(Report)) > 0) {
        int found = 0;
        
        // Cautam daca am mai intalnit acest inspector
        for (int i = 0; i < num_unique_inspectors; i++) {
            if (strcmp(scores[i].inspector_name, r.inspector) == 0) {
                scores[i].total_score += r.severity; // Adaugam la scor
                found = 1;
                break;
            }
        }
        
        // Daca e un inspector nou pe care nu l-am mai vazut
        if (!found && num_unique_inspectors < 100) {
            strcpy(scores[num_unique_inspectors].inspector_name, r.inspector);
            scores[num_unique_inspectors].total_score = r.severity;
            num_unique_inspectors++;
        }
    }
    close(fd);

    // 3. Printam Raportul (Asta va fi capturat prin pipe de city_hub!)
    printf("--- Workload Score pentru [%s] ---\n", district);
    if (num_unique_inspectors == 0) {
        printf("Niciun inspector gasit in acest district.\n");
    } else {
        for (int i = 0; i < num_unique_inspectors; i++) {
            printf(" > Inspector: %-10s | Scor Total Severitate: %d\n", 
                   scores[i].inspector_name, scores[i].total_score);
        }
    }
    printf("\n"); // Un rand liber la final ca sa arate frumos in consola principala

    return 0;
}