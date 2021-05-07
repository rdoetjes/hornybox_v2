#ifndef WEBSERVICE_HORN_H
#define WEBSERVICE_HORN_H
#include <httpserver.hpp>

#include "hornio.hpp"
#include "global.hpp"

using namespace httpserver;

class webservice : public http_resource {
  private:
    void addHeader(std::shared_ptr<httpserver::http_response> response);
  public:
    const std::shared_ptr<http_response> render_GET(const http_request&);
    const std::shared_ptr<http_response> render_OPTIONS(const http_request& req);
};

#endif
