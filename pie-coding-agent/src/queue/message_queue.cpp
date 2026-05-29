#include "pie/queue/message_queue.hpp"

namespace pie::queue {

void MessageQueue::push_steering(const std::string& msg) {
    std::lock_guard<std::mutex> lock(mu_);
    steering_.push_back(msg);
}

void MessageQueue::push_follow_up(const std::string& msg) {
    std::lock_guard<std::mutex> lock(mu_);
    follow_up_.push_back(msg);
}

std::vector<std::string> MessageQueue::drain(std::deque<std::string>& q, DrainMode mode) {
    std::vector<std::string> result;
    if (mode == DrainMode::OneAtATime) {
        if (!q.empty()) { result.push_back(q.front()); q.pop_front(); }
    } else {
        result.assign(q.begin(), q.end());
        q.clear();
    }
    return result;
}

std::vector<std::string> MessageQueue::drain_steering(DrainMode mode) {
    std::lock_guard<std::mutex> lock(mu_);
    return drain(steering_, mode);
}

std::vector<std::string> MessageQueue::drain_follow_up(DrainMode mode) {
    std::lock_guard<std::mutex> lock(mu_);
    return drain(follow_up_, mode);
}

std::vector<std::string> MessageQueue::drain_all() {
    std::lock_guard<std::mutex> lock(mu_);
    std::vector<std::string> result;
    result.insert(result.end(), steering_.begin(), steering_.end());
    result.insert(result.end(), follow_up_.begin(), follow_up_.end());
    steering_.clear();
    follow_up_.clear();
    return result;
}

size_t MessageQueue::steering_size() const {
    std::lock_guard<std::mutex> lock(mu_);
    return steering_.size();
}

size_t MessageQueue::follow_up_size() const {
    std::lock_guard<std::mutex> lock(mu_);
    return follow_up_.size();
}

}  // namespace pie::queue
