#include "QuantaLista.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <filesystem>

#include "cli.h"

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::string command = argv[1];
        if (command == "daemon") {
            Coordinator coordinator("./queue");
            coordinator.run();
        } else if (command == "add") {
            addTask(argc, argv);
        } else if (command == "list") {
            listTasks();
        }
        else {
            std::cerr << "Unknown command: " << command << std::endl;
            return 1;
        }
    } else {
        std::cout << "Usage: " << argv[0] << " <command>" << std::endl;
        std::cout << "Commands:" << std::endl;
        std::cout << "  daemon      - Run the QuantaLista daemon" << std::endl;
        std::cout << "  add         - Add a new task to the queue" << std::endl;
        std::cout << "  list        - List all tasks in the queue" << std::endl;
    }

    return 0;
}
