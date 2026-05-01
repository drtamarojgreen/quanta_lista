#ifndef TASK_SCHEDULING_STEPS_H
#define TASK_SCHEDULING_STEPS_H

namespace TaskSchedulingSteps {
void high_before_low();
void priority_order_hml();
void unmet_dependency_not_scheduled();
void child_unblocked_after_parent_complete();
void chain_executes_in_order();
void empty_scheduler_returns_null();
}

#endif
