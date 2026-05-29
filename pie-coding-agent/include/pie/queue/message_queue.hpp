#pragma once

#include <deque>
#include <mutex>
#include <string>
#include <vector>

namespace pie::queue {

enum class DrainMode { OneAtATime, All };

class MessageQueue {
public:
    void push_steering(const std::string& msg);
    void push_follow_up(const std::string& msg);

    std::vector<std::string> drain_steering(DrainMode mode = DrainMode::All);
    std::vector<std::string> drain_follow_up(DrainMode mode = DrainMode::All);
    std::vector<std::string> drain_all();

    size_t steering_size() const;
    size_t follow_up_size() const;

private:
    std::vector<std::string> drain(std::deque<std::string>& q, DrainMode mode);

    mutable std::mutex mu_;
    std::deque<std::string> steering_;
    std::deque<std::string> follow_up_;
};

}  // namespace pie::queue
