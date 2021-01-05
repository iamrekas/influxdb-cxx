#define BOOST_TEST_MODULE Test InfluxDB HTTP
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "../include/InfluxDBFactory.h"
#include "../src/InfluxDBException.h"

namespace influxdb {
namespace test {

BOOST_AUTO_TEST_CASE(write1)
{
  auto influxdb = influxdb::InfluxDBFactory::Get("http://influxdb:9999?db=test");
  influxdb->write(Point{"test"}
    .addField("value", 10)
    .addTag("host", "influxdb")
 );

  influxdb->write(Point{"test"}
    .addField("value", 20)
    .addTag("host", "influxdb")
  );

  influxdb->write(Point{"test"}
    .addField("value", 200LL)
    .addTag("host", "influxdb"));

    influxdb->write(Point{"string"}
    .addField("value", "influxdb-cxx")
    .addTag("host", "influxdb"));
}

BOOST_AUTO_TEST_CASE(writeWrongHost)
{
  auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost:9999?db=test");
  BOOST_CHECK_THROW(influxdb->write(Point{"test"}.addField("value", 10)), InfluxDBException);
}

} // namespace test
} // namespace influxdb
