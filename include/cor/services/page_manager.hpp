#ifndef COR_PAGE_MANAGER_HPP
#define COR_PAGE_MANAGER_HPP

#include <mutex>
#include <condition_variable>
#include <string>

#include "cor/system/macros.hpp"

namespace cor {

class Controller;

class PageManager
{

public:
    explicit PageManager(Controller *ctrl, bool first);
    
    ~PageManager();

    void CreateInitialContext();

    idp_t GenerateIdp();
    page_t GeneratePage();
    void UpdatePage(page_t page);

    PageManager() = delete;
    PageManager(PageManager const&) = delete;
    PageManager& operator=(PageManager const&) = delete;
    PageManager(PageManager&&) = delete;
    PageManager& operator=(PageManager&&) = delete;

private:
    Controller *_ctrl;
    bool _is_main_mgr;

    page_t _next_page;
    page_t _current_page;
    page_t _page_size;
    page_t _ids_counter;

    std::mutex _mtx;
    std::condition_variable _cv;
    bool _page_requested;

};

}

#endif
