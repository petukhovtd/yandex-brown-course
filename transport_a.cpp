#include <iostream>
#include <cmath>
#include <unordered_map>
#include <utility>
#include <vector>
#include <set>
#include <cassert>
#include <functional>
#include <iomanip>

#include "test_runner.h"

struct Stop
{
     Stop( double latitude, double longitude )
               : lat( ToRadian( latitude ) )
               , lon( ToRadian( longitude ) )
     {}

     double lat;
     double lon;

private:
     static double ToRadian( double degree )
     {
          return degree * 3.1415926535 / 180.0;
     }
};

double CalculateLength( const Stop& rhs, const Stop& lhs )
{
     return std::acos(
               std::sin( lhs.lat ) * std::sin( rhs.lat )
               + std::cos( lhs.lat ) * std::cos( rhs.lat )
                 * std::cos( std::abs( lhs.lon - rhs.lon ) ) ) * 6371000;
}

class Bus
{
public:
     enum Type
     {
          Linear,
          Circular
     };

     struct Stats
     {
          size_t stops;
          size_t uniqueStops;
          double length;
     };

     using LengthCalculator = std::function< double( const std::string&, const std::string& ) >;

     explicit Bus( Type type )
               : type_( type )
               , length_()
               , stops_()
               , uniqueStops_()
     {}

     void AddStop( std::string stopName )
     {
          uniqueStops_.insert( stopName );
          stops_.push_back( std::move( stopName ) );
     }

     size_t GetStopsOnRoute() const
     {
          switch( type_ )
          {

               case Linear:
                    return stops_.size() * 2 - 1;
               case Circular:
                    return stops_.size();
               default:
                    assert( false );
          }
     }

     size_t GetUniqueStops() const
     {
          return uniqueStops_.size();
     }

     const std::set< std::string >& GetUniqueStopsList() const
     {
          return uniqueStops_;
     }

     double GetLength( const LengthCalculator& lengthCalculator )
     {
          if( length_.has_value() )
          {
               return length_.value();
          }

          length_ = 0;

          if( stops_.empty() )
          {
               return length_.value();
          }

          for( size_t i = 0; i < ( stops_.size() - 1 ); ++i )
          {
               length_.value() += lengthCalculator( stops_[ i ], stops_[ i + 1 ] );
          }

          if( type_ == Linear )
          {
               length_.value() *= 2;
          }

          return length_.value();
     }

private:
     Type type_;
     std::optional< double > length_;
     std::vector< std::string > stops_;
     std::set< std::string > uniqueStops_;
};

class Transport
{
     struct StopInfo
     {
          std::optional< Stop > stop;
          std::set< std::string > buses;
     };

public:
     void AddStop( std::string name, Stop stop )
     {
          stops_[ std::move( name ) ].stop = stop;
     }

     void AddBus( std::string name, Bus bus )
     {
          for( const auto& stop: bus.GetUniqueStopsList() )
          {
               stops_[ stop ].buses.insert( name );
          }
          buses_.insert( { std::move( name ), std::move( bus ) } );
     }

     std::optional< Bus::Stats > GetBusStats( const std::string& name )
     {
          auto it = buses_.find( name );
          if( it == buses_.end() )
          {
               return std::nullopt;
          }
          Bus& bus = it->second;
          return Bus::Stats { bus.GetStopsOnRoute(), bus.GetUniqueStops(), bus.GetLength( GetBusLengthCalculator() ) };
     }

     const std::set< std::string >* GetStopBusList( const std::string& name ) const
     {
          auto it = stops_.find( name );
          if( it == stops_.end() )
          {
               return nullptr;
          }
          return &it->second.buses;
     }

private:
     Bus::LengthCalculator GetBusLengthCalculator()
     {
          return [ = ]( const std::string& s1, const std::string& s2 )->double
          {
               return CalculateLength( stops_.at( s1 ).stop.value(), stops_.at( s2 ).stop.value() );
          };
     }

private:
     std::unordered_map< std::string, StopInfo > stops_;
     std::unordered_map< std::string, Bus > buses_;
};


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

     virtual void ParsingFrom( std::string_view str ) = 0;

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

     virtual std::string Process( Transport& transport ) const = 0;
};

std::pair< std::string_view, std::optional< std::string_view > >
SplitTwoStrict( std::string_view s, std::string_view delimiter = " " )
{
     const size_t pos = s.find( delimiter );
     if( pos == s.npos )
     {
          return { s, std::nullopt };
     }
     else
     {
          return { s.substr( 0, pos ), s.substr( pos + delimiter.length() ) };
     }
}

std::pair< std::string_view, std::string_view > SplitTwo( std::string_view s, std::string_view delimiter = " " )
{
     const auto[lhs, rhs_opt] = SplitTwoStrict( s, delimiter );
     return { lhs, rhs_opt.value_or( "" ) };
}

std::string_view ReadToken( std::string_view& s, std::string_view delimiter = " " )
{
     const auto[lhs, rhs] = SplitTwo( s, delimiter );
     s = rhs;
     return lhs;
}

std::string_view Trim( std::string_view str )
{
     str.remove_prefix( std::min( str.find_first_not_of( " \t\r\v\n" ), str.size() ) );
     str.remove_suffix( std::min( str.size() - str.find_last_not_of( " \t\r\v\n" ) - 1, str.size() ) );
     return str;
}

struct AddStop
          : public AddRequest
{
     ~AddStop() override = default;

     void ParsingFrom( std::string_view str ) override
     {
          stopName = Trim( ReadToken( str, ":" ) );
          std::string_view lan = ReadToken( str, "," );
          std::string_view lon = ReadToken( str, "," );
          stop = Stop( std::stod( std::string( lan ) ), std::stod( std::string( lon ) ) );
     }

     void Process( Transport& transport ) const override
     {
          transport.AddStop( stopName, stop.value() );
     }

     std::string stopName;
     std::optional< Stop > stop;
};

struct AddBus
          : public AddRequest
{
     ~AddBus() override = default;

     void ParsingFrom( std::string_view str ) override
     {
          busName = Trim( ReadToken( str, ":" ) );
          std::string delim;
          auto it = std::find_if( str.begin(), str.end(), [ &delim ]( char c )
          {
               if( c == '>' || c == '-' )
               {
                    delim = c;
                    return true;
               }
               return false;
          } );
          if( it == str.end() )
          {
               throw std::invalid_argument( "invalid bus route" );
          }
          bus = Bus( [ delim ]()->Bus::Type
                     {
                          if( delim == ">" )
                          {
                               return Bus::Circular;
                          }
                          else if( delim == "-" )
                          {
                               return Bus::Linear;
                          }
                          throw std::invalid_argument( "invalid bus type" );
                     }() );
          while( !str.empty() )
          {
               std::string_view stop = ReadToken( str, delim );
               bus->AddStop( std::string( Trim( stop ) ) );
          }
     }

     void Process( Transport& transport ) const override
     {
          transport.AddBus( busName, bus.value() );
     }

     std::string busName;
     std::optional< Bus > bus;
};

struct GetBusStats
          : public GetRequest
{
     ~GetBusStats() override = default;

     void ParsingFrom( std::string_view str ) override
     {
          busName = Trim( str );
     }

     std::string Process( Transport& transport ) const override
     {
          auto stats = transport.GetBusStats( busName );
          std::ostringstream os;
          os.precision( 6 );
          os << "Bus " << busName << ": ";
          if( !stats.has_value() )
          {
               os << "not found";
               return os.str();
          }

          os << stats.value().stops << " stops on route, " << stats.value().uniqueStops << " unique stops, "
             << stats.value().length << " route length";
          return os.str();
     }

     std::string busName;
};

struct GetStopBusList: public GetRequest
{
     ~GetStopBusList() override = default;

     void ParsingFrom( std::string_view str ) override
     {
          stopName = Trim( str );
     }

     std::string Process( Transport& transport ) const override
     {
          auto* busList = transport.GetStopBusList( stopName );
          std::ostringstream os;
          os << "Stop " << stopName << ": ";
          if( !busList )
          {
               os << "not found";
          }
          else if( busList->empty() )
          {
               os << "no buses";
          }
          else
          {
               os << "buses";
               for( const auto& bus: *busList )
               {
                    os << " " << bus;
               }
          }
          return os.str();
     }

     std::string stopName;
};

size_t ReadRequestNumber( std::istream& in )
{
     size_t num;
     in >> num;
     std::string empty;
     std::getline( in, empty );
     return num;
}

RequestPtr ParsingRequest( Request::Type type, std::string_view str )
{
     std::string_view object = ReadToken( str );

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
     request->ParsingFrom( str );
     return request;
}

void ReadRequestsType( std::vector< RequestPtr >& requests, Request::Type type, std::istream& in )
{
     size_t number = ReadRequestNumber( in );
     requests.reserve( requests.size() + number );
     for( size_t i = 0; i < number; ++i )
     {
          std::string str;
          getline( in, str );
          requests.push_back( ParsingRequest( type, str ) );
     }
}

std::vector< RequestPtr > ReadRequests( std::istream& in = std::cin )
{
     std::vector< RequestPtr > requests;
     ReadRequestsType( requests, Request::Add, in );
     ReadRequestsType( requests, Request::Get, in );
     return requests;
}

std::vector< std::string > ProcessRequests( const std::vector< RequestPtr >& requests )
{
     std::vector< std::string > responses;
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

void PrintResponses( const std::vector< std::string >& responses, std::ostream& os = std::cout )
{
     for( const auto& response: responses )
     {
          os << response << '\n';
     }
}

void BusTest()
{
     Bus::LengthCalculator calc = []( const std::string&, const std::string& )
     {
          return 1;
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
          ASSERT_EQUAL( bus.GetLength( calc ), 5 )
     }
     {
          Bus bus( Bus::Linear );
          bus.AddStop( "Tolstopaltsevo" );
          bus.AddStop( "Marushkino" );
          bus.AddStop( "Rasskazovka" );
          ASSERT_EQUAL( bus.GetStopsOnRoute(), 5 )
          ASSERT_EQUAL( bus.GetUniqueStops(), 3 )
          ASSERT_EQUAL( bus.GetLength( calc ), 4 )
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
     transport.AddStop( "Tolstopaltsevo", Stop( 55.611087, 37.20829 ) );
     transport.AddStop( "Marushkino", Stop( 55.595884, 37.209755 ) );
     transport.AddStop( "Rasskazovka", Stop( 55.632761, 37.333324 ) );
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
          std::ostringstream os;
          os << std::setprecision( 6 ) << stats.value().length;
          ASSERT_EQUAL( os.str(), "20939.5" );
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
     transport.AddStop( "Biryulyovo Zapadnoye", Stop( 55.574371, 37.6517 ) );
     transport.AddStop( "Biryusinka", Stop( 55.581065, 37.64839 ) );
     transport.AddStop( "Universam", Stop( 55.587655, 37.645687 ) );
     transport.AddStop( "Biryulyovo Tovarnaya", Stop( 55.592028, 37.653656 ) );
     transport.AddStop( "Biryulyovo Passazhirskaya", Stop( 55.580999, 37.659164 ) );

     {
          auto stats = transport.GetBusStats( "256" );
          ASSERT( stats.has_value() );
          ASSERT_EQUAL( stats.value().stops, 6 );
          ASSERT_EQUAL( stats.value().uniqueStops, 5 );
          std::ostringstream os;
          os << std::setprecision( 6 ) << stats.value().length;
          ASSERT_EQUAL( os.str(), "4371.02" );
     }

     {
          auto stats = transport.GetBusStats( "751" );
          ASSERT( !stats.has_value() );
     }
}

void ParsingStopTest()
{
     AddStop addStop;
     std::string s = "Tolstopaltsevo asd: 55.611087, 37.20829";
     addStop.ParsingFrom( s );
     ASSERT_EQUAL( addStop.stopName, "Tolstopaltsevo asd" );
     ASSERT( addStop.stop.value().lat > 0 );
     ASSERT( addStop.stop.value().lon > 0 );
}

void ParsingBusTest()
{
     AddBus addBus;
     std::string s = "256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye";
     addBus.ParsingFrom( s );
     ASSERT_EQUAL( addBus.busName, "256" );
     ASSERT_EQUAL( addBus.bus.value().GetStopsOnRoute(), 6 );
     ASSERT_EQUAL( addBus.bus.value().GetUniqueStops(), 5 );

}

void RWTest()
{
     static const std::string inputStr = "10\n"
                                         "Stop Tolstopaltsevo: 55.611087, 37.20829\n"
                                         "Stop Marushkino: 55.595884, 37.209755\n"
                                         "Bus 256 aaa: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n"
                                         "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka\n"
                                         "Stop Rasskazovka: 55.632761, 37.333324\n"
                                         "Stop Biryulyovo Zapadnoye: 55.574371, 37.6517\n"
                                         "Stop Biryusinka: 55.581065, 37.64839\n"
                                         "Stop Universam: 55.587655, 37.645687\n"
                                         "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656\n"
                                         "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164\n"
                                         "3\n"
                                         "Bus 256 aaa\n"
                                         "Bus 750\n"
                                         "Bus 751\n";
     std::stringstream in( inputStr );
     std::stringstream os;
     auto requests = ReadRequests( in );
     auto responses = ProcessRequests( requests );
     PrintResponses( responses, os );

     std::string outStr = "Bus 256 aaa: 6 stops on route, 5 unique stops, 4371.02 route length\n"
                          "Bus 750: 5 stops on route, 3 unique stops, 20939.5 route length\n"
                          "Bus 751: not found\n";
     ASSERT_EQUAL( outStr, os.str() );
}

void RWTestPartB()
{
     static const std::string inputStr = "13\n"
                                         "Stop Tolstopaltsevo: 55.611087, 37.20829\n"
                                         "Stop Marushkino: 55.595884, 37.209755\n"
                                         "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n"
                                         "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka\n"
                                         "Stop Rasskazovka: 55.632761, 37.333324\n"
                                         "Stop Biryulyovo Zapadnoye: 55.574371, 37.6517\n"
                                         "Stop Biryusinka: 55.581065, 37.64839\n"
                                         "Stop Universam: 55.587655, 37.645687\n"
                                         "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656\n"
                                         "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164\n"
                                         "Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye\n"
                                         "Stop Rossoshanskaya ulitsa: 55.595579, 37.605757\n"
                                         "Stop Prazhskaya: 55.611678, 37.603831\n"
                                         "6\n"
                                         "Bus 256\n"
                                         "Bus 750\n"
                                         "Bus 751\n"
                                         "Stop Samara\n"
                                         "Stop Prazhskaya\n"
                                         "Stop Biryulyovo Zapadnoye\n";
     std::stringstream in( inputStr );
     std::stringstream os;
     auto requests = ReadRequests( in );
     auto responses = ProcessRequests( requests );
     PrintResponses( responses, os );

     std::string outStr = "Bus 256: 6 stops on route, 5 unique stops, 4371.02 route length\n"
                          "Bus 750: 5 stops on route, 3 unique stops, 20939.5 route length\n"
                          "Bus 751: not found\n"
                          "Stop Samara: not found\n"
                          "Stop Prazhskaya: no buses\n"
                          "Stop Biryulyovo Zapadnoye: buses 256 828\n";
     ASSERT_EQUAL( outStr, os.str() );
}

void TrimTest()
{
     ASSERT_EQUAL( "abc", Trim( " abc " ) );
     ASSERT_EQUAL( "abc", Trim( " abc" ) );
     ASSERT_EQUAL( "abc", Trim( "abc " ) );
     ASSERT_EQUAL( "abc", Trim( "abc" ) );

}

int main()
{
//     TestRunner testRunner;
//     RUN_TEST( testRunner, BusTest );
//     RUN_TEST( testRunner, StopTest );
//     RUN_TEST( testRunner, TransportTest );
//     RUN_TEST( testRunner, ParsingStopTest );
//     RUN_TEST( testRunner, ParsingBusTest );
//     RUN_TEST( testRunner, RWTest );
//     RUN_TEST( testRunner, TrimTest );
//     RUN_TEST( testRunner, RWTestPartB );
//     return 0;

     auto requests = ReadRequests();
     auto responses = ProcessRequests( requests );
     PrintResponses( responses );

     return 0;
}
