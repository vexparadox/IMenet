#include "inputhelper.hpp"
#include <iostream>

namespace IMenet
{
    InputHelper::~InputHelper()
    {
        stop();
    }

    void InputHelper::stop()
    {
        m_running = false;
        if(m_thread.joinable())
        {
            m_thread.join();
        }
    }

    bool InputHelper::run()
    {
        if (m_running.load())
        {
            return false;
        }
        m_running = true;
        m_thread = std::thread([&]()
                               {
            while(m_running.load())
            {
                std::string value;
                std::getline(std::cin, value);
                // this printf clears the last line (so what the character wrote)
                // #TODO make this optional
                printf("\033[1A");
                printf("\033[K");
                if(value.empty() == false)
                {
                    std::scoped_lock lock(m_input_mutex);
                    m_inputs.emplace_back(std::move(value));
                }
            } });
        return true;
    }

    std::vector<std::string> InputHelper::poll()
    {
        std::scoped_lock lock(m_input_mutex);
        std::vector<std::string> values = std::move(m_inputs);
        m_inputs = {};
        return values;
    }
}