# QuantaLista

## About QuantaLista
QuantaLista is an intelligent coordination and management layer for multi-agent systems. In complex environments where multiple Large Language Model (LLM) agents must collaborate, QuantaLista acts as the central nervous system, ensuring that tasks are managed, prioritized, and scheduled efficiently. It prevents chaos, eliminates redundant work, and optimizes the collective workflow to achieve complex goals faster and more reliably.

It is designed for developers and researchers building sophisticated applications with autonomous agents, from collaborative software development teams to distributed scientific discovery systems.

### Key Features:
- **Dynamic Task Scheduling and Prioritization**: A robust scheduling engine manages a queue of tasks, considering dependencies to ensure correct execution order.
- **Multi-Agent Coordination Engine**: A central coordinator assigns tasks to available agents and manages their states (`IDLE`, `BUSY`).
- **Structured and Testable Codebase**: The project is organized into a clean `src` and `test` directory structure, with a suite of unit tests to ensure reliability.
- **Ecosystem Integration**: Integrated with Quanta Tissu, Ethos, Haba, and Glia repositories.
- **Proven Workflows**: 50 workflows (25 basic and 25 enhanced) have been factually validated across the integrated repositories.

## Architecture Overview

QuantaLista is built on a distributed, multi-repository architecture. Each component is a distinct service, designed for independent development, deployment, and scaling.

- **`QuantaLista-Core`**: The central engine that orchestrates the entire process. It contains the `Coordinator`, `Scheduler`, and `AgentManager`.
- **`QuantaLista-Gateway`**: A lightweight API gateway providing a unified entry point.
- **`QuantaLista-Advisor`**: A Python-based service providing oversight and recommendations.
- **`QuantaLista-UI`**: A web-based interface for workflow visualization.

### Integrated Quanta Repositories:
- **quanta_tissu**: Low-level neural/tensor operations and tokenization.
- **quanta_lista**: Task scheduling and workflow orchestration (this repository).
- **quanta_ethos**: Ethical validation and trustworthiness scoring.
- **quanta_haba**: Structured design authoring and visualization.
- **quanta_glia**: Global memory and knowledge management.

## Getting Started

To get started, please refer to the `README.md` file in each of the following repositories:
- `QuantaLista-Core`
- `QuantaLista-Gateway`
- `QuantaLista-Advisor`
- `QuantaLista-UI`

### Local Configuration (.quanta)
For integration with the full ecosystem and local model, create a `.quanta` file in the root directory to specify local paths:
```ini
quanta_ethos.path=/path/to/quanta_ethos
quanta_tissu.path=/path/to/quanta_tissu
quanta_haba.path=/path/to/quanta_haba
quanta_glia.path=/path/to/quanta_glia
engine.model_path=/path/to/model.gguf
model.llama_cli_path=/path/to/llama-cli
```

## Documentation
Additional plans and roadmap details are available in the `docs/` directory:
- `integration_plan.md`: Details the 50 proven workflows and 50 future roadmap items.
- `implementation_plan.md`: Technical strategy for multi-repo validation.
- `enhanced_integration_plan.md`: Advanced multi-repo loops and self-healing logic.

## Testing

The repository includes a lightweight test framework in `test/test_framework.h` and several test suites:

- `test/unit_tests.cpp`: Unit-style tests for core logic.
- `test/integration/real_integration_tests.cpp`: Integration tests for basic multi-repo workflows.
- `test/integration/enhanced_integration_tests.cpp`: Integration tests for advanced ecosystem loops.

Build and run integration tests:
```bash
make real_integration_tests
make enhanced_integration_tests
```
