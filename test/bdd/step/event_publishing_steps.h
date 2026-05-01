#ifndef EVENT_PUBLISHING_STEPS_H
#define EVENT_PUBLISHING_STEPS_H

namespace EventPublishingSteps {
void one_agent_state_changed_on_register();
void register_event_payload_correct();
void no_event_on_same_agent_state();
void task_created_event_published();
void task_pending_status_event_published();
void task_completed_status_event_published();
void unsubscribe_stops_delivery();
void coordinator_event_sequence();
}

#endif
