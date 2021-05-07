#include "webservice.hpp"
#include "global.hpp"


const std::shared_ptr<http_response> webservice::render(const http_request& req) {
  if (req.get_path() == "/honk"){

    if (req.get_arg("msShort") == "" || req.get_arg("msLong") == "")
      return std::shared_ptr<http_response>(new string_response("msShort and/or msLong argument missing!"));

    hornio::hornDoubleTap( stoi(req.get_arg("msShort")), stoi(req.get_arg("msLong")) );
    return std::shared_ptr<http_response>(new string_response("HONK, HONK!"));
  }

  if (req.get_path() == "/facedetect"){
    if( req.get_arg("toggle") == "true" )
      FACE_DETECT ^= 1;
    return std::shared_ptr<http_response>(new string_response(to_string(FACE_DETECT)));
  }

  return std::shared_ptr<http_response>(new string_response(req.get_path()));
}

