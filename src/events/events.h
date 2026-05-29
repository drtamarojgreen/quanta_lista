#ifndef EVENTS_H
#define EVENTS_H

#include <string>
#include <vector>
#include <chrono>
#include <map>
#include "../models/models.h"

// Base Event structure
enum class EventType {
    TaskCreated,
    TaskStatusChanged,
    AgentStateChanged
};

struct Event {
    virtual ~Event() = default;
    const EventType type;
    const std::chrono::system_clock::time_point timestamp;

protected:
    Event(EventType t) : type(t), timestamp(std::chrono::system_clock::now()) {}
};

struct TaskCreatedEvent : public Event {
    const std::string task_id;
    const std::string description;

    TaskCreatedEvent(std::string id, std::string desc)
        : Event(EventType::TaskCreated), task_id(std::move(id)), description(std::move(desc)) {}
};

struct TaskStatusChangedEvent : public Event {
    const std::string task_id;
    const TaskStatus new_status;

    TaskStatusChangedEvent(std::string id, TaskStatus status)
        : Event(EventType::TaskStatusChanged), task_id(std::move(id)), new_status(status) {}
};

struct AgentStateChangedEvent : public Event {
    const std::string agent_id;
    const AgentState new_state;

    AgentStateChangedEvent(std::string id, AgentState state)
        : Event(EventType::AgentStateChanged), agent_id(std::move(id)), new_state(state) {}
};

// Subscriber interface
class ISubscriber {
public:
    virtual ~ISubscriber() = default;
    virtual void onEvent(const Event& event) = 0;
};

// Publisher class
class Publisher {
public:
    void subscribe(EventType eventType, ISubscriber* subscriber);
    void unsubscribe(EventType eventType, ISubscriber* subscriber);
    void publish(const Event& event);

private:
    std::map<EventType, std::vector<ISubscriber*>> subscribers;
};

#endif // EVENTS_H
