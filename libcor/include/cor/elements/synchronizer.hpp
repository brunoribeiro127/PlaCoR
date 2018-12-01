#ifndef COR_SYNCHRONIZER_HPP
#define COR_SYNCHRONIZER_HPP

#include "cereal/access.hpp"

#include "cor/system/macros.hpp"

namespace cor {

class Synchronizer
{

friend class cereal::access;

public:
    ~Synchronizer();

    Synchronizer(const Synchronizer&) = delete;
    Synchronizer& operator=(const Synchronizer&) = delete;

    Synchronizer(Synchronizer&&) noexcept;
    Synchronizer& operator=(Synchronizer&&) noexcept;

    void AcquireWrite() const;
    void ReleaseWrite() const;

    void AcquireRead() const;
    void ReleaseRead() const;

protected:
    Synchronizer();
    explicit Synchronizer(idp_t idp);

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
