#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Structura de baza pentru un raport, conform cerintelor */
typedef struct {
    int id;
    char inspector[32];     // fixed-length string pentru numele luat din --user
    float latitude;         // GPS coordonate
    float longitude;        // GPS coordonate
    char category[32];      // ex: "road", "lighting", "flooding"
    int severity;           // 1 = minor, 2 = moderate, 3 = critical
    time_t timestamp;
    char description[256];  // fixed-length string
} Report;

int main(int argc, char *argv[]) {
    // Verificare sumara a argumentelor (doar scheletul initial)
    if (argc < 5) {
        fprintf(stderr, "Usage: %s --role <manager|inspector> --user <name> <command> [args...]\n", argv[0]);
        return 1;
    }

    // TODO: Parcurgerea argumentelor pentru a extrage rolul si userul
    // TODO: Identificarea si executarea comenzii (--add, --list, --view, etc.)

    printf("Sistemul City Manager a fost initializat.\n");
    
    return 0;
}