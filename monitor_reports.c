#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>


volatile sig_atomic_t keep_running = 1;

void handle_signals(int signum) {
    if (signum == SIGINT) {
        const char *msg = "\n[MONITOR] Semnal SIGINT primit. Se inchide monitorizarea...\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        keep_running = 0;
    } 
    else if (signum == SIGUSR1) {
        const char *msg = "\n[MONITOR] Notificare primita: Un nou raport a fost adaugat!\n";
        write(STDOUT_FILENO, msg, strlen(msg));
    }
}

int main() {
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

    printf("[MONITOR] Pornit cu PID %d. Fisier .monitor_pid generat. Astept semnale...\n", my_pid);


    while (keep_running) {
        pause();
    }
    if (unlink(".monitor_pid") == 0) {
        printf("[MONITOR] Fisierul .monitor_pid a fost sters curat. La revedere!\n");
    } else {
        perror("[MONITOR WARNING] Eroare la stergerea .monitor_pid");
    }

    return 0;
}