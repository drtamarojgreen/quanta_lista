#include "QuantaLista.h"

// QuantaLista main application entry point
int main() {
    // 1. Create the main coordinator for the application
    Coordinator coordinator;

    // 2. Define the project, including agents and workflows
    Project my_project("proj_001", "Autonomous Research Agent Development");
    my_project.addAgent(Agent("agent_01", "Researcher"));
    my_project.addAgent(Agent("agent_02", "Writer"));

    Workflow research_workflow("wf_01", "Data Gathering and Analysis");
    research_workflow.addTask(Task("task_01", "Gather sources", "Find papers", "Pending", {}));
    research_workflow.addTask(Task("task_02", "Summarize papers", "Create summaries", "Pending", {"task_01"}));
    research_workflow.addTask(Task("task_03", "Synthesize findings", "Write review", "Pending", {"task_02"}));

    my_project.addWorkflow(research_workflow);

    // 3. Setup the coordinator with the project definition
    coordinator.setupProject(my_project);

    // 4. Run the simulation
    coordinator.run();

    return 0;
}
