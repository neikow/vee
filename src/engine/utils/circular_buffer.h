#ifndef VEE_CIRCULAR_BUFFER_H
#define VEE_CIRCULAR_BUFFER_H
#include <vector>

template<typename T>
class CircularBuffer {
    std::vector<T> m_Buffer;
    size_t m_Capacity;
    // Index of the next inserted element.
    size_t m_Head = 0;
    bool m_Full = false;

public:
    explicit CircularBuffer(size_t capacity) : m_Capacity(capacity) {
        if (capacity == 0) {
            throw std::invalid_argument("Capacity must be greater than 0");
        }
        m_Buffer.resize(capacity);
    }

    // Add an element to the buffer.
    void Push(const T &item) {
        m_Buffer[m_Head] = item;
        m_Head = (m_Head + 1) % m_Capacity;
        if (m_Head == 0) {
            m_Full = true;
        }
    }

    [[nodiscard]] size_t Size() const {
        if (m_Full) {
            return m_Capacity;
        }
        return m_Head;
    }

    const T &Get(const size_t index) const {
        if (index >= Size()) {
            throw std::out_of_range("Index out of bounds for current buffer size.");
        }
        const size_t start_index = m_Full ? m_Head : 0;
        size_t physical_index = (start_index + index) % m_Capacity;
        return m_Buffer[physical_index];
    }

    std::vector<T> AsVector() {
        std::vector<T> result;
        result.reserve(Size());

        if (m_Full) {
            for (size_t i = 0; i < m_Capacity; ++i) {
                result.push_back(m_Buffer[(m_Head + i) % m_Capacity]);
            }
        } else {
            for (size_t i = 0; i < m_Head; ++i) {
                result.push_back(m_Buffer[i]);
            }
        }

        return result;
    }

    void Clear() {
        m_Head = 0;
        m_Full = false;
    };
};

#endif //VEE_CIRCULAR_BUFFER_H
