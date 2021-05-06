#ifndef YANDEX_BROWN_COURSE_BUS_H
#define YANDEX_BROWN_COURSE_BUS_H

#include <string>
#include <functional>
#include <set>

namespace transport
{

class Bus
{
public:
     enum Type
     {
          Linear,
          Circular
     };

     struct LengthInfo
     {
          double length;
          double roadLength;

          LengthInfo& operator+=( const LengthInfo& other )
          {
               length += other.length;
               roadLength += other.roadLength;
               return *this;
          }

          LengthInfo& operator*=( int num )
          {
               length *= num;
               roadLength *= num;
               return *this;
          }
     };

     struct Stats
     {
          size_t stops;
          size_t uniqueStops;
          LengthInfo lengthInfo;
     };

     using LengthCalculator = std::function< LengthInfo( const std::string&, const std::string& ) >;

     explicit Bus( Type type );

     void AddStop( std::string stopName );

     size_t GetStopsOnRoute() const;

     size_t GetUniqueStops() const;

     const std::set< std::string >& GetUniqueStopsList() const;

     LengthInfo GetLength( const LengthCalculator& lengthCalculator );

     Type GetBusType() const;

     const std::vector< std::string >& GetRawStops() const;

private:
     Type type_;
     std::optional< LengthInfo > lengthInfo_;
     std::vector< std::string > stops_;
     std::set< std::string > uniqueStops_;
};

}

#endif
