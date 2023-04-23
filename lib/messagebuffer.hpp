#pragma once
#include <array>
#include <string>

namespace IMNet
{
    // A non-owning view of a buffer that allows read/write of primitives + strings
    class MessageBuffer
    {
    public:
        MessageBuffer(uint8_t *data, uint32_t size);
        virtual ~MessageBuffer() = default;
        void reset();
        bool advance_byte() const; // Moves the current read/write head forwards by one byte
        bool eof() const;
        void *buffer() const;

        virtual uint32_t size() const = 0;

    protected:
        const uint32_t m_capacity;
        uint8_t *m_buffer = nullptr;
        mutable uint32_t m_head = 0;
    };

    class MessageBufferRead : public MessageBuffer
    {
    public:
        MessageBufferRead(const uint8_t *data, uint32_t size);
        std::string read_string() const;
        uint8_t read_byte() const;
        uint32_t read_int() const;
        uint32_t size() const override;
    };

    class MessageBufferWrite : public MessageBuffer
    {
    public:
        MessageBufferWrite(uint8_t *data, uint32_t size);

        bool write(uint8_t value);
        bool write(uint32_t value);
        bool write(const std::string &);
        uint32_t size() const override;
    };

    // Message buffer that allocates its own data and controls the lifetime
    class OwningMessageBuffer final : public MessageBufferWrite
    {
    public:
        OwningMessageBuffer(size_t size);
        ~OwningMessageBuffer();
    };
}