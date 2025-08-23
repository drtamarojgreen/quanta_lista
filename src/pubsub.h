#ifndef PUBSUB_H
#define PUBSUB_H

#include <vector>
#include <algorithm>
#include <map>
#include "QuantaLista.h" // For Event base class and EventType enum

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

#endif // PUBSUB_H
