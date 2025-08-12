# QuantaLista

## About QuantaLista
QuantaLista is an intelligent coordination and management layer for multi-agent systems. In complex environments where multiple Large Language Model (LLM) agents must collaborate, QuantaLista acts as the central nervous system, ensuring that tasks are managed, prioritized, and scheduled efficiently. It prevents chaos, eliminates redundant work, and optimizes the collective workflow to achieve complex goals faster and more reliably.

It is designed for developers and researchers building sophisticated applications with autonomous agents, from collaborative software development teams to distributed scientific discovery systems.

### Key Features:
- **Dynamic Task Scheduling and Prioritization**: A robust scheduling engine manages a queue of tasks, considering dependencies to ensure correct execution order.
- **Multi-Agent Coordination Engine**: A central coordinator assigns tasks to available agents and manages their states (`IDLE`, `BUSY`).
- **Structured and Testable Codebase**: The project is organized into a clean `src` and `test` directory structure, with a suite of unit tests to ensure reliability.

## Architecture Overview

QuantaLista is built on a distributed, multi-repository architecture. Each component is a distinct service, designed for independent development, deployment, and scaling.

- **`QuantaLista-Core`**: The central engine that orchestrates the entire process. It contains the `Coordinator`, `Scheduler`, and `AgentManager`. This repository contains the core C++ application.
- **`QuantaLista-Gateway`**: A lightweight API gateway that provides a unified entry point for all client interactions, routing requests to the appropriate downstream service.
- **`QuantaLista-Advisor`**: A Python-based service that provides intelligent oversight, monitoring, and recommendations. It consumes data from the core engine to provide insights.
- **`QuantaLista-UI`**: A web-based user interface for workflow visualization, manual task management, and viewing agent status.

## Getting Started

This repository contains the documentation and overall architectural guidelines for the QuantaLista ecosystem. The source code for each service is located in its own repository.

To get started, please refer to the `README.md` file in each of the following repositories:
- `QuantaLista-Core`
- `QuantaLista-Gateway`
- `QuantaLista-Advisor`
- `QuantaLista-UI`

## Example Usage

The main application (`QuantaListaApp`) provides a demonstration of the core coordination loop. When you run it, it will:
1.  Initialize a `Coordinator`.
2.  Define a sample project with two agents ("Researcher" and "Writer").
3.  Define a sample workflow with three tasks, where the second task depends on the first, and the third depends on the second.
4.  Run a simulation loop where the `Coordinator` assigns the tasks to the agents in the correct, dependency-aware order.

The output will show the step-by-step process of tasks being assigned, processed, and completed. This demonstrates the core functionality of the task scheduler and agent manager working in harmony.

## Contributing

We welcome contributions! Please see the documents in the `docs/` directory for more information on the development plan and future enhancements.
