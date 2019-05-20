#ifdef COR_EXECUTOR_HPP

#include "cor/system/system.hpp"
#include "cor/system/pod.hpp"

namespace cor {

template <typename R, typename ... P>
Executor<R(P...)>::Executor() = default;

template <typename R, typename ... P>
Executor<R(P...)>::Executor(idp_t idp, std::function<R(P...)> const& f) :
    _idp{idp},
    _module{},
    _function{},
    _f{f}
{}

template <typename R, typename ... P>
Executor<R(P...)>::Executor(idp_t idp, std::string const& module, std::string const& function) :
    _idp{idp},
    _module{module},
    _function{function},
    _f{}
{}

template <typename R, typename ... P>
Executor<R(P...)>::~Executor() = default;

template <typename R, typename ... P>
Executor<R(P...)>::Executor(Executor<R(P...)>&&) noexcept = default;

template <typename R, typename ... P>
Executor<R(P...)>& Executor<R(P...)>::operator=(Executor<R(P...)>&&) noexcept = default;

template <typename R, typename ... P>
template <typename ... Args>
void Executor<R(P...)>::Run(Args&&... args)
{
    // if function name 
    if (!_function.empty())
        _f = global::pod->LoadFunction<R(P...)>(_module, _function);

    // create task and get future
    std::packaged_task<R(P...)> task;
    if constexpr (std::is_void<R>{}) {
        task = std::move(std::packaged_task<R(P...)>(
            [=](auto&&... args)
            {
                global::pod->InsertActiveResource(std::this_thread::get_id(), _idp);
                _f(std::forward<Args>(args)...);
                global::pod->RemoveActiveResource(std::this_thread::get_id());
            }
        ));
    } else {
        task = std::move(std::packaged_task<R(P...)>(
            [=](auto&&... args)
            {
                global::pod->InsertActiveResource(std::this_thread::get_id(), _idp);
                auto ret = _f(std::forward<Args>(args)...);
                global::pod->RemoveActiveResource(std::this_thread::get_id());
                return ret;
            }
        ));
    }
    _future = task.get_future();
    
    // run task and assign the thread to the current resource
    _thread = std::move(std::thread(std::move(task), std::forward<Args>(args)...));
}

template <typename R, typename ... P>
void Executor<R(P...)>::Wait()
{
//std::cout << "WAIT()" << std::endl;
    _thread.join();
    _future.wait();
//std::cout << "~WAIT()" << std::endl;
}

template <typename R, typename ... P>
R Executor<R(P...)>::Get()
{
    //return _future.get();
    if constexpr (std::is_void<R>{}) {
//std::cout << "Get()" << std::endl;
        _future.get();
//std::cout << "~Get()" << std::endl;
    } else {
//std::cout << "Get()" << std::endl;
        auto res = _future.get();
        return res;
//std::cout << "~Get()" << std::endl;
    }
}

template <typename R, typename ... P>
void Executor<R(P...)>::ChangeIdp(idp_t idp)
{
    global::pod->ChangeActiveResource(_thread.get_id(), idp);
}

template <typename R, typename ... P>  
void Executor<R(P...)>::ResumeIdp()
{
    global::pod->ChangeActiveResource(_thread.get_id(), _idp);
}

template <typename R, typename ... P>  
idp_t Executor<R(P...)>::CurrentIdp() const
{
    return global::pod->GetCurrentActiveResource(_thread.get_id());
}

template <typename R, typename ... P>
idp_t Executor<R(P...)>::OriginalIdp() const
{
    return _idp;
}

}

#endif
