// recyclectl.c
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#define PID_PATH "/var/run/recycled.pid"

int get_pid() {
    FILE *f = fopen(PID_PATH, "r");
    if (!f) return -1;
    pid_t pid;
    fscanf(f, "%d", &pid);
    fclose(f);
    return pid;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: recyclectl <reload|stop|status>\n");
        return 0;
    }

    pid_t pid = get_pid();
    if (pid == -1) {
        printf("Recycled not running (no pid file)\n");
        return 1;
    }

    if (strcmp(argv[1], "reload") == 0) {
        kill(pid, SIGHUP);
        printf("Reload signal sent (pid %d)\n", pid);
    } else if (strcmp(argv[1], "stop") == 0) {
        kill(pid, SIGTERM);
        printf("Stop signal sent (pid %d)\n", pid);
    } else if (strcmp(argv[1], "status") == 0) {
        if (kill(pid, 0) == 0)
            printf("Recycled is running (pid %d)\n", pid);
        else
            printf("Recycled not responding.\n");
    } else {
        printf("Unknown command.\n");
    }
    return 0;
}
