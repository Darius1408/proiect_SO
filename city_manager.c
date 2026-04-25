#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

// Structura actualizata conform cerintelor
typedef struct Report{
    int id;
    char inspector[32];
    float latitude;       // Adaugat
    float longitude;      // Adaugat
    char category[32];
    int severity;
    time_t timestamp;
    char description[256];
} Report;

// Functie helper pentru a transforma st_mode in string "rw-rw-r--"
void permissions_to_string(mode_t mode, char *str) {
    strcpy(str, "---------");
    if (mode & S_IRUSR) str[0] = 'r';
    if (mode & S_IWUSR) str[1] = 'w';
    if (mode & S_IXUSR) str[2] = 'x';
    if (mode & S_IRGRP) str[3] = 'r';
    if (mode & S_IWGRP) str[4] = 'w';
    if (mode & S_IXGRP) str[5] = 'x';
    if (mode & S_IROTH) str[6] = 'r';
    if (mode & S_IWOTH) str[7] = 'w';
    if (mode & S_IXOTH) str[8] = 'x';
}

// Functie pentru crearea districtului daca nu exista
void setup_district(const char* district) {
    struct stat st = {0};
    if (stat(district, &st) == -1) {
        mkdir(district, 0750); // Permisiuni 750 pentru director
    }
}

// --- FUNCTII GENERATE DE AI PENTRU FILTER ---
int parse_condition(const char *input, char *field, char *op, char *value) {
    // Extrage field, operator si value din sirul "field:operator:value"
    if (sscanf(input, "%[^:]:%[^:]:%s", field, op, value) == 3) {
        return 1;
    }
    return 0;
}

int match_condition(Report *r, const char *field, const char *op, const char *value) {
    // Compara severity (int)
    if (strcmp(field, "severity") == 0) {
        int val = atoi(value);
        if (strcmp(op, "==") == 0) return r->severity == val;
        if (strcmp(op, "!=") == 0) return r->severity != val;
        if (strcmp(op, ">=") == 0) return r->severity >= val;
        if (strcmp(op, "<=") == 0) return r->severity <= val;
        if (strcmp(op, ">") == 0) return r->severity > val;
        if (strcmp(op, "<") == 0) return r->severity < val;
    }
    // Compara category (string)
    else if (strcmp(field, "category") == 0) {
        if (strcmp(op, "==") == 0) return strcmp(r->category, value) == 0;
        if (strcmp(op, "!=") == 0) return strcmp(r->category, value) != 0;
    }
    // Compara inspector (string)
    else if (strcmp(field, "inspector") == 0) {
        if (strcmp(op, "==") == 0) return strcmp(r->inspector, value) == 0;
        if (strcmp(op, "!=") == 0) return strcmp(r->inspector, value) != 0;
    }
    return 0;
}

// --- FUNCTIA DE LOGARE ---
void log_action(const char* district, const char* role, const char* user, const char* action) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/logged_district", district);

    // Verificam permisiunile daca fisierul exista deja
    struct stat st;
    if (stat(filepath, &st) == 0) {
        if (strcmp(role, "inspector") == 0) {
            // Inspectorii sunt grupul, verificam bitul de scriere grup S_IWGRP
            if (!(st.st_mode & S_IWGRP)) {
                printf("[ERROR] Rolul 'inspector' nu are permisiunea de a scrie in %s!\n", filepath);
                return; // Refuza scrierea
            }
        }
    }

    int fd = open(filepath, O_WRONLY | O_CREAT | O_APPEND, 0644); // 644 = rw-r--r--
    if (fd >= 0) {
        chmod(filepath, 0644); // Fortam permisiunile
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "%ld\t%s\t%s\t%s\n", time(NULL), user, role, action);
        write(fd, buffer, strlen(buffer));
        close(fd);
    }
}

// --- FUNCTIA DE ADAUGARE ---
void add_report(const char* role, const char* user, const char* district) {
    setup_district(district);
    
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/reports.dat", district);

    // Deschidem fisierul. Daca nu exista, il cream cu 0664
    int fd = open(filepath, O_WRONLY | O_CREAT | O_APPEND, 0664);
    if (fd < 0) { perror("Eroare deschidere fisier"); return; }
    
    // Fortam permisiunile exact cum cere enuntul dupa creare
    chmod(filepath, 0664);

    Report r;
    memset(&r, 0, sizeof(Report));
    r.id = (int)time(NULL); 
    r.timestamp = time(NULL);
    strncpy(r.inspector, user, 31);
    
    // Pentru faza 1, citim datele de la tastatura simulat ca in screenshot
    printf("X (lat): "); scanf("%f", &r.latitude);
    printf("Y (lon): "); scanf("%f", &r.longitude);
    printf("Category (road/lighting/flooding/other): "); scanf("%31s", r.category);
    printf("Severity level (1/2/3): "); scanf("%d", &r.severity);
    getchar(); // consumam newline
    printf("Description: ");
    fgets(r.description, 255, stdin);
    r.description[strcspn(r.description, "\n")] = 0; // stergem newline-ul

    write(fd, &r, sizeof(Report));
    close(fd);
    
    // Cream simlink-ul cerut activ
    char symlink_name[256];
    snprintf(symlink_name, sizeof(symlink_name), "active_reports-%s", district);
    symlink(filepath, symlink_name); // Ignoram eroarea daca exista deja

    log_action(district, role, user, "add");

    printf("[SUCCESS] Raport adaugat in %s!\n", filepath);
}

// --- FUNCTIA DE LISTARE ---
void list_reports(const char* role, const char* user, const char* district) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/reports.dat", district);

    struct stat file_stat;
    if (stat(filepath, &file_stat) == -1) {
        printf("[INFO] Nu exista rapoarte pentru %s.\n", district);
        return;
    }

    // Afisam permisiunile manual, conform cerintei
    char perms[10];
    permissions_to_string(file_stat.st_mode, perms);
    printf("Raport file: %s, Size: %lld bytes, Permissions: %s\n", filepath, file_stat.st_size, perms);

    int fd = open(filepath, O_RDONLY);
    if (fd < 0) return;

    Report r;
    printf("%-12s | %-10s | %-10s | %-5s | %s\n", "ID", "Inspector", "Categorie", "Sev", "Descriere");
    printf("----------------------------------------------------------------------\n");

    while (read(fd, &r, sizeof(Report)) > 0) {
        printf("%-12d | %-10s | %-10s | %-5d | %s\n", 
               r.id, r.inspector, r.category, r.severity, r.description);
    }
    close(fd);
}

// --- FUNCTIA DE STERGERE (Rescrisa corect cu lseek si ftruncate) ---
void delete_report(const char* role, const char* district, int target_id) {
    if (strcmp(role, "manager") != 0) {
        printf("[ERROR] Doar rolul de manager poate sterge rapoarte.\n");
        return;
    }

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/reports.dat", district);

    int fd = open(filepath, O_RDWR);
    if (fd < 0) {
        perror("Eroare deschidere fisier");
        return;
    }

    Report r;
    off_t pos = 0;
    int found = 0;
    off_t target_pos = -1;

    // 1. Cautam inregistrarea
    while (read(fd, &r, sizeof(Report)) > 0) {
        if (r.id == target_id) {
            found = 1;
            target_pos = pos;
            break;
        }
        pos += sizeof(Report);
    }

    if (!found) {
        printf("[ERROR] Raportul %d nu a fost gasit.\n", target_id);
        close(fd);
        return;
    }

    // 2. Shiftam restul fisierului peste inregistrarea stearsa
    off_t read_pos = target_pos + sizeof(Report);
    off_t write_pos = target_pos;

    while (1) {
        lseek(fd, read_pos, SEEK_SET);
        int bytes_read = read(fd, &r, sizeof(Report));
        if (bytes_read <= 0) break; // Am ajuns la final

        lseek(fd, write_pos, SEEK_SET);
        write(fd, &r, sizeof(Report));

        read_pos += sizeof(Report);
        write_pos += sizeof(Report);
    }

    // 3. Trunchiem fisierul pentru a taia spatiul mort de la final
    ftruncate(fd, write_pos);
    close(fd);

    log_action(district, role, "unknown_user", "remove_report");

    printf("[SUCCESS] Raportul %d a fost sters folosind shiftare si truncate.\n", target_id);
}


// --- FUNCTIA DE UPDATE THRESHOLD ---
void update_threshold(const char* role, const char* district, int value) {
    if (strcmp(role, "manager") != 0) {
        printf("[ERROR] Doar rolul de manager poate modifica district.cfg.\n");
        return;
    }

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/district.cfg", district);

    struct stat st;
    // Daca fisierul exista, verificam bitii 640 conform cerintei
    if (stat(filepath, &st) == 0) {
        if ((st.st_mode & 0777) != 0640) {
            printf("[ERROR] Permisiunile fisierului district.cfg nu sunt 640. Operatiune refuzata.\n");
            return;
        }
    }

    int fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, 0640);
    if (fd >= 0) {
        chmod(filepath, 0640); // Asiguram 640 (rw-r-----)
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "severity_threshold=%d\n", value);
        write(fd, buffer, strlen(buffer));
        close(fd);
        printf("[SUCCESS] Threshold actualizat la %d in %s\n", value, filepath);
        
        log_action(district, role, "unknown", "update_threshold");
    }
}

// --- FUNCTIA DE FILTRARE ---
void filter_reports(const char* district, int condition_count, char* conditions[]) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/reports.dat", district);

    int fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        printf("[INFO] Nu exista rapoarte de filtrat in %s.\n", district);
        return;
    }

    Report r;
    int found = 0;
    printf("\n--- REZULTATE FILTRARE ---\n");
    printf("%-12s | %-10s | %-10s | %-5s | %s\n", "ID", "Inspector", "Categorie", "Sev", "Descriere");
    printf("----------------------------------------------------------------------\n");

    while (read(fd, &r, sizeof(Report)) > 0) {
        int matches_all = 1;
        // Verificam toate conditiile (AND logic)
        for (int i = 0; i < condition_count; i++) {
            char field[32], op[8], value[64];
            if (parse_condition(conditions[i], field, op, value)) {
                if (!match_condition(&r, field, op, value)) {
                    matches_all = 0;
                    break;
                }
            }
        }
        
        if (matches_all) {
            printf("%-12d | %-10s | %-10s | %-5d | %s\n", 
                   r.id, r.inspector, r.category, r.severity, r.description);
            found++;
        }
    }
    close(fd);
    
    if (found == 0) printf("Nu s-au gasit rapoarte care sa respecte conditiile.\n");
}


// --- PROGRAMUL PRINCIPAL ---
int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s --role <manager|inspector> --user <nume> <comanda> [args]\n", argv[0]);
        return 1;
    }

    char *role = NULL, *user = NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--role") == 0) role = argv[++i];
        else if (strcmp(argv[i], "--user") == 0) user = argv[++i];
    }

    if (role && user) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "--add") == 0 && i + 1 < argc) {
                add_report(role, user, argv[i+1]);
                break;
            } 
            else if (strcmp(argv[i], "--list") == 0 && i + 1 < argc) {
                list_reports(role, user, argv[i+1]);
                break;
            } 
            else if (strcmp(argv[i], "--remove_report") == 0 && i + 2 < argc) {
                delete_report(role, argv[i+1], atoi(argv[i+2]));
                break;
            }
            else if (strcmp(argv[i], "--update_threshold") == 0 && i + 2 < argc) {
                update_threshold(role, argv[i+1], atoi(argv[i+2]));
                break;
            }
            else if (strcmp(argv[i], "--filter") == 0 && i + 2 < argc) {
                // argv[i+1] este districtul, de la i+2 incolo sunt conditiile
                int condition_count = argc - (i + 2);
                filter_reports(argv[i+1], condition_count, &argv[i+2]);
                break;
            }
        }
    }
    return 0;
}