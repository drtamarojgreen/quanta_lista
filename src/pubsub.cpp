#include "pubsub.h"

void Publisher::subscribe(EventType eventType, ISubscriber* subscriber) {
    if (subscriber) {
        subscribers[eventType].push_back(subscriber);
    }
}

void Publisher::unsubscribe(EventType eventType, ISubscriber* subscriber) {
    if (subscribers.count(eventType)) {
        auto& subs = subscribers.at(eventType);
        subs.erase(std::remove(subs.begin(), subs.end(), subscriber), subs.end());
    }
}

void Publisher::publish(const Event& event) {
    // Notify subscribers for the specific event type
    if (subscribers.count(event.type)) {
        // Create a copy of the subscriber list to iterate over,
        // in case a subscriber modifies the list during notification.
        auto subs_copy = subscribers.at(event.type);
        for (ISubscriber* subscriber : subs_copy) {
            subscriber->onEvent(event);
        }
    }
}
