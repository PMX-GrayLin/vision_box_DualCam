#pragma once

#ifndef __OVERWRITE_GING_BUFFER_H__
#define __OVERWRITE_GING_BUFFER_H__

#include <vector>
#include <mutex>
#include <opencv2/opencv.hpp>

template<typename T>
class OverwriteRingBuffer {
public:
    OverwriteRingBuffer(size_t size) : buffer(size), head(0), tail(0) {}

    void push(const T& value) {
        std::unique_lock<std::mutex> lock(mutex);
        size_t next_head = (head + 1) % buffer.size();

        if (next_head == tail) {
            // buffer is full, increment tail
            tail = (tail + 1) % buffer.size();
        }

        buffer[head] = value.clone();
        head = next_head;
    }

    bool pop(T& value) {
        std::unique_lock<std::mutex> lock(mutex);
        if (tail == head) {
            return false; // buffer is empty
        }
        value = buffer[tail];
        tail = (tail + 1) % buffer.size();
        return true;
    }

    bool clear() {
        std::unique_lock<std::mutex> lock(mutex);
        head = tail;
        return true;
    }

    // deep copy
    //bool pop(T& value) {
    //    std::unique_lock<std::mutex> lock(mutex);
    //    if (tail == head) {
    //        return false; // buffer is empty
    //    }
    //    buffer[tail].copyTo(value);
    //    tail = (tail + 1) % buffer.size();
    //    return true;
    //}


    size_t size() const {
        std::unique_lock<std::mutex> lock(mutex);
        return (head - tail + buffer.size()) % buffer.size();
    }

    bool empty() const {
        std::unique_lock<std::mutex> lock(mutex);
        return head == tail;
    }

private:
    std::vector<T> buffer;
    size_t head;
    size_t tail;
    mutable std::mutex mutex;
};


template<typename T>
class TimestampedValue {
public:
    T value;
    std::chrono::time_point<std::chrono::system_clock> timestamp;

    TimestampedValue() : value(), timestamp(std::chrono::system_clock::now()) {}
    TimestampedValue(const T& val) : value(val.clone()), timestamp(std::chrono::system_clock::now()) {}

    // �ϥΪ�������ȦӤ��ϥ�clone()
    //TimestampedValue(const T& val) : value(val), timestamp(std::chrono::system_clock::now()) {}
};

template<typename T>
class TimestampedRingBuffer {
public:
    TimestampedRingBuffer(size_t size) : buffer(size), head(0), tail(0) {}

    void push(const T& value) {
        std::unique_lock<std::mutex> lock(mutex);
        size_t next_head = (head + 1) % buffer.size();

        if (next_head == tail) {
            // buffer is full, increment tail
            tail = (tail + 1) % buffer.size();
        }

        buffer[head] = TimestampedValue<T>(value);
        head = next_head;
    }

    bool pop(TimestampedValue<T>& value) {
        std::unique_lock<std::mutex> lock(mutex);
        if (tail == head) {
            return false; // buffer is empty
        }
        value = buffer[tail];
        tail = (tail + 1) % buffer.size();
        return true;
    }

    void clear() {
        std::unique_lock<std::mutex> lock(mutex);
        head = tail;
    }

    size_t size() const {
        std::unique_lock<std::mutex> lock(mutex);
        return (head + buffer.size() - tail) % buffer.size();
    }

    bool empty() const {
        std::unique_lock<std::mutex> lock(mutex);
        return head == tail;
    }

private:
    std::vector<TimestampedValue<T>> buffer;
    size_t head;
    size_t tail;
    mutable std::mutex mutex;
};


#endif // __OVERWRITE_GING_BUFFER_H__
