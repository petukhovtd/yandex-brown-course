#ifndef YANDEX_BROWN_COURSE_TRANSPORT_H
#define YANDEX_BROWN_COURSE_TRANSPORT_H

#include "stop.h"
#include "bus.h"

namespace transport
{

class Transport
{
     struct StopInfo
     {
          std::optional< Stop > stop;
          std::set< std::string > buses;
          std::unordered_map< std::string, unsigned int > roadLength;
     };

public:
     void AddStop( const std::string& stopName, Stop stop, const std::vector< std::pair< std::string, unsigned int >>& roadLength );

     void AddBus( std::string name, Bus bus );

     std::optional< Bus::Stats > GetBusStats( const std::string& name );

     const std::set< std::string >* GetStopBusList( const std::string& name ) const;

private:
     Bus::LengthCalculator GetBusLengthCalculator();

private:
     std::unordered_map< std::string, StopInfo > stops_;
     std::unordered_map< std::string, Bus > buses_;
};

}

#endif
