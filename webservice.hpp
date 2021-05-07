#ifndef WEBSERVICE_HORN_H
#define WEBSERVICE_HORN_H
#include <httpserver.hpp>

#include "hornio.hpp"
#include "global.hpp"

using namespace httpserver;

class webservice : public http_resource {
  public:
    const std::shared_ptr<http_response> render(const http_request&);
};

#endif
