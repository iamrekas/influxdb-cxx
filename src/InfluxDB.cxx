///
/// \author Adam Wegrzynek <adam.wegrzynek@cern.ch>
///

#include "InfluxDB.h"
#include "InfluxDBException.h"
#include "rapidcsv.h"

#include <iostream>
#include <memory>
#include <string>

#ifdef INFLUXDB_WITH_BOOST
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#endif

namespace influxdb
{

  const std::string WHITESPACE = " \n\r\t\f\v";

  std::string ltrim(const std::string &s)
  {
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
  }

  std::string rtrim(const std::string &s)
  {
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
  }

  std::string trim(const std::string &s)
  {
    return rtrim(ltrim(s));
  }

  InfluxDB::InfluxDB(std::unique_ptr<Transport> transport) : mTransport(std::move(transport))
  {
    mBuffer = {};
    mBuffering = false;
    mBufferSize = 0;
    mGlobalTags = {};
  }

  void InfluxDB::batchOf(const std::size_t size)
  {
    mBufferSize = size;
    mBuffering = true;
  }

  void InfluxDB::flushBuffer()
  {
    if (!mBuffering || mBuffer.empty())
    {
      return;
    }
    std::string stringBuffer{};
    for (const auto &i : mBuffer)
    {
      stringBuffer += i + "\n";
    }
    mBuffer.clear();
    transmit(std::move(stringBuffer));
  }

  void InfluxDB::addGlobalTag(std::string_view key, std::string_view value)
  {
    if (!mGlobalTags.empty())
      mGlobalTags += ",";
    mGlobalTags += key;
    mGlobalTags += "=";
    mGlobalTags += value;
  }

  InfluxDB::~InfluxDB()
  {
    if (mBuffering)
    {
      flushBuffer();
    }
  }

  void InfluxDB::transmit(std::string &&point)
  {
    mTransport->send(std::move(point));
  }

  void InfluxDB::write(Point &&metric)
  {
    if (mBuffering)
    {
      mBuffer.emplace_back(metric.toLineProtocol());
      if (mBuffer.size() >= mBufferSize)
      {
        flushBuffer();
      }
    }
    else
    {
      transmit(metric.toLineProtocol());
    }
  }

  void InfluxDB::setAuthToken(const std::string &token)
  {
    mTransport->setAuthToken(token);
  }

#ifdef INFLUXDB_WITH_BOOST
  std::vector<Point> InfluxDB::query_v1(const std::string &query)
  {
    auto response = mTransport->query(query);
    std::stringstream ss;
    ss << response;
    std::vector<Point> points;
    boost::property_tree::ptree pt;
    boost::property_tree::read_json(ss, pt);

    for (auto &result : pt.get_child("results"))
    {
      auto isResultEmpty = result.second.find("series");
      if (isResultEmpty == result.second.not_found())
        return {};
      for (auto &series : result.second.get_child("series"))
      {
        auto columns = series.second.get_child("columns");

        for (auto &values : series.second.get_child("values"))
        {
          Point point{series.second.get<std::string>("name")};
          auto iColumns = columns.begin();
          auto iValues = values.second.begin();
          for (; iColumns != columns.end() && iValues != values.second.end(); iColumns++, iValues++)
          {
            auto value = iValues->second.get_value<std::string>();
            auto column = iColumns->second.get_value<std::string>();
            if (!column.compare("time"))
            {
              std::tm tm = {};
              std::stringstream ss;
              ss << value;
              ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
              point.setTimestamp(std::chrono::system_clock::from_time_t(std::mktime(&tm)));
              continue;
            }
            // cast all values to double, if strings add to tags
            try
            {
              point.addField(column, boost::lexical_cast<double>(value));
            }
            catch (...)
            {
              point.addTag(column, value);
            }
          }
          points.push_back(std::move(point));
        }
      }
    }
    return points;
  }
  std::vector<Point> InfluxDB::query(const std::string &query)
  {
    auto response = mTransport->query(query);
    std::vector<Point> points;

    if (trim(response).length() == 0)
    {
      return points;
    }

    std::stringstream sstream(response);
    rapidcsv::Document doc(sstream, rapidcsv::LabelParams(0, 0));

    const size_t columnCount = doc.GetColumnCount();
    const size_t rowCount = doc.GetRowCount();
    size_t i, o;
    for (i = 0; i < rowCount - 1; i++)
    {
      Point point{doc.GetCell<std::string>("_measurement", i)};
      std::tm tm = {};
      std::stringstream ss;
      ss << doc.GetCell<std::string>("_time", i);
      ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
      point.setTimestamp(std::chrono::system_clock::from_time_t(std::mktime(&tm)));

      try
      {
        point.addField(doc.GetCell<std::string>("_field", i), boost::lexical_cast<double>(doc.GetCell<std::string>("_value", i)));
      }
      catch (...)
      {
      }

      for (o = doc.GetColumnIdx("_measurement") + 1; o < columnCount; ++o)
      {
        point.addTag(doc.GetColumnName(o), doc.GetCell<std::string>(o, i));
      }

      points.push_back(std::move(point));
    }
    return points;
  }
#else
  std::vector<Point> InfluxDB::query_v1(const std::string & /*query*/)
  {
    throw InfluxDBException("InfluxDB::query", "Boost is required");
  }
  std::vector<Point> InfluxDB::query(const std::string & /*query*/)
  {
    throw InfluxDBException("InfluxDB::query", "Boost is required");
  }
#endif

} // namespace influxdb
