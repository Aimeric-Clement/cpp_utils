#ifndef UTL_SCOPED_RESOURCE_WRAPPER_HPP
#define UTL_SCOPED_RESOURCE_WRAPPER_HPP

#include <functional>
namespace utl
{
    /**
     * @brief Invoke a given clean-up function upon destruction
     * @details A simple wrapper to enforce RAII upon a unmanaged ressource
     */
    class scoped_resource_wrapper
    {
    private:
        std::function<void()> m_on_close;
        // prevent any leaking through heap allocation and enforce RAII
        static void *operator new     (size_t) = delete;
        static void *operator new[]   (size_t) = delete;
        static void  operator delete  (void*)  = delete;
        static void  operator delete[](void*)  = delete;
        scoped_resource_wrapper(scoped_resource_wrapper const &) = delete;
     public:
        scoped_resource_wrapper() {}
        template <typename F, typename... Ts>
        scoped_resource_wrapper(F&& functor, Ts&&... args): m_on_close(std::bind(std::forward<F>(functor), std::forward<Ts>(args)...)) {}        
        scoped_resource_wrapper(std::function<void()>&& on_close) : m_on_close(std::move(on_close)) {}
         ~scoped_resource_wrapper()
        {
            if(m_on_close != nullptr)
                std::invoke(m_on_close);
        }
    };
}
#endif // UTL_SCOPED_RESOURCE_WRAPPER_HPP