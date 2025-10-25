// recycled.c
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <stdarg.h>

#define CONFIG_PATH "/etc/recycled.conf"
#define LOG_PATH "/var/log/recycled.log"
#define PID_PATH "/var/run/recycled.pid"

typedef enum {
    UNIT_HOUR,
    UNIT_DAY,
    UNIT_MONTH
} TimeUnit;

typedef struct {
    char path[512];
    double threshold_hours;
    char excludes[10][32];
    int exclude_count;
} RecycleRule;

#define MAX_RULES 64
static RecycleRule rules[MAX_RULES];
static int rule_count = 0;
static volatile sig_atomic_t reload_config = 0;
static volatile sig_atomic_t running = 1;

void log_msg(const char *fmt, ...) {
    FILE *log = fopen(LOG_PATH, "a");
    if (!log) return;
    time_t now = time(NULL);
    char tbuf[64];
    strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    fprintf(log, "[%s] ", tbuf);
    va_list args;
    va_start(args, fmt);
    vfprintf(log, fmt, args);
    va_end(args);
    fprintf(log, "\n");
    fclose(log);
}

void write_pid() {
    FILE *f = fopen(PID_PATH, "w");
    if (f) {
        fprintf(f, "%d\n", getpid());
        fclose(f);
    } else {
        log_msg("Warning: could not write PID file %s: %s", PID_PATH, strerror(errno));
    }
}

void remove_pid() {
    unlink(PID_PATH);
}

int has_excluded_ext(const char *filename, RecycleRule *rule) {
    const char *dot = strrchr(filename, '.');
    if (!dot) return 0;
    for (int i = 0; i < rule->exclude_count; i++) {
        if (strcasecmp(dot + 1, rule->excludes[i]) == 0)
            return 1;
    }
    return 0;
}

void cleanup_rule(RecycleRule *rule) {
    DIR *dir = opendir(rule->path);
    if (!dir) {
        log_msg("Could not open directory %s: %s", rule->path, strerror(errno));
        return;
    }

    time_t now = time(NULL);
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", rule->path, entry->d_name);

        struct stat st;
        if (stat(fullpath, &st) == 0 && S_ISREG(st.st_mode)) {
            double age_hours = difftime(now, st.st_mtime) / 3600.0;
            if (age_hours > rule->threshold_hours && !has_excluded_ext(entry->d_name, rule)) {
                if (unlink(fullpath) == 0)
                    log_msg("Deleted: %s (%.1f h old)", fullpath, age_hours);
                else
                    log_msg("Failed to delete %s: %s", fullpath, strerror(errno));
            }
        }
    }
    closedir(dir);
}

void read_config() {
    FILE *f = fopen(CONFIG_PATH, "r");
    if (!f) {
        log_msg("Failed to read config file %s", CONFIG_PATH);
        return;
    }

    rule_count = 0;
    char line[1024];
    while (fgets(line, sizeof(line), f) && rule_count < MAX_RULES) {
        if (line[0] == '#' || strlen(line) < 2) continue;
        RecycleRule *r = &rules[rule_count];
        memset(r, 0, sizeof(*r));

        // Example line: /home/user/Downloads 30d exclude iso zip
        char *tok = strtok(line, " \t\n");
        if (!tok) continue;
        strncpy(r->path, tok, sizeof(r->path));

        tok = strtok(NULL, " \t\n");
        if (!tok) continue;
        double val = atof(tok);
        char unit = tok[strlen(tok) - 1];
        switch (unit) {
            case 'h': r->threshold_hours = val; break;
            case 'd': r->threshold_hours = val * 24.0; break;
            case 'm': r->threshold_hours = val * 24.0 * 30.0; break;
            default:  r->threshold_hours = val * 24.0; break; // default days
        }

        tok = strtok(NULL, " \t\n");
        if (tok && strcasecmp(tok, "exclude") == 0) {
            while ((tok = strtok(NULL, " \t\n")) != NULL && r->exclude_count < 10) {
                strncpy(r->excludes[r->exclude_count++], tok, 31);
            }
        }
        rule_count++;
    }

    fclose(f);
    log_msg("Config loaded: %d rules", rule_count);
}

void sighup_handler(int signo) {
    (void)signo;
    reload_config = 1;
}

void sigterm_handler(int signo) {
    (void)signo;
    running = 0;
}

void daemonize() {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    umask(0);
    setsid();
    chdir("/");

    fclose(stdin);
    fclose(stdout);
    fclose(stderr);
}

int main(int argc, char *argv[]) {
    int once = 0;
    const char *config_override = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--once") == 0) once = 1;
        if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) config_override = argv[++i];
    }

    if (config_override) setenv("RECYCLED_CONFIG", config_override, 1);

    daemonize();
    signal(SIGHUP, sighup_handler);
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);

    write_pid();
    read_config();
    log_msg("Recycled daemon started.");

    do {
        if (reload_config) {
            read_config();
            reload_config = 0;
        }

        for (int i = 0; i < rule_count; i++)
            cleanup_rule(&rules[i]);

        if (once) break;
        sleep(3600);
    } while (running);

    remove_pid();
    log_msg("Recycled daemon stopped.");
    return 0;
}
