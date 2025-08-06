# QuantaLista

## About QuantaLista
QuantaLista is an intelligent coordination and management layer for multi-agent systems. In complex environments where multiple Large Language Model (LLM) agents must collaborate, QuantaLista acts as the central nervous system, ensuring that tasks are managed, prioritized, and scheduled efficiently. It prevents chaos, eliminates redundant work, and optimizes the collective workflow to achieve complex goals faster and more reliably.

It is designed for developers and researchers building sophisticated applications with autonomous agents, from collaborative software development teams to distributed scientific discovery systems.

### Key Features:
- **Dynamic Task Scheduling and Prioritization**: A robust scheduling engine manages a queue of tasks, considering dependencies to ensure correct execution order.
- **Multi-Agent Coordination Engine**: A central coordinator assigns tasks to available agents and manages their states (`IDLE`, `BUSY`).
- **Structured and Testable Codebase**: The project is organized into a clean `src` and `test` directory structure, with a suite of unit tests to ensure reliability.

## Architecture Overview

QuantaLista is built on a modular C++ architecture:

- **Coordinator**: The central engine that orchestrates the entire process.
- **Scheduler**: The core component that handles task prioritization and dependency resolution.
- **AgentManager**: Manages agent registration and state.
- **Data Models**: Clear C++ classes for `Project`, `Workflow`, `Task`, and `Agent`.

## Getting Started

The source code and related files are located in the `QuantaLista/` directory.

### Prerequisites
- A C++ compiler that supports C++11 (e.g., g++, clang++).

### Compilation
Navigate to the `QuantaLista/` directory to run the compilation commands.

**To compile the main application:**
```bash
g++ -std=c++11 -Isrc -o QuantaListaApp src/main.cpp src/QuantaLista.cpp
```

**To compile the unit tests:**
```bash
g++ -std=c++11 -Isrc -o run_tests test/tests.cpp src/QuantaLista.cpp
```

### Running the Application
**To run the main simulation:**
```bash
./QuantaListaApp
```

**To run the unit tests:**
```bash
./run_tests
```

## Example Usage

The main application (`QuantaListaApp`) provides a demonstration of the core coordination loop. When you run it, it will:
1.  Initialize a `Coordinator`.
2.  Define a sample project with two agents ("Researcher" and "Writer").
3.  Define a sample workflow with three tasks, where the second task depends on the first, and the third depends on the second.
4.  Run a simulation loop where the `Coordinator` assigns the tasks to the agents in the correct, dependency-aware order.

The output will show the step-by-step process of tasks being assigned, processed, and completed. This demonstrates the core functionality of the task scheduler and agent manager working in harmony.

## Contributing

We welcome contributions! Please see the documents in the `docs/` directory for more information on the development plan and future enhancements.
