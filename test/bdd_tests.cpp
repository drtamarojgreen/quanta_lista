#include "test_framework.h"
#include <memory>

void feature_agent_lifecycle() {
    Gherkin::feature("Agent Lifecycle Management")

        .scenario("An agent begins in IDLE state upon registration")
        .given("an AgentManager with no registered agents", []() {
            // context established in when/then via shared state below
        })
        .when("a new agent is registered", []() {
            // see then — state shared via captures in full scenarios
        })
        .then("the agent is available as idle", []() {
            Publisher pub;
            AgentManager am(pub);
            am.registerAgent(Agent("a1", "Alpha"));

            Agent* agent = am.getIdleAgent();
            Assert::not_null(agent, "Agent should be returned as idle");
            Assert::equal(agent->state, AgentState::IDLE, "State must be IDLE");
        })

        .scenario("An IDLE agent transitions to BUSY when assigned work")
        .given("an idle agent named Beta", []() {})
        .when("Beta's state is set to BUSY", []() {})
        .then("Beta is no longer returned as an idle agent", []() {
            Publisher pub;
            AgentManager am(pub);
            am.registerAgent(Agent("beta", "Beta"));
            am.setAgentState("beta", AgentState::BUSY);

            Assert::is_null(am.getIdleAgent(), "No idle agent expected while Beta is BUSY");
        })
        .and_("Beta's state is retrievable as BUSY", []() {
            Publisher pub;
            AgentManager am(pub);
            am.registerAgent(Agent("beta", "Beta"));
            am.setAgentState("beta", AgentState::BUSY);

            const Agent* a = am.getAgent("beta");
            Assert::not_null(a, "Agent should still be retrievable");
            Assert::equal(a->state, AgentState::BUSY, "State must be BUSY");
        })

        .scenario("A BUSY agent returns to IDLE after completing work")
        .given("agent Gamma is currently BUSY", []() {})
        .when("Gamma's state is set back to IDLE", []() {})
        .then("Gamma is once again returned as an idle agent", []() {
            Publisher pub;
            AgentManager am(pub);
            am.registerAgent(Agent("gamma", "Gamma"));
            am.setAgentState("gamma", AgentState::BUSY);
            am.setAgentState("gamma", AgentState::IDLE);

            Agent* agent = am.getIdleAgent();
            Assert::not_null(agent, "Gamma should be idle again");
            Assert::equal(agent->id, std::string("gamma"), "Correct agent returned");
        })

        .scenario("An agent in ERROR state is not selected for new work")
        .given("agent Delta is in ERROR state", []() {})
        .when("the coordinator looks for an idle agent", []() {})
        .then("no idle agent is returned", []() {
            Publisher pub;
            AgentManager am(pub);
            am.registerAgent(Agent("delta", "Delta"));
            am.setAgentState("delta", AgentState::ERROR);

            Assert::is_null(am.getIdleAgent(), "ERROR agent must not be selected as idle");
        })

        .scenario("The idle agent is preferred over a busy one in a pool")
        .given("a pool with one BUSY agent and one IDLE agent", []() {})
        .when("the coordinator requests an idle agent", []() {})
        .then("only the IDLE agent is returned", []() {
            Publisher pub;
            AgentManager am(pub);
            am.registerAgent(Agent("busy_one", "BusyAgent"));
            am.registerAgent(Agent("idle_one", "IdleAgent"));
            am.setAgentState("busy_one", AgentState::BUSY);

            Agent* selected = am.getIdleAgent();
            Assert::not_null(selected, "An idle agent should be found");
            Assert::equal(selected->id, std::string("idle_one"), "Idle agent should be selected");
        })

        .run();
}

void feature_task_scheduling() {
    Gherkin::feature("Task Scheduling and Priority")

        .scenario("A high-priority task is scheduled before a low-priority task")
        .given("a scheduler containing a low-priority task and a high-priority task", []() {})
        .when("the next available task is requested", []() {})
        .then("the high-priority task is returned first", []() {
            Publisher pub;
            Scheduler scheduler(pub);
            scheduler.submitTask(Task("low", "low task", "low", {}, "c", 1));
            scheduler.submitTask(Task("high", "high task", "high", {}, "c", 1));

            Task* t = scheduler.getNextAvailableTask();
            Assert::not_null(t, "A task should be returned");
            Assert::equal(t->task_id, std::string("high"), "High-priority task must come first");
        })

        .scenario("Tasks are served in high, medium, then low priority order")
        .given("a scheduler with one task at each priority level", []() {})
        .when("tasks are dequeued one by one", []() {})
        .then("they arrive in high, medium, low order", []() {
            Publisher pub;
            Scheduler scheduler(pub);
            scheduler.submitTask(Task("lo", "low", "low", {}, "c", 1));
            scheduler.submitTask(Task("hi", "high", "high", {}, "c", 1));
            scheduler.submitTask(Task("me", "medium", "medium", {}, "c", 1));

            Assert::equal(scheduler.getNextAvailableTask()->task_id, std::string("hi"), "1st: high");
            Assert::equal(scheduler.getNextAvailableTask()->task_id, std::string("me"), "2nd: medium");
            Assert::equal(scheduler.getNextAvailableTask()->task_id, std::string("lo"), "3rd: low");
        })

        .scenario("A task with an unmet dependency is not scheduled")
        .given("a scheduler with a task whose dependency does not exist", []() {})
        .when("the next available task is requested", []() {})
        .then("no task is returned", []() {
            Publisher pub;
            Scheduler scheduler(pub);
            scheduler.submitTask(Task("child", "child", "high", {"missing_parent"}, "c", 1));

            Assert::is_null(scheduler.getNextAvailableTask(), "Blocked task should not be scheduled");
        })

        .scenario("A dependent task becomes schedulable once its parent completes")
        .given("a parent task and a child task that depends on it", []() {})
        .when("the parent is completed", []() {})
        .then("the child becomes the next available task", []() {
            Publisher pub;
            Scheduler scheduler(pub);
            scheduler.submitTask(Task("parent", "parent", "high", {}, "c", 1));
            scheduler.submitTask(Task("child", "child", "high", {"parent"}, "c", 1));

            Task* p = scheduler.getNextAvailableTask();
            Assert::equal(p->task_id, std::string("parent"), "Parent must be first");

            Assert::is_null(scheduler.getNextAvailableTask(), "Child must be blocked before parent completes");

            scheduler.markTaskAsCompleted("parent");

            Task* c = scheduler.getNextAvailableTask();
            Assert::not_null(c, "Child should be available after parent completes");
            Assert::equal(c->task_id, std::string("child"), "Child task should be unblocked");
        })

        .scenario("A three-task chain executes strictly in dependency order")
        .given("tasks T1, T2 (depends on T1), and T3 (depends on T2)", []() {})
        .when("tasks are dequeued and completed one at a time", []() {})
        .then("they complete in the order T1, T2, T3", []() {
            Publisher pub;
            Scheduler scheduler(pub);
            scheduler.submitTask(Task("t1", "step 1", "high", {}, "c", 1));
            scheduler.submitTask(Task("t2", "step 2", "high", {"t1"}, "c", 1));
            scheduler.submitTask(Task("t3", "step 3", "high", {"t2"}, "c", 1));

            auto advance = [&](const std::string& expected_id) {
                Task* t = scheduler.getNextAvailableTask();
                Assert::not_null(t, "Expected task: " + expected_id);
                Assert::equal(t->task_id, expected_id, "Wrong task in chain");
                scheduler.markTaskAsCompleted(t->task_id);
            };
            advance("t1");
            advance("t2");
            advance("t3");
        })

        .scenario("An empty scheduler returns no tasks")
        .given("a scheduler with no submitted tasks", []() {})
        .when("the next available task is requested", []() {})
        .then("null is returned", []() {
            Publisher pub;
            Scheduler scheduler(pub);
            Assert::is_null(scheduler.getNextAvailableTask(), "Empty scheduler must return null");
        })

        .run();
}

void feature_event_publishing() {
    Gherkin::feature("Event Publishing")

        .scenario("Registering an agent fires an AgentStateChanged(IDLE) event")
        .given("a Publisher with a subscriber for AgentStateChanged events", []() {})
        .when("a new agent is registered with the AgentManager", []() {})
        .then("exactly one AgentStateChanged event is published", []() {
            Publisher pub;
            MockSubscriber sub;
            pub.subscribe(EventType::AgentStateChanged, &sub);
            AgentManager am(pub);

            am.registerAgent(Agent("a1", "Alpha"));

            Assert::size_equals(sub.received_events, size_t(1), "Expected exactly 1 event");
        })
        .and_("the event carries the correct agent ID and IDLE state", []() {
            Publisher pub;
            MockSubscriber sub;
            pub.subscribe(EventType::AgentStateChanged, &sub);
            AgentManager am(pub);
            am.registerAgent(Agent("a1", "Alpha"));

            const auto& e = sub.get<AgentStateChangedEvent>(0);
            Assert::equal(e.agent_id, std::string("a1"), "Event agent_id must match");
            Assert::equal(e.new_state, AgentState::IDLE, "Event state must be IDLE");
        })

        .scenario("Setting an agent to the same state does not publish a new event")
        .given("an idle agent and a subscriber", []() {})
        .when("the agent's state is set to IDLE again (no change)", []() {})
        .then("no additional event is published", []() {
            Publisher pub;
            MockSubscriber sub;
            pub.subscribe(EventType::AgentStateChanged, &sub);
            AgentManager am(pub);
            am.registerAgent(Agent("a1", "Alpha"));
            sub.clear();

            am.setAgentState("a1", AgentState::IDLE);

            Assert::size_equals(sub.received_events, size_t(0), "No event expected for same-state transition");
        })

        .scenario("Submitting a task publishes TaskCreated and TaskStatusChanged(Pending) events")
        .given("a Scheduler with subscribers for TaskCreated and TaskStatusChanged", []() {})
        .when("a task is submitted", []() {})
        .then("a TaskCreated event is fired with the task's ID and description", []() {
            Publisher pub;
            MockSubscriber sub;
            pub.subscribe(EventType::TaskCreated, &sub);
            pub.subscribe(EventType::TaskStatusChanged, &sub);
            Scheduler scheduler(pub);
            scheduler.submitTask(Task("t1", "Do something", "high", {}, "c", 5));

            Assert::equal(sub.count_of<TaskCreatedEvent>(), 1, "Expected 1 TaskCreated event");
            const auto& created = sub.get<TaskCreatedEvent>(0);
            Assert::equal(created.task_id, std::string("t1"), "TaskCreated: task_id");
            Assert::equal(created.description, std::string("Do something"), "TaskCreated: description");
        })
        .and_("a TaskStatusChanged(Pending) event is fired", []() {
            Publisher pub;
            MockSubscriber sub;
            pub.subscribe(EventType::TaskCreated, &sub);
            pub.subscribe(EventType::TaskStatusChanged, &sub);
            Scheduler scheduler(pub);
            scheduler.submitTask(Task("t1", "Do something", "high", {}, "c", 5));

            Assert::equal(sub.count_of<TaskStatusChangedEvent>(), 1, "Expected 1 TaskStatusChanged event");
            const auto& status = sub.get<TaskStatusChangedEvent>(1);
            Assert::equal(status.new_status, TaskStatus::Pending, "Initial status must be Pending");
        })

        .scenario("Completing a task fires a TaskStatusChanged(Completed) event")
        .given("a scheduler with one task that has been moved to InProgress", []() {})
        .when("the task is marked as completed", []() {})
        .then("a TaskStatusChanged(Completed) event is published", []() {
            Publisher pub;
            MockSubscriber sub;
            pub.subscribe(EventType::TaskStatusChanged, &sub);
            Scheduler scheduler(pub);
            scheduler.submitTask(Task("t1", "task", "high", {}, "c", 1));
            scheduler.getNextAvailableTask();
            sub.clear();

            scheduler.markTaskAsCompleted("t1");

            Assert::size_equals(sub.received_events, size_t(1), "Expected exactly 1 event on completion");
            const auto& e = sub.get<TaskStatusChangedEvent>(0);
            Assert::equal(e.new_status, TaskStatus::Completed, "Status must be Completed");
        })

        .scenario("Unsubscribing stops further event delivery to that subscriber")
        .given("a subscriber registered for AgentStateChanged events", []() {})
        .when("the subscriber is unsubscribed before an agent is registered", []() {})
        .then("the subscriber receives no events", []() {
            Publisher pub;
            MockSubscriber sub;
            pub.subscribe(EventType::AgentStateChanged, &sub);
            pub.unsubscribe(EventType::AgentStateChanged, &sub);
            AgentManager am(pub);

            am.registerAgent(Agent("a1", "Alpha"));

            Assert::size_equals(sub.received_events, size_t(0), "No events should be received after unsubscribe");
        })

        .scenario("A full coordinator run fires events in the correct sequence")
        .given("a project with two tasks: T1 (high) and T2 (low, depends on T1)", []() {})
        .and_("one registered agent", []() {})
        .when("the coordinator runs to completion", []() {})
        .then("11 events fire in the expected agent-state and task-status sequence", []() {
            Project project("p1", "BDD Project");
            Workflow workflow("wf1", "WF");
            workflow.addTask(Task("t1", "task 1", "high", {}, "c1", 1));
            workflow.addTask(Task("t2", "task 2", "low", {"t1"}, "c1", 1));
            project.addWorkflow(workflow);

            Coordinator coordinator(project, "./queue_bdd_seq");
            MockSubscriber sub;
            Publisher& pub = coordinator.getEventPublisher();
            pub.subscribe(EventType::AgentStateChanged, &sub);
            pub.subscribe(EventType::TaskStatusChanged, &sub);

            coordinator.registerAgent(Agent("bdd_agent", "BDDAgent"));
            coordinator.run();

            Assert::size_equals(sub.received_events, size_t(11), "Expected 11 events total");
            Assert::equal(sub.count_of<AgentStateChangedEvent>(), 5, "Expected 5 AgentStateChanged events");
            Assert::equal(sub.count_of<TaskStatusChangedEvent>(), 6,
                          "Expected 6 TaskStatusChanged events (2 Pending + 2 InProgress + 2 Completed)");

            const auto& e0 = sub.get<AgentStateChangedEvent>(0);
            Assert::equal(e0.agent_id, std::string("bdd_agent"), "Event 0: agent_id");
            Assert::equal(e0.new_state, AgentState::IDLE, "Event 0: IDLE on registration");

            const auto& e1 = sub.get<TaskStatusChangedEvent>(1);
            Assert::equal(e1.new_status, TaskStatus::Pending, "Event 1: t1 Pending");

            const auto& e2 = sub.get<TaskStatusChangedEvent>(2);
            Assert::equal(e2.new_status, TaskStatus::Pending, "Event 2: t2 Pending");

            const auto& e3 = sub.get<TaskStatusChangedEvent>(3);
            Assert::equal(e3.task_id, std::string("t1"), "Event 3: t1 InProgress");
            Assert::equal(e3.new_status, TaskStatus::InProgress, "Event 3: status InProgress");

            const auto& e10 = sub.get<AgentStateChangedEvent>(10);
            Assert::equal(e10.agent_id, std::string("bdd_agent"), "Event 10: final agent_id");
            Assert::equal(e10.new_state, AgentState::IDLE, "Event 10: IDLE after all work");
        })

        .run();
}

void feature_json_serialization() {
    Gherkin::feature("Task JSON Serialization")

        .scenario("A fully-populated task survives a JSON round-trip without data loss")
        .given("a Task with ID, description, priority, component, runtime, and two dependencies", []() {})
        .when("the task is serialised to JSON and deserialised back", []() {})
        .then("all fields on the restored task match the original", []() {
            Task original("t1", "Do something", "high", {"dep_a", "dep_b"}, "module_x", 60);
            Task restored = from_json(to_json(original));

            Assert::equal(restored.task_id, original.task_id, "task_id");
            Assert::equal(restored.description, original.description, "description");
            Assert::equal(restored.priority, original.priority, "priority");
            Assert::equal(restored.component, original.component, "component");
            Assert::equal(restored.max_runtime_sec, original.max_runtime_sec, "max_runtime_sec");
            Assert::size_equals(restored.dependencies, size_t(2), "dependency count");
            Assert::equal(restored.dependencies[0], std::string("dep_a"), "dependency[0]");
            Assert::equal(restored.dependencies[1], std::string("dep_b"), "dependency[1]");
        })

        .scenario("A task with no dependencies round-trips without inventing dependencies")
        .given("a Task with an empty dependencies list", []() {})
        .when("the task is serialised and deserialised", []() {})
        .then("the restored task's dependency list is empty", []() {
            Task original("t2", "Solo task", "low", {}, "c", 10);
            Task restored = from_json(to_json(original));

            Assert::is_true(restored.dependencies.empty(), "Empty dependency list should be preserved");
        })

        .scenario("A task with a single dependency preserves that dependency exactly")
        .given("a Task with exactly one dependency ID", []() {})
        .when("the task is serialised and deserialised", []() {})
        .then("the restored task has exactly one dependency with the correct ID", []() {
            Task original("t3", "Chained task", "medium", {"sole_dep"}, "c", 5);
            Task restored = from_json(to_json(original));

            Assert::size_equals(restored.dependencies, size_t(1), "Exactly one dependency");
            Assert::equal(restored.dependencies[0], std::string("sole_dep"), "Dependency ID must match");
        })

        .run();
}

void feature_multi_agent_coordination() {
    Gherkin::feature("Multi-Agent Coordination")

        .scenario("A single task assigned to a single agent completes successfully")
        .given("a project with one task and one registered agent", []() {})
        .when("the coordinator runs", []() {})
        .then("the task appears in the completed task list", []() {
            system("rm -rf ./queue_bdd_solo");
            Project project("p", "Solo Project");
            Workflow wf("w", "WF");
            wf.addTask(Task("t1", "sole task", "high", {}, "c", 1));
            project.addWorkflow(wf);

            Coordinator coordinator(project, "./queue_bdd_solo");
            coordinator.registerAgent(Agent("a1", "Solo"));
            coordinator.run();

            Assert::size_equals(coordinator.getScheduler().getCompletedTaskIds(), size_t(1), "One task should complete");
        })

        .scenario("Tasks complete in priority order when no dependencies exist")
        .given("a project with high, medium, and low priority independent tasks", []() {})
        .and_("a single agent", []() {})
        .when("the coordinator runs", []() {})
        .then("tasks complete in high, medium, low order", []() {
            system("rm -rf ./queue_bdd_prio");
            Project project("p", "Priority Project");
            Workflow wf("w", "WF");
            wf.addTask(Task("t_lo", "low", "low", {}, "c", 1));
            wf.addTask(Task("t_hi", "high", "high", {}, "c", 1));
            wf.addTask(Task("t_me", "med", "medium", {}, "c", 1));
            project.addWorkflow(wf);

            Coordinator coordinator(project, "./queue_bdd_prio");
            coordinator.registerAgent(Agent("a1", "Solo"));
            coordinator.run();

            const auto& c = coordinator.getScheduler().getCompletedTaskIds();
            Assert::size_equals(c, size_t(3), "All 3 tasks should complete");
            Assert::equal(c[0], std::string("t_hi"), "1st: high priority");
            Assert::equal(c[1], std::string("t_me"), "2nd: medium priority");
            Assert::equal(c[2], std::string("t_lo"), "3rd: low priority");
        })

        .scenario("A four-task dependency chain completes in the correct sequence")
        .given("a project with tasks T1→T2→T3→T4 in a linear dependency chain", []() {})
        .and_("two available agents", []() {})
        .when("the coordinator runs to completion", []() {})
        .then("all four tasks complete", []() {
            system("rm -rf ./queue_bdd_chain");
            Project project("p1", "Chain Project");
            Workflow wf("wf1", "WF");
            wf.addTask(Task("t1", "analyze", "high", {}, "c", 1));
            wf.addTask(Task("t2", "design", "high", {"t1"}, "c", 1));
            wf.addTask(Task("t3", "implement", "medium", {"t2"}, "c", 1));
            wf.addTask(Task("t4", "document", "low", {"t3"}, "c", 1));
            project.addWorkflow(wf);

            Coordinator coordinator(project, "./queue_bdd_chain");
            coordinator.registerAgent(Agent("a1", "Worker"));
            coordinator.registerAgent(Agent("a2", "Helper"));
            coordinator.run();

            Assert::size_equals(coordinator.getScheduler().getCompletedTaskIds(), size_t(4), "All 4 tasks must complete");
        })
        .and_("each task completes after all its predecessors", []() {
            system("rm -rf ./queue_bdd_chain2");
            Project project("p1", "Chain Project");
            Workflow wf("wf1", "WF");
            wf.addTask(Task("t1", "analyze", "high", {}, "c", 1));
            wf.addTask(Task("t2", "design", "high", {"t1"}, "c", 1));
            wf.addTask(Task("t3", "implement", "medium", {"t2"}, "c", 1));
            wf.addTask(Task("t4", "document", "low", {"t3"}, "c", 1));
            project.addWorkflow(wf);

            Coordinator coordinator(project, "./queue_bdd_chain2");
            coordinator.registerAgent(Agent("a1", "Worker"));
            coordinator.run();

            const auto& c = coordinator.getScheduler().getCompletedTaskIds();
            auto pos = [&](const std::string& id) -> int {
                for (int i = 0; i < static_cast<int>(c.size()); ++i) {
                    if (c[i] == id) {
                        return i;
                    }
                }
                return -1;
            };
            Assert::is_true(pos("t1") < pos("t2"), "t1 must complete before t2");
            Assert::is_true(pos("t2") < pos("t3"), "t2 must complete before t3");
            Assert::is_true(pos("t3") < pos("t4"), "t3 must complete before t4");
        })

        .scenario("A mixed-priority project with dependencies respects both constraints")
        .given("tasks T1 (high), T2 (high, depends on T1), T3 (medium), T4 (low, depends on T3)", []() {})
        .and_("a single agent", []() {})
        .when("the coordinator runs", []() {})
        .then("T1 runs first, followed by T2, then T3, then T4", []() {
            system("rm -rf ./queue_bdd_mixed");
            Project project("p1", "Mixed Project");
            Workflow wf("wf1", "WF");
            wf.addTask(Task("t1", "a", "high", {}, "c", 1));
            wf.addTask(Task("t2", "b", "high", {"t1"}, "c", 1));
            wf.addTask(Task("t3", "c", "medium", {}, "c", 1));
            wf.addTask(Task("t4", "d", "low", {"t3"}, "c", 1));
            project.addWorkflow(wf);

            Coordinator coordinator(project, "./queue_bdd_mixed");
            coordinator.registerAgent(Agent("a1", "Worker"));
            coordinator.run();

            const auto& c = coordinator.getScheduler().getCompletedTaskIds();
            Assert::size_equals(c, size_t(4), "All 4 tasks must complete");
            Assert::equal(c[0], std::string("t1"), "1st: t1 (high, unblocked)");
            Assert::equal(c[1], std::string("t2"), "2nd: t2 (high, after t1)");
            Assert::equal(c[2], std::string("t3"), "3rd: t3 (medium, unblocked)");
            Assert::equal(c[3], std::string("t4"), "4th: t4 (low, after t3)");
        })

        .run();
}

int main() {
    std::cout << Color::BOLD << "\n══════════════════════════════════════════\n"
              << "  QuantaLista — BDD Tests (Gherkin)\n"
              << "══════════════════════════════════════════\n"
              << Color::RESET;

    feature_agent_lifecycle();
    feature_task_scheduling();
    feature_event_publishing();
    feature_json_serialization();
    feature_multi_agent_coordination();

    TestRegistry::print_summary();
    return TestRegistry::failed_count() > 0 ? 1 : 0;
}
