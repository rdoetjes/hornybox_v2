#include "webservice.hpp"


const std::shared_ptr<http_response> webservice::render(const http_request&) {
  hornio::hornDoubleTap(300, 600);
  return std::shared_ptr<http_response>(new string_response("HONK, HONK!"));
}
