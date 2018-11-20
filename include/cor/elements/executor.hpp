#ifndef COR_EXECUTOR_HPP
#define COR_EXECUTOR_HPP

#include <thread>
#include <future>
#include <atomic>
#include <functional>

#include "cor/system/macros.hpp"
#include "cor/system/system.hpp"

#include "cereal/types/string.hpp"

namespace cor {

template <typename> class Executor;

template <typename R, typename ... P>
class Executor<R(P...)>
{

friend class cereal::access;

public:
    ~Executor();

    Executor(const Executor&) = delete;
    Executor& operator=(const Executor&) = delete;

    Executor(Executor&&) noexcept;
    Executor& operator=(Executor&&) noexcept;

    template <typename ... Args>
    void Run(Args&&... args);

    void Wait();

    R Get();

    void ChangeIdp(idp_t idp);
    void ResumeIdp();

    idp_t CurrentIdp() const;
    idp_t OriginalIdp() const;

protected:
    Executor();
    //explicit Executor(idp_t idp, std::string const& function);
    explicit Executor(idp_t idp, std::function<R(P...)> const& f);
    explicit Executor(idp_t idp, std::string const& module, std::string const& function);

private:
    template <typename Archive>
    void serialize(Archive& ar) 
    {
        ar(_idp, _function);
    }

    idp_t _idp;
    std::string _module;
    std::string _function;
    std::function<R(P...)> _f;

    std::future<R> _future;
    std::thread _thread;

};

}

#include "cor/elements/executor.tpp"

#endif
