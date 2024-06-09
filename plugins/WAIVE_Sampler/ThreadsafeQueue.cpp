template <typename T>
class ThreadsafeQueue
{
    std::queue<T> queue_;
    mutable std::mutex mutex_;

public:
    ThreadsafeQueue() = default;
    ThreadsafeQueue(const ThreadsafeQueue<T> &) = delete;
    ThreadsafeQueue &operator=(const ThreadsafeQueue<T> &) = delete;

    ThreadsafeQueue(ThreadsafeQueue<T> &&other)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_ = std::move(other.queue_);
    }

    ~ThreadsafeQueue() noexcept = default;

    unsigned long size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    std::optional<T> pop()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty())
        {
            return {};
        }
        T tmp = queue_.front();
        queue_.pop();
        return tmp;
    }

    void push(const T &item)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(item);
    }
};