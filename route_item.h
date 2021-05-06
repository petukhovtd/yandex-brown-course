#ifndef YANDEX_BROWN_COURSE_ROUTEITEM_H
#define YANDEX_BROWN_COURSE_ROUTEITEM_H

#include <unordered_map>
#include <memory>
#include <utility>
#include "json.h"

namespace transport
{

class RouteItem
{

public:
     enum Type
               : int
     {
          Wait,
          Bus
     };

     explicit RouteItem( Type type, double time )
               : time_( time )
               , type_( type )
     {}

     virtual ~RouteItem() = default;

     virtual Json::Node GetItemInfo() const = 0;

protected:
     double time_;

private:
     Type type_;
};

using RouteItemPtr = std::unique_ptr< RouteItem >;

class WaitItem
          : public RouteItem
{
public:
     ~WaitItem() override = default;

     WaitItem( std::string stopName, double waitTime )
               : RouteItem( Wait, waitTime )
               , stopName_( std::move( stopName ) )
     {}

     Json::Node GetItemInfo() const override
     {
          std::map< std::string, Json::Node > result;
          result[ "type" ] = std::string( "Wait");
          result[ "stop_name" ] = stopName_;
          result[ "time" ] = time_;
          return Json::Node( result );
     }

private:
     std::string stopName_;
};

class BusItem
          : public RouteItem
{
public:
     ~BusItem() override = default;

     BusItem( std::string busName, int spanCount, double time )
               : RouteItem( Bus, time )
               , busName_( std::move( busName ) )
               , spanCount_( spanCount )
     {}

     Json::Node GetItemInfo() const override
     {
          std::map< std::string, Json::Node > result;
          result[ "type" ] = std::string( "Bus" );
          result[ "bus" ] = busName_;
          result[ "span_count" ] = spanCount_;
          result[ "time" ] = time_;
          return Json::Node( result );
     }

private:
     std::string busName_;
     int spanCount_;
};

}

#endif //YANDEX_BROWN_COURSE_ROUTEITEM_H
