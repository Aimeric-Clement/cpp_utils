#ifndef ASYNC_UTILS_AC_HPP
#define ASYNC_UTILS_AC_HPP

#include <thread>
#include <condition_variable>
#include <functional>
// #include <queue>
// #include <type_traits>

/**
 * @brief contains a number of runtime tools, meta-functions helpers and useful container types
 * 
 * namely,
 *  timeout:
 *      Triggers a function after some delay, starting from the handler instance construction.
 *      The call may be canceled on demand or if the handler runs out of scope.
 * 
 *   jthread:
 *      A simple joining-thread implementation that also always checks if the thread is joinable.
 */
namespace utl
{

    /**
     * @brief automatically joining thread
     * @details is a thread wrapper that automatically join on destruction (if required) and prevents detaching the thread
     * @warning must ensure that the underlying function terminates, and this before the jthread's destructor is invoked,
     * otherwise the destructor will encounter the same termination problem and potentially stall forever.
     * 
     * (before c++20, which has a std::jthread with similar features)
     */
    class jthread
    {
    private:
        std::thread m_thread;
    
    public:
        jthread() noexcept = default;
        jthread(const jthread&) noexcept = delete;
        jthread(jthread&&) noexcept = default;
        jthread(std::thread&& other) noexcept : m_thread(std::move(other)) {};

        template <typename F, typename... Args>
        jthread(F&& func, Args&&... args) : m_thread( std::forward<F>(func), std::forward<Args>(args)...) {}
        jthread& operator=(jthread&& other) noexcept = default;

        /// @brief join the thread early, if it has to
        void join() noexcept
        { if(m_thread.joinable()) m_thread.join(); }

        ~jthread()
        { join(); }
    };

    /**
     * @brief run a task after some delay
     * @warning if it is copy-initialized, the task will run immediately when discarding the original object.
     */
    template <typename Callable, typename DelayT, typename... Args>
    class timeout
    {
    static_assert(std::is_invocable_v<Callable, std::decay_t<Args>...>, "First template argument must be a callable type with the arguments");

    public:
        [[nodiscard]] timeout(const Callable task, const DelayT delay, Args... args)
        {
            m_worker = std::thread(
            [this, delay = std::move(delay)](const Callable task, Args... args)
            {
                std::mutex mtx;
                std::unique_lock unique_thread_lock(mtx);
                const std::cv_status why = m_thread_unlocking_condition.wait_for(unique_thread_lock, delay);
                if (why == std::cv_status::timeout)
                    // runs the task only if the timeout has been reached
                    std::invoke(std::forward<const Callable>(task), std::forward<Args>(args)...);
            }
            , std::forward<const Callable>(task)
            , std::forward<Args>(args)...);
        }

        ~timeout()
        { cancel(); } // tries to interrupt the timeout if it is destroyed

        void cancel()
        {
            m_thread_unlocking_condition.notify_all();
            m_worker.join();
        }
        
    private:
        std::condition_variable m_thread_unlocking_condition;
        std::thread m_worker;
    };

    // template <typename T>
    // class locking_bipartite_queue
    // {
    // static_assert(!std::is_trivially_destructible_v<T>, "The value-type must be trivially destructible");

    // private:
    //     std::condition_variable             m_wait_sending_cond;
    //     std::mutex                          m_swap_queues_mtx;

    //     std::mutex                          m_outgoing_queue_mtx;
    //     std::queue                          m_incoming_queue;
        
    //     std::mutex                          m_outgoing_queue_mtx;
    //     std::queue                          m_outgoing_queue;
        
    //     std::function<void(std::queue&)>    m_consume;
    //     std::thread                         m_worker;
    //     std::atomic_bool                    m_stop_worker = false;

    // public:
    //     /**
    //      * @arg consume_values is a function that must consume all values of the given queue; before any remains are automatically discared
    //      */
    //     locking_bipartite_queue(const std::function<void(std::queue&)> consume_values)
    //     : m_consume (consume_values)
    //     {}

    //     //TODO: emplace_back

    //     void push_back(const T val)
    //     {
    //         std::scoped_lock lock(m_incoming_queue_mtx, m_swap_queues_mtx);
    //         m_outgoing_queue.pushback(val);
    //         m_wait_sending_cond.notify_all();
    //     }
    // };
}

#endif