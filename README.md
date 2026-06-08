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

## Building

QuantaLista is currently built directly with `g++` and the repository `Makefile`. The codebase targets C++17.

### Prerequisites

- `g++` with C++17 support.
- `make`.
- For the cross-repository integration suites, local checkouts of `quanta_ethos`, `quanta_tissu`, `quanta_haba`, and `quanta_glia`, with their paths configured in `.quanta`.
- For real model execution through `ModelBackend`, a local GGUF model and `llama-cli` path configured in `.quanta`.

### Build the CLI

The Makefile is focused on test targets, so build the CLI executable directly:

```bash
g++ -std=c++17 -Isrc \
  src/main.cpp \
  src/cli/cli.cpp \
  src/core/core.cpp \
  src/events/events.cpp \
  src/models/ModelBackend.cpp \
  src/ui/SchedulerUI.cpp \
  src/utils/json_utils.cpp \
  -o quantalista
```

Run it with:

```bash
./quantalista help
./quantalista add task1 "Analyze requirements" high analysis 10
./quantalista list
./quantalista daemon
./quantalista ui dashboard
```

### Makefile Targets

The currently defined Makefile targets are:

- `make bridge_test`: builds `run_bridge_tests` and runs the bridge integration tests.
- `make real_integration_tests`: builds `run_real_integration_tests` and runs the W01-W25 real integration workflow suite.
- `make enhanced_integration_tests`: builds `run_enhanced_integration_tests` and runs the E01-E25 enhanced integration workflow suite.
- `make clean`: removes generated binaries listed in the Makefile.

The integration targets read repository paths from `.quanta` using keys such as `quanta_ethos.path`, `quanta_tissu.path`, `quanta_haba.path`, and `quanta_glia.path`. If those paths are missing or point to incompatible checkouts, the cross-repository targets will not compile.

## Documentation
Additional plans and roadmap details are available in the `docs/` directory:
- `integration_plan.md`: Details the 50 proven workflows and 50 future roadmap items.
- `implementation_plan.md`: Technical strategy for multi-repo validation.
- `enhanced_integration_plan.md`: Advanced multi-repo loops and self-healing logic.

## Testing

The repository includes a lightweight test framework in `test/test_framework.h` and several test suites. Some suites are wired into the Makefile, while others are source-level suites that can be compiled directly when needed.

### Quick Test Commands

Build and run the Makefile-supported suites:

```bash
make bridge_test
make real_integration_tests
make enhanced_integration_tests
```

Run generated binaries directly after building:

```bash
./run_bridge_tests
./run_real_integration_tests
./run_enhanced_integration_tests
```

### Test Suite Overview

- `test/unit/test_model_backend.cpp`: Verifies `ModelBackend` configuration loading. It creates a temporary `.quanta` file, confirms that configured model paths make the backend available, removes `.quanta`, and confirms the fallback error when no backend is configured.
- `test/unit/unit_tests.cpp`: Exercises core in-process behavior: agent registration and state transitions, scheduler priority and dependency handling, JSON round-trips for tasks and schedules, schedule persistence, CLI queue writes, coordinator/daemon processing order, topological sorting, duplicate import handling, and archive/restore behavior.
- `test/tests.cpp`: Legacy all-in-one test runner for core logic, CLI queue operations, daemon scheduling, and event-publishing behavior. It includes BDD-style event checks for agent registration, task submission, and full coordinator runs.
- `test/bdd/bdd_tests.cpp`: Gherkin-style BDD runner that groups behavior by feature: agent lifecycle, task scheduling, event publishing, JSON serialization, multi-agent coordination, and schedule management. The feature files delegate to step files under `test/bdd/step/`.
- `test/integration/bridge_tests.cpp`: Lightweight bridge tests using mocks. It checks that Lista can call a model backend abstraction and that an Ethos-like validator accepts safe input while rejecting unsafe input.
- `test/integration/workflow_e2e_tests.cpp`: End-to-end queue workflow test. It writes a task JSON file into `queue/pending`, runs `Coordinator::processPendingTasks()`, and verifies that the task moves to `queue/completed` with completion metadata.
- `test/integration/real_integration_tests.cpp`: Cross-repository workflow suite W01-W25. It validates Lista-led artifact ingestion, Tissu parsing, Ethos checks and scoring, Haba HTML generation/editor logic, Glia storage/curation/annotation/translation, scheduler dependency and priority behavior, PII/toxicity enforcement, knowledge export, gap analysis, and final plan delivery.
- `test/integration/enhanced_integration_tests.cpp`: Advanced cross-repository workflow suite E01-E25. It covers multi-repo dependency resolution, trustworthiness feedback loops, design refinement, context retrieval, task decomposition, redline enforcement, knowledge pruning, versioning, event propagation, model selection, semantic gap filling, resource constraints, self-correction, consensus review, memory synchronization, effort prediction, lineage, automated test generation, monitoring, access control, self-healing, and final all-repo synchronization.
- `test/integration/workflow_suite.cpp`: Smaller workflow harness for early workflow validation. It currently exercises initial W01/W02-style workflow checks and is useful as a compact example of the workflow test pattern.
- `test/sdd/`: Specification-by-data artifacts for scheduler behavior. `test/sdd/facts/scheduler.facts` stores scheduler facts, `SchedulerCards.cpp` defines scheduler cards, and the check-in/check-out markdown files document SDD review notes.

### Real Integration Workflow Coverage

`real_integration_tests.cpp` validates these named workflows:

- W01 Raw Artifact Scan
- W02 Semantic Clustering
- W03 Ethical Boundary Mapping
- W04 Design Structure Synthesis
- W05 Task Dependency Extraction
- W06 Knowledge Retrieval Indexing
- W07 Summary Drafting
- W08 Visual Roadmap Creation
- W09 Fact-Check Audit
- W10 Capability Mapping
- W11 Conflict Resolution
- W12 Stylistic Normalization
- W13 Dependency Integrity Check
- W14 Self-Improvement Loop
- W15 Artifact Quality Scoring
- W16 Enhancement Prioritization
- W17 Cross-Repo Indexing
- W18 Interactive Design Review
- W19 Automated Review Cycles
- W20 Constraint Enforcement
- W21 Visual Documentation Assembly
- W22 Knowledge Synthesis Export
- W23 Gap Analysis
- W24 Safety Compliance Verification
- W25 Final Plan Delivery

### Enhanced Integration Workflow Coverage

`enhanced_integration_tests.cpp` validates these named workflows:

- E01 Multi-Repo Dependency Resolution
- E02 Trustworthiness Feedback Loop
- E03 Interactive Design Refinement
- E04 Context-Aware Retrieval
- E05 Automated Task Decomposition
- E06 Ethical Redline Enforcement
- E07 Knowledge Base Pruning
- E08 Visual Design Versioning
- E09 Cross-Module Event Propagation
- E10 Collaborative Model Selection
- E11 Semantic Gap Filling
- E12 Toxicity Scrubbing of Design Docs
- E13 Resource-Constrained Scheduling
- E14 Cross-Language Code Analysis
- E15 Self-Correction Loop
- E16 Multi-Agent Consensus
- E17 Global Memory Synchronization
- E18 Task Effort Prediction
- E19 Ethical Bias Mitigation Plan
- E20 Artifact Lineage Reconstruction
- E21 Automated Test Generation
- E22 Real-time Performance Monitoring
- E23 Role-Based Access Control
- E24 Ecosystem-Wide Self-Healing
- E25 Final Integration Sync

### Build and Run Integration Tests

Use the Makefile targets for the two main cross-repository suites:

```bash
make real_integration_tests
make enhanced_integration_tests
```

These targets compile source files from the sibling Quanta repositories named in `.quanta`, then execute the resulting binaries.
