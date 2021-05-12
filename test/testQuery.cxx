#define BOOST_TEST_MODULE Test InfluxDB Query
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "../include/InfluxDBFactory.h"
#include "../src/InfluxDBException.h"
#include <iostream>
namespace influxdb
{
  namespace test
  {

    static auto initInflux()
    {
      auto influxdb = influxdb::InfluxDBFactory::Get("http://HOST:8086/?org=ORG&bucket=BUCKET&precision=ns");
      influxdb->setAuthToken("TOKEN");
      return influxdb;
    }

#define writeData(influxdb)                                                        \
  influxdb->write(Point{"test"}.addField("value", 10).addTag("host", "influxdb")); \
  influxdb->write(Point{"test"}.addField("value", 20).addTag("host", "influxdb")); \
  influxdb->write(Point{"test"}.addField("value", 200LL).addTag("host", "influxdb"));

    BOOST_AUTO_TEST_CASE(query1)
    {
      auto influxdb = initInflux();
      writeData(influxdb) auto points = influxdb->query(std::string("from(bucket:\"test\") \n\
        |> range(start: -1h) \n\
        |> filter(fn: (r) => r[\"_measurement\"] == \"test\") \n\
        |> filter(fn: (r) => r[\"host\"] == \"influxdb\") \n\
        |> limit(n:3, offset: 0)"));
      BOOST_CHECK_EQUAL(points.size(), 3);
      BOOST_CHECK_EQUAL(points[0].getName(), "test");
      BOOST_CHECK_EQUAL(points[1].getName(), "test");
      BOOST_CHECK_EQUAL(points[2].getName(), "test");
      BOOST_CHECK_EQUAL(points[0].getFields(), "value=10.00");
      BOOST_CHECK_EQUAL(points[1].getFields(), "value=20.00");
      BOOST_CHECK_EQUAL(points[2].getFields(), "value=200.00");
      BOOST_CHECK_EQUAL(points[0].getTags(), "host=influxdb");
      BOOST_CHECK_EQUAL(points[1].getTags(), "host=influxdb");
      BOOST_CHECK_EQUAL(points[2].getTags(), "host=influxdb");
    }

    /*
    BOOST_AUTO_TEST_CASE(timeStampVerify)
    {
      double timeZone = 3600; //+1h

      auto influxdb = initInflux();
      Point point = Point{"timestampCheck"}.addField("value", 10);
      auto timestamp = point.getTimestamp();
      influxdb->write(std::move(point));

      auto points = influxdb->query(std::string("from(bucket:\"test\") \n\
        |> range(start: -1h) \n\
        |> filter(fn: (r) => r[\"_measurement\"] == \"timestampCheck\") \n\
        |> limit(n:1, offset: 0)"));

      std::chrono::duration<double> diff = timestamp - points[0].getTimestamp();
      double diffZone = diff.count() - timeZone;
      BOOST_CHECK(diffZone < 1); // 1s
    }
*/
    BOOST_AUTO_TEST_CASE(queryPerformance)
    {
      auto influxdb = initInflux();
      auto t1 = std::chrono::high_resolution_clock::now();
      auto points = influxdb->query(std::string("from(bucket:\"test\") \n\
        |> range(start: -1h) \n\
        |> filter(fn: (r) => r[\"_measurement\"] == \"test\") \n\
        |> filter(fn: (r) => r[\"host\"] == \"influxdb\")"));
      BOOST_CHECK(points.size() >= 3);
      auto t2 = std::chrono::high_resolution_clock::now();
      double duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
      BOOST_CHECK(duration < 20000);
    }

    BOOST_AUTO_TEST_CASE(failedQuery1)
    {
      auto influxdb = initInflux();
      auto points = influxdb->query(std::string("from(bucket:\"test\") \n\
        |> range(start: -1h) \n\
        |> filter(fn: (r) => r[\"_measurement\"] == \"test1\") \n\
        |> filter(fn: (r) => r[\"host\"] == \"influxdb\")"));
      BOOST_CHECK_EQUAL(points.size(), 0);
    }

    BOOST_AUTO_TEST_CASE(failedQuery2)
    {
      auto influxdb = initInflux();
      BOOST_CHECK_THROW(influxdb->query(std::string("from(bucket:\"test\") \n\
        |> range(start: -1h) \n\
        |> filter(fn: (r) => r[\"_measurement\"] *== \"test1\") \n\
        |> filter(fn: (r) => r[\"host\"] == \"influxdb\")")),
                        InfluxDBException);
    }

  } // namespace test
} // namespace influxdb
