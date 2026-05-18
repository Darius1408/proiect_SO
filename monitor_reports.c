#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

volatile sig_atomic_t keep_running = 1;

// 1. Modificam mesajele din semnale ca sa aiba "Tag-uri" speciale
void handle_signals(int signum) {
    if (signum == SIGINT) {
        const char *msg = "MSG:EXIT|Semnal SIGINT primit. Se inchide monitorizarea...\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        keep_running = 0;
    } 
    else if (signum == SIGUSR1) {
        const char *msg = "MSG:NOTIFY|Un nou raport a fost adaugat!\n";
        write(STDOUT_FILENO, msg, strlen(msg));
    }
}

int main() {
    int fd_check = open(".monitor_pid", O_RDONLY);
    if (fd_check >= 0) {
        char pid_buf[32] = {0};
        read(fd_check, pid_buf, sizeof(pid_buf) - 1);
        close(fd_check);

        pid_buf[strcspn(pid_buf, "\n")] = 0;

        char err_msg[256];
        snprintf(err_msg, sizeof(err_msg), "MSG:ERR|Monitor deja pornit cu PID %s. Refuz pornirea.\n", pid_buf);
        write(STDOUT_FILENO, err_msg, strlen(err_msg));
        
        return 1;
    }

    struct sigaction sa;
    sa.sa_handler = handle_signals;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Eroare la inregistrarea SIGINT");
        return 1;
    }
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("Eroare la inregistrarea SIGUSR1");
        return 1;
    }

    pid_t my_pid = getpid();

    int fd = open(".monitor_pid", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("Eroare la crearea .monitor_pid");
        return 1;
    }
    
    char pid_str[32];
    snprintf(pid_str, sizeof(pid_str), "%d\n", my_pid);
    write(fd, pid_str, strlen(pid_str));
    close(fd);

    // 2. Modificam si mesajul de start sa aiba tag-ul MSG:INFO
    char startup_msg[256];
    snprintf(startup_msg, sizeof(startup_msg), "MSG:INFO|Pornit cu PID %d. Fisier .monitor_pid generat. Astept semnale...\n", my_pid);
    write(STDOUT_FILENO, startup_msg, strlen(startup_msg));

    while (keep_running) {
        pause();
    }

    if (unlink(".monitor_pid") == 0) {
        const char *msg_end = "MSG:INFO|Fisierul .monitor_pid a fost sters curat. La revedere!\n";
        write(STDOUT_FILENO, msg_end, strlen(msg_end));
    }
    else{
        perror("[MONITOR WARNING] Eroare la stergerea .monitor_pid");
    }

    return 0;
}