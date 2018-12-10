#ifndef COR_SMUTEX_HPP
#define COR_SMUTEX_HPP

#include "cereal/access.hpp"

#include "cor/system/macros.hpp"

namespace cor {

class SMutex
{

friend class cereal::access;

public:
    ~SMutex();

    SMutex(const SMutex&) = delete;
    SMutex& operator=(const SMutex&) = delete;

    SMutex(SMutex&&) noexcept;
    SMutex& operator=(SMutex&&) noexcept;

    void Acquire();
    void Release();

protected:
    SMutex();
    explicit SMutex(idp_t idp);

private:
    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(_idp);
    }

    idp_t _idp;

};

}

#endif
