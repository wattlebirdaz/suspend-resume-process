#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sched.h>
#include <ctime>

// wait for a file to be modified using inotify
void wait_for_file_modification(const std::string &filename) {
    int ifd, wd;
    char buffer[sizeof(struct inotify_event)];

    // Initialize inotify instance
    ifd = inotify_init();
    if (ifd < 0) {
        perror("inotify_init");
        exit(EXIT_FAILURE);
    }

    // Add watch for file modification
    wd = inotify_add_watch(ifd, filename.c_str(), IN_MODIFY);
    if (wd < 0) {
        perror("inotify_add_watch");
        exit(EXIT_FAILURE);
    }

    std::cout << "Waiting for file modification...\n";

    // Read to block until an event occurs
    int length = read(ifd, buffer, sizeof(buffer));
    if (length < 0) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    std::cout << "File: " << filename << " was modified!\n";

    // Cleaning up
    inotify_rm_watch(ifd, wd);
    close(ifd);
}

void run_child_process() {
    // Run your program here
    system("./your_program");
}

void suspend_child_process(pid_t pid) {
    std::cout << "Suspending child process...\n";

    std::string buffer = "criu-3.17.1/criu/criu dump -t " + std::to_string(pid) + " -D ./images --shell-job --file-locks --log-file dump_log.txt";
    system(buffer.c_str());

    system("sync");
    system("echo 1 > /proc/sys/vm/drop_caches");
}

void resume_child_process(pid_t pid) {
    std::cout << "Resuming child process...\n";

    std::string buffer = "criu-3.17.1/criu/criu restore -D ./images --shell-job --file-locks --log-file restore_log.txt";
    system(buffer.c_str());

    kill(pid, SIGCONT);
}

int main(int argc, char *argv[]) {
    pid_t pid;
    pid = fork();
    int parent_process_sleep_time = 3;

    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Your program you want to run
        std::cout << "Child process running...\n";
        run_child_process();
    } else {
        // Wait for the child process to finish initializing
        wait_for_file_modification(".setup");

        // create a image directory, delete if it already exists
        system("rm -rf images");
        system("mkdir images");

        // Sleep for 3 seconds
        std::cout << "Parent process waiting for " << parent_process_sleep_time << " seconds before suspending child process...\n";
        sleep(parent_process_sleep_time);

        // Suspend the child process
        suspend_child_process(pid);
        wait(NULL);

        // Resume the child process
        resume_child_process(pid);
        wait(NULL);
    }

    return 0;
}
