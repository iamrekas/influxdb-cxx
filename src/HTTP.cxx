///
/// \author Adam Wegrzynek <adam.wegrzynek@cern.ch>
///

#include "HTTP.h"
#include "InfluxDBException.h"
#include <iostream>
#include <cpr/cpr.h>

namespace influxdb
{
  namespace transports
  {

    HTTP::HTTP(const std::string &url)
    {
      baseUrl = url;
    }

    std::string HTTP::query(const std::string &query)
    {
      std::string mReadUrl = baseUrl;
      auto position = mReadUrl.find("?");
      if (position == std::string::npos)
      {
        throw InfluxDBException("HTTP::initCurl", "Database not specified");
      }
      if (mReadUrl.at(position - 1) != '/')
      {
        mReadUrl.insert(position, "/api/v2/query");
      }
      else
      {
        mReadUrl.insert(position, "api/v2/query");
      }

      cpr::Response r = cpr::Post(
          cpr::Url{mReadUrl},
          cpr::Body{query},
          cpr::Header{
              {"Content-Type", "application/vnd.flux"},
              {"Accept", "application/csv"},
              {"Authorization", std::string("Token " + token)}}
          /*
          cpr::DebugCallback{[&](cpr::DebugCallback::InfoType type, std::string data) {
            // std::cout << "type " << " data: " << data << std::endl;
          }}
          */
      );

      if (r.status_code != 200)
      {
        throw InfluxDBException("HTTP::query", "Status code: " + std::to_string(r.status_code));
      }

      return r.text;
    }

    void HTTP::setAuthToken(const std::string &_token)
    {
      token = _token;
      // headers = curl_slist_append(headers, std::string("Authorization: Token "+token).c_str());
    }

    void HTTP::enableBasicAuth(const std::string & /*auth*/)
    {
      /*
  curl_easy_setopt(curlHandle, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
  curl_easy_setopt(curlHandle, CURLOPT_USERPWD, auth.c_str());
  */
    }

    void HTTP::enableSsl()
    {
      // curl_easy_setopt(curlHandle, CURLOPT_SSL_VERIFYPEER, 0L);
    }

    HTTP::~HTTP()
    {
    }

    void HTTP::send(std::string &&post)
    {

      std::string writeUrl = baseUrl;
      auto position = writeUrl.find("?");
      if (position == std::string::npos)
      {
        throw InfluxDBException("HTTP::initCurl", "Database not specified");
      }
      if (writeUrl.at(position - 1) != '/')
      {
        writeUrl.insert(position, "/api/v2/write");
      }
      else
      {
        writeUrl.insert(position, "api/v2/write");
      }

      cpr::Response r = cpr::Post(cpr::Url{writeUrl},
                                  cpr::Body{std::move(post)},
                                  cpr::Header{
                                      {"Authorization", std::string("Token " + token)},
                                      {"Content-Type", "text/plain"}}
                                  /*
                                  cpr::DebugCallback{[&](cpr::DebugCallback::InfoType type, std::string data) {
                                    // std::cout << "type " << " data: " << data << std::endl;
                                  }}
                                  */
      );

      if (r.status_code < 200 || r.status_code > 206)
      {
        throw InfluxDBException("HTTP::send", "Response code: " + std::to_string(r.status_code));
      }
    }

  } // namespace transports
} // namespace influxdb
