#include <iostream>
#include <cmath>
#include <unordered_map>
#include <vector>
#include <set>
#include <cassert>

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
                 * std::cos( std::abs( lhs.lon - rhs.lon ) ) * 6371000 );
}


class Bus
{
public:
     enum Type
     {
          Linear,
          Circular
     };

     explicit Bus( std::string busName, Type type )
               : name_( std::move( busName) )
               , type_( type )
               , length_( 0 )
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

               case Linear: return stops_.size() * 2 - 1;
               case Circular: return stops_.size();
               default:
                    assert( false );
          }
     }

     size_t GetUniqueStops() const
     {
          return uniqueStops_.size();
     }

     double GetLength()
     {
          if( length_.has_value() )
          {
               return length_.value();
          }

          return -1;
     }

private:
     std::string name_;
     Type type_;
     std::vector< std::string > stops_;
     std::set< std::string > uniqueStops_;
     std::optional< double > length_;
};

class Transport
{
public:
     void AddStop( std::string name, Stop stop )
     {
          stops_.insert( { std::move( name ), stop } );
     }

private:
     std::unordered_map< std::string, Stop > stops_;
};

int main()
{
     {
          Bus bus( "256", Bus::Circular );
          bus.AddStop( "Biryulyovo Zapadnoye" );
          bus.AddStop( "Biryusinka" );
          bus.AddStop( "Universam" );
          bus.AddStop( "Biryulyovo Tovarnaya" );
          bus.AddStop( "Biryulyovo Passazhirskaya" );
          bus.AddStop( "Biryulyovo Zapadnoye" );
          std::cout << bus.GetStopsOnRoute() << '\n';
          std::cout << bus.GetUniqueStops() << '\n';
     }
     {
          Bus bus( "750", Bus::Linear );
          bus.AddStop( "Tolstopaltsevo" );
          bus.AddStop( "Marushkino" );
          bus.AddStop( "Rasskazovka" );
          std::cout << bus.GetStopsOnRoute() << '\n';
          std::cout << bus.GetUniqueStops() << '\n';
     }
     return 0;
}
