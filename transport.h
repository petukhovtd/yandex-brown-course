#ifndef YANDEX_BROWN_COURSE_TRANSPORT_H
#define YANDEX_BROWN_COURSE_TRANSPORT_H

#include "stop.h"
#include "bus.h"
#include "graph.h"
#include "router.h"
#include "route_item.h"
#include <variant>

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

     typedef double Widget;

     struct EdgeWidget
     {
          double weight;
          bool waitEdge;
          std::string busName;
          size_t spanCount;
          Graph::VertexId from;
          Graph::VertexId to;

          EdgeWidget( double w, Graph::VertexId fromId, Graph::VertexId toId )
                    : weight( w )
                    , waitEdge( true )
                    , busName()
                    , spanCount( 0 )
                    , from( fromId )
                    , to( toId )
          {}

          EdgeWidget( double w, Graph::VertexId fromId, Graph::VertexId toId, std::string bus, size_t span )
                    : weight( w )
                    , waitEdge( false )
                    , busName( std::move( bus ) )
                    , spanCount( span )
                    , from( fromId )
                    , to( toId )
          {}
     };

     enum StopType
     {
          In,
          Out
     };

     struct RouteContext
     {
          std::unique_ptr< Graph::Router< Widget > > router;
          std::unique_ptr< Graph::DirectedWeightedGraph< Widget > > graph;
          std::unordered_map< Graph::VertexId, std::pair< std::string, StopType > > vertexIdToName;
          std::unordered_map< std::string, std::pair< Graph::VertexId, Graph::VertexId > > vertexNameToId;
          std::unordered_map< Graph::EdgeId, EdgeWidget > edges;

          bool HaveRouter() const
          {
               return !!router && !!graph;
          }

          void Reset()
          {
               graph.reset();
               router.reset();
          }
     };

public:
     struct Settings
     {
          double busWaitTime;
          double busVelocity;
     };

     struct RouteResult
     {
          double time;
          std::vector< RouteItemPtr > items;
     };

     void AddStop( const std::string& stopName, Stop stop,
                   const std::vector< std::pair< std::string, unsigned int >>& roadLength );

     void AddBus( std::string name, Bus bus );

     std::optional< Bus::Stats > GetBusStats( const std::string& name );

     const std::set< std::string >* GetStopBusList( const std::string& name ) const;

     std::variant< RouteResult, std::string > GetRoute( const std::string& from, const std::string& to ) const;

     void SetSettings( Settings settings );

private:
     Bus::LengthCalculator GetBusLengthCalculator();

     void InitRouterContext() const;

     void AddStopsToRouteContext() const;

     void AddBusesToRouteContext() const;

private:
     std::unordered_map< std::string, StopInfo > stops_;
     std::unordered_map< std::string, Bus > buses_;
     Settings settings_;
     mutable RouteContext routeContext_;
};

}

#endif
