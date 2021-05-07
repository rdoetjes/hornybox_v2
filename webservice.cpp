#include "webservice.hpp"
#include "global.hpp"


void webservice::addHeader(std::shared_ptr<httpserver::http_response> response){
  response->with_header("Access-Control-Allow-Headers", "*");
  response->with_header("Access-Control-Allow-Origin", "*");
}

const std::shared_ptr<http_response> webservice::render_GET(const http_request& req) {
  
  if (req.get_path() == "/honk"){
    if (req.get_arg("msShort") == "" || req.get_arg("msLong") == ""){
      std::shared_ptr<httpserver::http_response> response = std::shared_ptr<http_response>(new string_response("msShort and/or msLong argument missing!"));
      addHeader(response);
      return response;
    }

    hornio::hornDoubleTap( stoi(req.get_arg("msShort")), stoi(req.get_arg("msLong")) );
    std::shared_ptr<httpserver::http_response> response = std::shared_ptr<http_response>(new string_response("HONK, HONK!"));
    addHeader(response);
    return response;
  }

  if (req.get_path() == "/facedetect"){

    if( req.get_arg("toggle") == "true" )
      FACE_DETECT ^= 1;

    std::shared_ptr<httpserver::http_response> response = std::shared_ptr<http_response>(new string_response(to_string(FACE_DETECT)));
    addHeader(response);
    return response;
  }

  return std::shared_ptr<http_response>(new string_response(req.get_path()));
}


//THIS IS TO SATISFY THE OPTIONS (CORS) REQUEST
const std::shared_ptr<http_response> webservice::render_OPTIONS(const http_request& req){
   std::shared_ptr<httpserver::http_response> response = std::shared_ptr<http_response>(new string_response(req.get_path()));
   addHeader(response);
   return response;
}
