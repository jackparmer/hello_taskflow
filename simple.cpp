#include "taskflow/taskflow.hpp"
#include <iostream>
#include <fstream>
#include "toml11/include/toml.hpp"
#include <cstdlib>

#include <unistd.h>
#include <sys/wait.h>
#include <vector>
#include <array>

// Function to execute a command and capture its output
std::string exec(const char* cmd) {
    int pipefd[2];
    pid_t pid;
    std::array<char, 256> buffer;
    std::string result;

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {  // Child process
        close(pipefd[0]);  // Close the unused read end
        dup2(pipefd[1], STDOUT_FILENO);  // Redirect stdout to the write end of the pipe
        dup2(pipefd[1], STDERR_FILENO);  // Redirect stderr to the write end of the pipe
        execlp("python3", "python3", cmd, nullptr);
        exit(EXIT_FAILURE);  // execlp only returns on error
    } else {  // Parent process
        close(pipefd[1]);  // Close the unused write end
        while (read(pipefd[0], buffer.data(), buffer.size()) > 0) {
            result += buffer.data();
        }
        waitpid(pid, NULL, 0);  // Wait for the child to exit
    }
    return result;
}

int main() {
    try {
        auto config = toml::parse("config.toml");
        auto scripts = toml::find<std::vector<std::string>>(config, "scripts");

        tf::Executor executor;
        tf::Taskflow taskflow;
        std::vector<tf::Task> tasks;

        for (const auto& script : scripts) {
            tasks.emplace_back(
                taskflow.emplace([script]() {
                    std::string output = exec(script.c_str());
                    std::cout << "Output from " << script << ": " << output << std::endl;
                    return output;
                })
            );
        }

        for (size_t i = 0; i < tasks.size() - 1; ++i) {
            tasks[i].precede(tasks[i + 1]);
        }

        executor.run(taskflow).wait();
    } catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
