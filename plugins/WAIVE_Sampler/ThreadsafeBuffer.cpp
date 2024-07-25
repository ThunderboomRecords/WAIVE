#include <vector>
#include <mutex>
#include <stdexcept>

/*
    Threadsafe Buffer
    Stores (multi-channel) audio data as a float, with threadsafe operations

*/
class ThreadsafeBuffer
{
private:
    std::vector<std::vector<float>> buffer;
    std::mutex mtx;
    // unsigned int channels;

public:
    // Add a float to the vector
    void addFloat(float value)
    {
        std::lock_guard<std::mutex> lock(mtx);
        buffer.emplace_back(std::vector<float>{value});
    }

    // Add a vector of floats to the vector
    void addVector(const std::vector<float> &values)
    {
        std::lock_guard<std::mutex> lock(mtx);
        buffer.push_back(values);
    }

    // Get a float from the vector
    float getFloat(size_t index)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (index >= buffer.size() || buffer[index].size() != 1)
        {
            throw std::out_of_range("Index out of range or not a float");
        }
        return buffer[index][0];
    }

    // Get a vector of floats from the vector
    std::vector<float> getVector(size_t index)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (index >= buffer.size() || buffer[index].size() <= 1)
        {
            throw std::out_of_range("Index out of range or not a vector");
        }
        return buffer[index];
    }

    // Remove a float from the vector
    void removeFloat(size_t index)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (index >= buffer.size() || buffer[index].size() != 1)
        {
            throw std::out_of_range("Index out of range or not a float");
        }
        buffer.erase(buffer.begin() + index);
    }

    // Remove a vector of floats from the vector
    void removeVector(size_t index)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (index >= buffer.size() || buffer[index].size() <= 1)
        {
            throw std::out_of_range("Index out of range or not a vector");
        }
        buffer.erase(buffer.begin() + index);
    }

    // Clear the vector
    void clear()
    {
        std::lock_guard<std::mutex> lock(mtx);
        buffer.clear();
    }

    // Get number of channels
    std::size_t getNumberChannels() const
    {
        return buffer.size();
    }
};