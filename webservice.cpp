#include "webservice.hpp"


const std::shared_ptr<http_response> webservice::render(const http_request& req) {
  hornio::hornDoubleTap( stoi(req.get_arg("msShort")), stoi(req.get_arg("msLong")) );
  return std::shared_ptr<http_response>(new string_response("HONK, HONK!"));
}
