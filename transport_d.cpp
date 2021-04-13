#include <iostream>
#include <utility>
#include <vector>
#include <cassert>
#include <iomanip>
#include <fstream>

#include "test_runner.h"
#include "json.h"
#include "stop.h"
#include "bus.h"
#include "transport.h"
#include "request.h"

using namespace transport;

struct AddStop
          : public AddRequest
{
     ~AddStop() override = default;

     void ParsingFrom( const Json::Node& request ) override
     {
          const auto& requestMap = request.AsMap();
          stopName = requestMap.at( "name" ).AsString();
          stop = Stop( requestMap.at( "latitude" ).AsDouble(), requestMap.at( "longitude" ).AsDouble() );
          const auto& roadDistance = requestMap.at( "road_distances" ).AsMap();
          roadLength.reserve( roadDistance.size() );
          for( const auto& [ key, valueNode ]: roadDistance )
          {
               roadLength.emplace_back( key, valueNode.AsInt() );
          }

     }

     void Process( Transport& transport ) const override
     {
          transport.AddStop( stopName, stop.value(), roadLength );
     }

     std::string stopName;
     std::optional< Stop > stop;
     std::vector< std::pair< std::string, unsigned int >> roadLength;
};

struct AddBus
          : public AddRequest
{
     ~AddBus() override = default;

     void Process( Transport& transport ) const override
     {
          transport.AddBus( busName, bus.value() );
     }

     void ParsingFrom( const Json::Node& request ) override
     {
          const auto& requestMap = request.AsMap();
          busName = requestMap.at( "name" ).AsString();
          bus = Bus( requestMap.at( "is_roundtrip" ).AsBool()? Bus::Type::Circular: Bus::Type::Linear );
          for( const auto& item: requestMap.at( "stops" ).AsArray() )
          {
               bus->AddStop( item.AsString() );
          }
     }

     std::string busName;
     std::optional< Bus > bus;
};

struct GetBusStats
          : public GetRequest
{
     ~GetBusStats() override = default;

     void ParsingFrom( const Json::Node& request ) override
     {
          const auto& requestMap = request.AsMap();
          busName = requestMap.at( "name" ).AsString();
          id = requestMap.at( "id" ).AsInt();
     }

     Json::Node Process( Transport& transport ) const override
     {
          std::map< std::string, Json::Node > result;
          result[ "request_id" ] = id;

          auto stats = transport.GetBusStats( busName );
          if( !stats.has_value() )
          {
               result[ "error_message" ] = std::string( "not found" );
               return Json::Node( result );
          }

          result[ "stop_count" ] = static_cast< int >( stats.value().stops );
          result[ "unique_stop_count" ] = static_cast< int >( stats.value().uniqueStops );
          result[ "route_length" ] = static_cast< int >( stats.value().lengthInfo.roadLength );
          result[ "curvature" ] = stats.value().lengthInfo.roadLength / stats.value().lengthInfo.length;

          return Json::Node( result );
     }

     std::string busName;
};

struct GetStopBusList: public GetRequest
{
     ~GetStopBusList() override = default;

     void ParsingFrom( const Json::Node& request ) override
     {
          const auto& requestMap = request.AsMap();
          stopName = requestMap.at( "name" ).AsString();
          id = requestMap.at( "id" ).AsInt();
     }

     Json::Node Process( Transport& transport ) const override
     {
          std::map< std::string, Json::Node > result;
          result[ "request_id" ] = id;

          auto* busList = transport.GetStopBusList( stopName );
          if( !busList )
          {
               result[ "error_message" ] = std::string( "not found" );
               return Json::Node( result );
          }

          std::vector< Json::Node > buses( busList->begin(), busList->end() );
          result[ "buses" ] = std::move( buses );

          return Json::Node( result );
     }

     std::string stopName;
};

RequestPtr ParsingRequest( Request::Type type, const Json::Node& requestNode )
{
     const std::string& object = requestNode.AsMap().at( "type" ).AsString();

     RequestPtr request;
     if( object == "Stop" )
     {
          switch( type )
          {
               case Request::Add:
                    request = std::make_unique< AddStop >();
                    break;
               case Request::Get:
                    request = std::make_unique< GetStopBusList >();
                    break;
               default:
                    assert( false );
          }
     }
     else if( object == "Bus" )
     {
          switch( type )
          {
               case Request::Add:
                    request = std::make_unique< AddBus >();
                    break;
               case Request::Get:
                    request = std::make_unique< GetBusStats >();
                    break;
               default:
                    assert( false );
          }
     }
     request->ParsingFrom( requestNode );
     return request;
}

std::vector< RequestPtr > ReadRequests( std::istream& in = std::cin )
{
     std::vector< RequestPtr > requests;
     auto doc = Json::Load( in );
     const auto& root = doc.GetRoot();
     const auto& rootMap = root.AsMap();
     const auto& addRequests = rootMap.at( "base_requests" ).AsArray();
     const auto& getRequests = rootMap.at( "stat_requests" ).AsArray();
     requests.reserve( addRequests.size() + getRequests.size() );
     for( const auto& request: addRequests )
     {
          requests.push_back( ParsingRequest( Request::Add, request ) );
     }
     for( const auto& request: getRequests )
     {
          requests.push_back( ParsingRequest( Request::Get, request ) );
     }

     return requests;
}

std::vector< Json::Node > ProcessRequests( const std::vector< RequestPtr >& requests )
{
     std::vector< Json::Node > responses;
     Transport transport;
     for( const auto& request: requests )
     {
          switch( request->type )
          {
               case Request::Add:
               {
                    const auto& addRequest = dynamic_cast< const AddRequest& >( *request );
                    addRequest.Process( transport );
               }
                    break;
               case Request::Get:
               {
                    const auto& getRequest = dynamic_cast< const GetRequest& >( *request );
                    responses.push_back( getRequest.Process( transport ) );
               }
                    break;
          }
     }
     return responses;
}

void PrintResponses( const std::vector< Json::Node >& responses, std::ostream& os = std::cout )
{
     os << '[';
     bool first = true;
     for( const auto& response: responses )
     {
          if( first )
          {
               first = false;
          }
          else
          {
               os << ',';
          }
          response.Print( os );
     }
     os << ']';
}

void BusTest()
{
     Bus::LengthCalculator calc = []( const std::string&, const std::string& )
     {
          return Bus::LengthInfo{ 1, 2 };
     };

     {
          Bus bus( Bus::Circular );
          bus.AddStop( "Biryulyovo Zapadnoye" );
          bus.AddStop( "Biryusinka" );
          bus.AddStop( "Universam" );
          bus.AddStop( "Biryulyovo Tovarnaya" );
          bus.AddStop( "Biryulyovo Passazhirskaya" );
          bus.AddStop( "Biryulyovo Zapadnoye" );
          ASSERT_EQUAL( bus.GetStopsOnRoute(), 6 )
          ASSERT_EQUAL( bus.GetUniqueStops(), 5 )
          ASSERT_EQUAL( bus.GetLength( calc ).length, 5 )
          ASSERT_EQUAL( bus.GetLength( calc ).roadLength, 10 )
     }
     {
          Bus bus( Bus::Linear );
          bus.AddStop( "Tolstopaltsevo" );
          bus.AddStop( "Marushkino" );
          bus.AddStop( "Rasskazovka" );
          ASSERT_EQUAL( bus.GetStopsOnRoute(), 5 )
          ASSERT_EQUAL( bus.GetUniqueStops(), 3 )
          ASSERT_EQUAL( bus.GetLength( calc ).length, 4 )
          ASSERT_EQUAL( bus.GetLength( calc ).roadLength, 8 )
     }
}

void StopTest()
{
     Stop tolstopaltsevo( 55.611087, 37.20829 );
     Stop marushkino( 55.595884, 37.209755 );
     Stop rasskazovka( 55.632761, 37.333324 );

     double len = CalculateLength( tolstopaltsevo, marushkino );
     len += CalculateLength( marushkino, rasskazovka );
     len *= 2;
     std::ostringstream os;
     os << std::setprecision( 6 ) << len;
     ASSERT_EQUAL( os.str(), "20939.5" );
}

void TransportTest()
{
     Transport transport;
     transport.AddStop( "Tolstopaltsevo", Stop( 55.611087, 37.20829 ), { { "Marushkino", 3900 } } );
     transport.AddStop( "Marushkino", Stop( 55.595884, 37.209755 ), { { "Rasskazovka", 9900 } } );
     transport.AddStop( "Rasskazovka", Stop( 55.632761, 37.333324 ), {} );
     {
          Bus bus( Bus::Linear );
          bus.AddStop( "Tolstopaltsevo" );
          bus.AddStop( "Marushkino" );
          bus.AddStop( "Rasskazovka" );
          transport.AddBus( "750", std::move( bus ) );
     }
     {
          auto stats = transport.GetBusStats( "750" );
          ASSERT( stats.has_value() );
          ASSERT_EQUAL( stats.value().stops, 5 );
          ASSERT_EQUAL( stats.value().uniqueStops, 3 );
          ASSERT_EQUAL( stats.value().lengthInfo.roadLength, 27600 );
          {
               std::ostringstream os;
               os << std::setprecision( 6 ) << stats.value().lengthInfo.length;
               ASSERT_EQUAL( os.str(), "20939.5" );
          }
          {
               std::ostringstream os;
               os << std::setprecision( 6 ) << stats.value().lengthInfo.roadLength;
               ASSERT_EQUAL( os.str(), "27600" );
          }
          {
               std::ostringstream os;
               os << std::setprecision( 6 ) << stats.value().lengthInfo.roadLength / stats.value().lengthInfo.length;
               ASSERT_EQUAL( os.str(), "1.31808" );
          }

     }


     {
          Bus bus( Bus::Circular );
          bus.AddStop( "Biryulyovo Zapadnoye" );
          bus.AddStop( "Biryusinka" );
          bus.AddStop( "Universam" );
          bus.AddStop( "Biryulyovo Tovarnaya" );
          bus.AddStop( "Biryulyovo Passazhirskaya" );
          bus.AddStop( "Biryulyovo Zapadnoye" );
          transport.AddBus( "256", std::move( bus ) );
     }
     transport.AddStop( "Biryulyovo Zapadnoye", Stop( 55.574371, 37.6517 ),
                        { { "Rossoshanskaya ulitsa", 7500 },
                          { "Biryusinka",            1800 },
                          { "Universam",             2400 } }
     );
     transport.AddStop( "Biryusinka", Stop( 55.581065, 37.64839 ), { { "Universam", 750 } } );
     transport.AddStop( "Universam", Stop( 55.587655, 37.645687 ),
                        { { "Rossoshanskaya ulitsa", 5600 },
                          { "Biryulyovo Tovarnaya",  900 } }
     );
     transport.AddStop( "Biryulyovo Tovarnaya", Stop( 55.592028, 37.653656 ), { { "Biryulyovo Passazhirskaya", 1300 } } );
     transport.AddStop( "Biryulyovo Passazhirskaya", Stop( 55.580999, 37.659164 ), { { "Biryulyovo Zapadnoye", 1200 } } );

     {
          auto stats = transport.GetBusStats( "256" );
          ASSERT( stats.has_value() );
          ASSERT_EQUAL( stats.value().stops, 6 );
          ASSERT_EQUAL( stats.value().uniqueStops, 5 );
          {
               std::ostringstream os;
               os << std::setprecision( 6 ) << stats.value().lengthInfo.length;
               ASSERT_EQUAL( os.str(), "4371.02" );
          }
          {
               std::ostringstream os;
               os << std::setprecision( 6 ) << stats.value().lengthInfo.roadLength;
               ASSERT_EQUAL( os.str(), "5950" );
          }
          {
               std::ostringstream os;
               os << std::setprecision( 6 ) << stats.value().lengthInfo.roadLength / stats.value().lengthInfo.length;
               ASSERT_EQUAL( os.str(), "1.36124" );
          }
     }

     {
          auto stats = transport.GetBusStats( "751" );
          ASSERT( !stats.has_value() );
     }

     {
          auto* res = transport.GetStopBusList( "Samara" );
          ASSERT( res == nullptr );
     }
     {
          transport.AddStop( "Prazhskaya", Stop( 55.611678, 37.603831 ), { } );
          auto* res = transport.GetStopBusList( "Prazhskaya" );
          ASSERT( res->empty() );
     }
     {
          auto* res = transport.GetStopBusList( "Biryulyovo Zapadnoye" );
          ASSERT_EQUAL( res->size(), 1 );
     }
}

void JsonReadTest()
{
     static const std::string inStr = "{\n"
                                      "\"bool_true\": true,\n"
                                      "\"bool_false\": false,\n"

                                      "\"double1\": 123.321,\n"
                                      "\"double2\": -123.321,\n"
                                      "\"double3\": 123\n"

                                      "}\n";
     std::istringstream in( inStr );
     auto doc = Json::Load( in );
     const auto& root = doc.GetRoot();
     const auto& map = root.AsMap();

     ASSERT( map.at("bool_true").AsBool() );
     ASSERT( !map.at("bool_false").AsBool() );

     ASSERT_EQUAL( map.at("double1").AsDouble(), 123.321 );
     ASSERT_EQUAL( map.at("double2").AsDouble(), -123.321 );
     ASSERT_EQUAL( map.at("double3").AsDouble(), 123 );
}

void JsonTest()
{
     static const std::string inStr = "{\n"
                                      "  \"base_requests\": [\n"
                                      "    {\n"
                                      "      \"type\": \"Stop\",\n"
                                      "      \"road_distances\": {\n"
                                      "        \"Marushkino\": 3900\n"
                                      "      },\n"
                                      "      \"longitude\": 37.20829,\n"
                                      "      \"name\": \"Tolstopaltsevo\",\n"
                                      "      \"latitude\": 55.611087\n"
                                      "    },\n"
                                      "    {\n"
                                      "      \"type\": \"Stop\",\n"
                                      "      \"road_distances\": {\n"
                                      "        \"Rasskazovka\": 9900\n"
                                      "      },\n"
                                      "      \"longitude\": 37.209755,\n"
                                      "      \"name\": \"Marushkino\",\n"
                                      "      \"latitude\": 55.595884\n"
                                      "    },\n"
                                      "    {\n"
                                      "      \"type\": \"Bus\",\n"
                                      "      \"name\": \"256\",\n"
                                      "      \"stops\": [\n"
                                      "        \"Biryulyovo Zapadnoye\",\n"
                                      "        \"Biryusinka\",\n"
                                      "        \"Universam\",\n"
                                      "        \"Biryulyovo Tovarnaya\",\n"
                                      "        \"Biryulyovo Passazhirskaya\",\n"
                                      "        \"Biryulyovo Zapadnoye\"\n"
                                      "      ],\n"
                                      "      \"is_roundtrip\": true\n"
                                      "    },\n"
                                      "    {\n"
                                      "      \"type\": \"Bus\",\n"
                                      "      \"name\": \"750\",\n"
                                      "      \"stops\": [\n"
                                      "        \"Tolstopaltsevo\",\n"
                                      "        \"Marushkino\",\n"
                                      "        \"Rasskazovka\"\n"
                                      "      ],\n"
                                      "      \"is_roundtrip\": false\n"
                                      "    },\n"
                                      "    {\n"
                                      "      \"type\": \"Stop\",\n"
                                      "      \"road_distances\": {},\n"
                                      "      \"longitude\": 37.333324,\n"
                                      "      \"name\": \"Rasskazovka\",\n"
                                      "      \"latitude\": 55.632761\n"
                                      "    },\n"
                                      "    {\n"
                                      "      \"type\": \"Stop\",\n"
                                      "      \"road_distances\": {\n"
                                      "        \"Rossoshanskaya ulitsa\": 7500,\n"
                                      "        \"Biryusinka\": 1800,\n"
                                      "        \"Universam\": 2400\n"
                                      "      },\n"
                                      "      \"longitude\": 37.6517,\n"
                                      "      \"name\": \"Biryulyovo Zapadnoye\",\n"
                                      "      \"latitude\": 55.574371\n"
                                      "    },\n"
                                      "    {\n"
                                      "      \"type\": \"Stop\",\n"
                                      "      \"road_distances\": {\n"
                                      "        \"Universam\": 750\n"
                                      "      },\n"
                                      "      \"longitude\": 37.64839,\n"
                                      "      \"name\": \"Biryusinka\",\n"
                                      "      \"latitude\": 55.581065\n"
                                      "    },\n"
                                      "    {\n"
                                      "      \"type\": \"Stop\",\n"
                                      "      \"road_distances\": {\n"
                                      "        \"Rossoshanskaya ulitsa\": 5600,\n"
                                      "        \"Biryulyovo Tovarnaya\": 900\n"
                                      "      },\n"
                                      "      \"longitude\": 37.645687,\n"
                                      "      \"name\": \"Universam\",\n"
                                      "      \"latitude\": 55.587655\n"
                                      "    },\n"
                                      "    {\n"
                                      "      \"type\": \"Stop\",\n"
                                      "      \"road_distances\": {\n"
                                      "        \"Biryulyovo Passazhirskaya\": 1300\n"
                                      "      },\n"
                                      "      \"longitude\": 37.653656,\n"
                                      "      \"name\": \"Biryulyovo Tovarnaya\",\n"
                                      "      \"latitude\": 55.592028\n"
                                      "    },\n"
                                      "    {\n"
                                      "      \"type\": \"Stop\",\n"
                                      "      \"road_distances\": {\n"
                                      "        \"Biryulyovo Zapadnoye\": 1200\n"
                                      "      },\n"
                                      "      \"longitude\": 37.659164,\n"
                                      "      \"name\": \"Biryulyovo Passazhirskaya\",\n"
                                      "      \"latitude\": 55.580999\n"
                                      "    },\n"
                                      "    {\n"
                                      "      \"type\": \"Bus\",\n"
                                      "      \"name\": \"828\",\n"
                                      "      \"stops\": [\n"
                                      "        \"Biryulyovo Zapadnoye\",\n"
                                      "        \"Universam\",\n"
                                      "        \"Rossoshanskaya ulitsa\",\n"
                                      "        \"Biryulyovo Zapadnoye\"\n"
                                      "      ],\n"
                                      "      \"is_roundtrip\": true\n"
                                      "    },\n"
                                      "    {\n"
                                      "      \"type\": \"Stop\",\n"
                                      "      \"road_distances\": {},\n"
                                      "      \"longitude\": 37.605757,\n"
                                      "      \"name\": \"Rossoshanskaya ulitsa\",\n"
                                      "      \"latitude\": 55.595579\n"
                                      "    },\n"
                                      "    {\n"
                                      "      \"type\": \"Stop\",\n"
                                      "      \"road_distances\": {},\n"
                                      "      \"longitude\": 37.603831,\n"
                                      "      \"name\": \"Prazhskaya\",\n"
                                      "      \"latitude\": 55.611678\n"
                                      "    }\n"
                                      "  ],\n"
                                      "  \"stat_requests\": [\n"
                                      "    {\n"
                                      "      \"type\": \"Bus\",\n"
                                      "      \"name\": \"256\",\n"
                                      "      \"id\": 1965312327\n"
                                      "    },\n"
                                      "    {\n"
                                      "      \"type\": \"Bus\",\n"
                                      "      \"name\": \"750\",\n"
                                      "      \"id\": 519139350\n"
                                      "    },\n"
                                      "    {\n"
                                      "      \"type\": \"Bus\",\n"
                                      "      \"name\": \"751\",\n"
                                      "      \"id\": 194217464\n"
                                      "    },\n"
                                      "    {\n"
                                      "      \"type\": \"Stop\",\n"
                                      "      \"name\": \"Samara\",\n"
                                      "      \"id\": 746888088\n"
                                      "    },\n"
                                      "    {\n"
                                      "      \"type\": \"Stop\",\n"
                                      "      \"name\": \"Prazhskaya\",\n"
                                      "      \"id\": 65100610\n"
                                      "    },\n"
                                      "    {\n"
                                      "      \"type\": \"Stop\",\n"
                                      "      \"name\": \"Biryulyovo Zapadnoye\",\n"
                                      "      \"id\": 1042838872\n"
                                      "    }\n"
                                      "  ]\n"
                                      "}\n";
     std::istringstream in( inStr );

     std::fstream out("../out.json", std::ios::out | std::ios::trunc);

     auto requests = ReadRequests( in );
     auto responses = ProcessRequests( requests );
     PrintResponses( responses, out );


}

int main()
{
//     TestRunner testRunner;
//     RUN_TEST( testRunner, BusTest );
//     RUN_TEST( testRunner, StopTest );
//     RUN_TEST( testRunner, TransportTest );
//     RUN_TEST( testRunner, JsonReadTest );
//     RUN_TEST( testRunner, JsonTest );
//     return 0;

     auto requests = ReadRequests();
     auto responses = ProcessRequests( requests );
     PrintResponses( responses );

     return 0;
}
