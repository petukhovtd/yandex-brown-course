#include "transport.h"
#include "router.h"

#include <utility>

namespace transport
{

void Transport::AddStop( const std::string& stopName, Stop stop, const std::vector< std::pair< std::string, unsigned int >>& roadLength )
{
     routeContext_.Reset();
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
     routeContext_.Reset();
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

std::variant< Transport::RouteResult, std::string > Transport::GetRoute( const std::string& from, const std::string& to ) const
{
     if( !routeContext_.HaveRouter() )
     {
          InitRouterContext();
     }

     Graph::VertexId fromId = routeContext_.vertexNameToId.at( from ).first;
     Graph::VertexId toId = routeContext_.vertexNameToId.at( to ).first;
     auto result = routeContext_.router->BuildRoute( fromId, toId );
     if( !result.has_value() )
     {
          return "not found";
     }
     const Graph::Router< Widget >::RouteInfo& routeInfo = result.value();
     RouteResult routeResult;
     routeResult.time = routeInfo.weight;
     routeResult.items.reserve( routeInfo.edge_count );
     for( size_t edgeIndex = 0; edgeIndex < routeInfo.edge_count; ++edgeIndex )
     {
          Graph::EdgeId edgeId = routeContext_.router->GetRouteEdge( result.value().id, edgeIndex );
          const EdgeWidget& edgeWidget = routeContext_.edges.at( edgeId );
          RouteItemPtr item;
          if( edgeWidget.waitEdge )
          {
               const std::string& stopName = routeContext_.vertexIdToName.at( edgeWidget.from ).first;
               item = std::make_unique< WaitItem >( stopName, edgeWidget.weight );
          }
          else
          {
               item = std::make_unique< BusItem >( edgeWidget.busName, edgeWidget.spanCount, edgeWidget.weight );
          }
          routeResult.items.push_back( std::move( item ) );
     }
     return routeResult;
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

void Transport::SetSettings( Settings settings )
{
     settings_ = settings;
}

void Transport::InitRouterContext() const
{
     routeContext_.graph = std::make_unique< Graph::DirectedWeightedGraph< Widget > >( stops_.size() * 2 );
     AddStopsToRouteContext();
     AddBusesToRouteContext();
     routeContext_.router = std::make_unique< Graph::Router< Widget > >( *routeContext_.graph );
}

void Transport::AddStopsToRouteContext() const
{
     Graph::VertexId id = 0;
     for( const auto& [ stop, stopInfo ]: stops_ )
     {
          Graph::VertexId inId = id++;
          Graph::VertexId outId = id++;
          routeContext_.vertexIdToName[ inId ] = { stop, In };
          routeContext_.vertexIdToName[ outId ] = { stop, Out };
          routeContext_.vertexNameToId[ stop ] = { inId, outId };
          Graph::EdgeId edgeId = routeContext_.graph->AddEdge( { inId, outId, settings_.busWaitTime });
          routeContext_.edges.insert({ edgeId, EdgeWidget( settings_.busWaitTime, inId, outId ) } );
     }
}

static std::vector< std::string > ConvertBusStops( const Bus& bus )
{
     switch( bus.GetBusType() )
     {
          case Bus::Linear:
          {
               std::vector< std::string > stops = bus.GetRawStops();
               // a - b - c (3)
               // a - b - c - b - a (5)
               // Длина обратного маршрута меньше на одну остановку
               stops.resize( stops.size() * 2 - 1 );
               auto it = stops.begin();
               std::advance( it, bus.GetRawStops().size() );
               std::copy( std::next( bus.GetRawStops().rbegin() ), bus.GetRawStops().rend(), it );
               return stops;
          }
          case Bus::Circular:
               return bus.GetRawStops();
          default:
               throw std::runtime_error("");
     }
}

void Transport::AddBusesToRouteContext() const
{
     for( const auto& [ busName, bus ]: buses_ )
     {
          const std::vector< std::string > busStops = ConvertBusStops( bus );
          if( busStops.empty() || busStops.size() == 1 )
          {
               continue;
          }
          // A - B - C - B - A
          // A > B
          // A >   > C
          // A >   >   > B
          // A >   >   >   > A

          for( size_t in = 0; in < ( busStops.size() - 1 ); ++in )
          {
               double forwardTime = 0;

               for( size_t out = ( in + 1 ); out < busStops.size(); ++out )
               {
                    const std::string& fromStop = busStops[ in ];
                    const std::string& toStop = busStops[ out ];
                    Graph::VertexId fromOutId = routeContext_.vertexNameToId[ fromStop ].second;
                    Graph::VertexId toInId = routeContext_.vertexNameToId[ toStop ].first;

                    const std::string& prevStop = busStops[ ( out - 1 ) ];

                    const StopInfo& prevStopInfo = stops_.at( prevStop );
                    double roadLength = prevStopInfo.roadLength.at( toStop );
                    forwardTime += roadLength / settings_.busVelocity;

                    Graph::EdgeId edgeId = routeContext_.graph->AddEdge( { fromOutId, toInId,  forwardTime });
                    routeContext_.edges.insert( { edgeId, EdgeWidget( forwardTime, fromOutId, toInId, busName, ( out - in ) ) } );
               }
          }
     }
}

}
