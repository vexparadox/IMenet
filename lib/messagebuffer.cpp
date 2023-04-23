#include "messagebuffer.hpp"
namespace IMenet
{
    ////////////////////////////////////////////////////////////////////////
    // Owning
    OwningMessageBuffer::OwningMessageBuffer(size_t size) : MessageBufferWrite(new uint8_t[size](), size)
    {
    }

    OwningMessageBuffer::~OwningMessageBuffer()
    {
        delete m_buffer;
    }

    ////////////////////////////////////////////////////////////////////////
    // Write

    MessageBufferWrite::MessageBufferWrite(uint8_t *data, uint32_t size) : MessageBuffer(data, size)
    {
    }

    bool MessageBufferWrite::write(uint8_t value)
    {
        if (m_head + sizeof(uint8_t) < m_capacity)
        {
            *(uint8_t *)(m_buffer + m_head) = value;
            m_head += sizeof(uint8_t);
            return true;
        }
        return false;
    }

    bool MessageBufferWrite::write(uint32_t value)
    {
        if (m_head + sizeof(uint32_t) < m_capacity)
        {
            *(uint32_t *)(m_buffer + m_head) = value;
            m_head += sizeof(uint32_t);
            return true;
        }
        return false;
    }

    bool MessageBufferWrite::write(const std::string &value)
    {
        if (m_head + value.size() + sizeof(uint32_t) < m_capacity)
        {
            // #TODO need to check if the size can be written here for both the size and the string
            if (write((uint32_t)value.size()) == false)
            {
                return false;
            }
            strcpy((char *)(m_buffer + m_head), value.data());
            m_head += value.size() + 1; // +1 for the null terminator
            return true;
        }
        return false;
    }

    uint32_t MessageBufferWrite::size() const
    {
        // The number of bytes written
        return m_head;
    }

    ////////////////////////////////////////////////////////////////////////
    // Read

    MessageBufferRead::MessageBufferRead(const uint8_t *data, uint32_t size) : MessageBuffer(const_cast<uint8_t *>(data), size)
    {
    }

    std::string MessageBufferRead::read_string() const
    {
        const uint32_t str_length = read_int();
        if (str_length > 0)
        {
            std::string value = {(const char *)(m_buffer + m_head), str_length};
            m_head += value.size() + 1; // +1 here for the null terminator
            return value;
        }
        return {};
    }

    uint8_t MessageBufferRead::read_byte() const
    {
        if (m_head + sizeof(uint8_t) < m_capacity)
        {
            const uint8_t value = *(uint8_t *)(m_buffer + m_head);
            m_head += sizeof(uint8_t);
            return value;
        }
        return {};
    }

    uint32_t MessageBufferRead::read_int() const
    {
        if (m_head + sizeof(uint32_t) < m_capacity)
        {
            const uint32_t value = *(uint32_t *)(m_buffer + m_head);
            m_head += sizeof(uint32_t);
            return value;
        }
        return {};
    }

    uint32_t MessageBufferRead::size() const
    {
        return m_capacity;
    }

    ////////////////////////////////////////////////////////////////////////
    // Base

    MessageBuffer::MessageBuffer(uint8_t *data, uint32_t size)
        : m_capacity(size), m_buffer(data)
    {
    }

    void MessageBuffer::reset()
    {
        m_head = 0;
    }

    bool MessageBuffer::advance_byte() const
    {
        if(m_head + 1 < m_capacity)
        {
            m_head++;
            return true;
        }
        return false;
    }

    bool MessageBuffer::eof() const
    {
        return m_head >= m_capacity;
    }

    uint32_t MessageBuffer::size() const
    {
        // This needs some work
        return m_head;
    }

    void *MessageBuffer::buffer() const
    {
        return m_buffer;
    }
}