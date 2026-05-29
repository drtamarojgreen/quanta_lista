#include "events.h"
#include <algorithm>

void Publisher::subscribe(EventType eventType, ISubscriber* subscriber) {
    subscribers[eventType].push_back(subscriber);
}

void Publisher::unsubscribe(EventType eventType, ISubscriber* subscriber) {
    auto& subs = subscribers[eventType];
    subs.erase(std::remove(subs.begin(), subs.end(), subscriber), subs.end());
}

void Publisher::publish(const Event& event) {
    if (subscribers.count(event.type)) {
        for (auto subscriber : subscribers[event.type]) {
            subscriber->onEvent(event);
        }
    }
}
