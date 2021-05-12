///
/// \author Adam Wegrzynek
///

#ifndef INFLUXDATA_TRANSPORTS_HTTP_H
#define INFLUXDATA_TRANSPORTS_HTTP_H

#include "Transport.h"
#include <curl/curl.h>
#include <memory>
#include <string>

namespace influxdb
{
  namespace transports
  {

    /// \brief HTTP transport
    class HTTP : public Transport
    {
    public:
      /// Constructor
      HTTP(const std::string &url);

      /// Default destructor
      ~HTTP();

      /// Sends point via HTTP POST
      ///  \throw InfluxDBException	when CURL fails on POSTing or response code != 200
      void send(std::string &&post) override;

      void setAuthToken(const std::string &_token) override;

      /// Queries database
      /// \throw InfluxDBException	when CURL GET fails
      std::string query(const std::string &query) override;

      /// Enable Basic Auth
      /// \param auth <username>:<password>
      void enableBasicAuth(const std::string &auth);

      /// Enable SSL
      void enableSsl();

    private:
      std::string baseUrl;

      std::string token;
    };

  } // namespace transports
} // namespace influxdb

#endif // INFLUXDATA_TRANSPORTS_HTTP_H
