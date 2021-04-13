#include "transport.h"

namespace transport
{

void Transport::AddStop( const std::string& stopName, Stop stop, const std::vector< std::pair< std::string, unsigned int >>& roadLength )
{
     StopInfo& stopInfo = stops_[ stopName ];
     stopInfo.stop = stop;
     for( const auto& [ otherStopName, length ]: roadLength )
     {
          stopInfo.roadLength[ otherStopName ] = length;
          StopInfo& otherStop = stops_[ otherStopName ];
          if( otherStop.roadLength.count( stopName ) == 0 )
          {
               stops_[ otherStopName ].roadLength[ stopName ] = length;
          }
     }
}

void Transport::AddBus( std::string name, Bus bus )
{
     for( const auto& stop: bus.GetUniqueStopsList() )
     {
          stops_[ stop ].buses.insert( name );
     }
     buses_.insert( { std::move( name ), std::move( bus ) } );
}

std::optional< Bus::Stats > Transport::GetBusStats( const std::string& name )
{
     auto it = buses_.find( name );
     if( it == buses_.end() )
     {
          return std::nullopt;
     }
     Bus& bus = it->second;
     return Bus::Stats { bus.GetStopsOnRoute(), bus.GetUniqueStops(), bus.GetLength( GetBusLengthCalculator() ) };
}

const std::set< std::string >* Transport::GetStopBusList( const std::string& name ) const
{
     auto it = stops_.find( name );
     if( it == stops_.end() )
     {
          return nullptr;
     }
     return &it->second.buses;
}

Bus::LengthCalculator Transport::GetBusLengthCalculator()
{
     return [ = ]( const std::string& s1, const std::string& s2 )->Bus::LengthInfo
     {
          Bus::LengthInfo lengthInfo{};
          lengthInfo.length = CalculateLength( stops_.at( s1 ).stop.value(), stops_.at( s2 ).stop.value() );
          lengthInfo.roadLength = stops_.at( s1 ).roadLength[ s2 ];
          return lengthInfo;
     };
}

}
