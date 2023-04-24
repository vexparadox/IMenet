#pragma once
#include <vector>
#include <string>
#include <thread>
namespace IMenet
{
    class InputHelper final
    {
    public:
        ~InputHelper();
        bool run();
        void stop();
        std::vector<std::string> poll(); // Returns the strings that have been entered since the last poll
    private:
        std::thread m_thread;
        std::atomic<bool> m_running{};
        std::mutex m_input_mutex;
        std::vector<std::string> m_inputs;
    };
}