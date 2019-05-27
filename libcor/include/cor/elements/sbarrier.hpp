#ifndef COR_SBARRIER_HPP
#define COR_SBARRIER_HPP

#include "cereal/access.hpp"

#include "cor/system/macros.hpp"

namespace cor {

class SBarrier
{

friend class cereal::access;

public:
    ~SBarrier();

    SBarrier(const SBarrier&) = delete;
    SBarrier& operator=(const SBarrier&) = delete;

    SBarrier(SBarrier&&) noexcept;
    SBarrier& operator=(SBarrier&&) noexcept;

    void Synchronize();

protected:
    SBarrier();
    explicit SBarrier(idp_t idp, idp_t clos);

private:
    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(_idp, _clos);
    }

    idp_t _idp;
    idp_t _clos;

};

}

#endif
