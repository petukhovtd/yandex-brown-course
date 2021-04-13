#ifndef YANDEX_BROWN_COURSE_REQUEST_H
#define YANDEX_BROWN_COURSE_REQUEST_H

#include "transport.h"

namespace Json
{

class Node;

}


namespace transport
{

struct Request
{
     enum Type
     {
          Add,
          Get
     };

     explicit Request( Type t )
               : type( t )
     {}

     virtual ~Request() = default;

     virtual void ParsingFrom( const Json::Node& request ) = 0;

     Type type;
};

using RequestPtr = std::unique_ptr< Request >;

struct AddRequest
          : public Request
{
     AddRequest()
               : Request( Request::Add )
     {}

     virtual void Process( Transport& transport ) const = 0;
};

struct GetRequest
          : public Request
{
     GetRequest()
               : Request( Request::Get )
     {}

     virtual Json::Node Process( Transport& transport ) const = 0;

     int id = 0;
};

}

#endif
