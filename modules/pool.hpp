#ifndef COR_POOL
#define COR_POOL

#include "cor/cor.hpp"

#include <mutex>
#include <condition_variable>

namespace cor
{
    class Pool
    {

    public:
        Pool(int num_agents);
        ~Pool();

        void Dispatch(void (*fct)(void*), void *arg);
        void WaitForIdle();

    protected:
        void PeerAgent();

    private:
        class Barrier
        {
            public:
                Barrier(int count);
                ~Barrier();
                void Wait();
                void WaitForIdle();
                void ReleaseThreads();

            private:
                std::mutex _mtx;
                std::condition_variable _qtask;
                std::condition_variable _qidle;
                int _nthreads;
                int _counter;
                bool _status;
                bool _is_idle;
        };

        void JoinAgents();

        friend void AgentFunc(void *arg);

        int _num_agents;
        Barrier *_barrier;

        std::mutex _mtx;
        void (*_fct)(void*);
        void *_arg;
        bool _shut;

        idp_t _group;
        std::vector<idp_t> _agents;
    };
}

#endif
