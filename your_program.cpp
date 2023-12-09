#include <chrono>
#include <thread>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>

void initialize() {
    std::cout << "Initializing..." << std::endl;
    // sleep for 2 seconds
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Initialization complete!" << std::endl;

    // Add the current time to ".setup"
    // The parent process is waiting for this file to be modified
    // to know that the child process has finished initializing
    // and is ready to be suspended

    auto now = std::chrono::system_clock::now();
    int fd = open(".setup", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    write(fd, &now, sizeof(now));
    fsync(fd);
    close(fd);
}

void execute() {
    std::cout << "Executing..." << std::endl;
    // for 10 seconds, keep sleeping and printing the current time
    for (int i = 0; i < 10; i++) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        auto now = std::chrono::system_clock::now();
        std::cout << "Current time: " << std::chrono::system_clock::to_time_t(now) << std::endl;
    }
    std::cout << "Execution complete!" << std::endl;
}

int main() {
    // Setup code
    initialize();
    execute();
}