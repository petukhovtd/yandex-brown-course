#include "bus.h"
#include <cassert>

namespace transport
{

Bus::Bus( Type type )
: type_( type )
, lengthInfo_()
, stops_()
, uniqueStops_()
{}

void Bus::AddStop( std::string stopName )
{
     uniqueStops_.insert( stopName );
     stops_.push_back( std::move( stopName ) );
}

size_t Bus::GetStopsOnRoute() const
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

size_t Bus::GetUniqueStops() const
{
     return uniqueStops_.size();
}

const std::set< std::string >& Bus::GetUniqueStopsList() const
{
     return uniqueStops_;
}

Bus::LengthInfo Bus::GetLength( const LengthCalculator& lengthCalculator )
{
     if( lengthInfo_.has_value() )
     {
          return lengthInfo_.value();
     }

     lengthInfo_ = { 0, 0 };

     if( stops_.empty() )
     {
          return lengthInfo_.value();
     }

     for( size_t i = 0; i < ( stops_.size() - 1 ); ++i )
     {
          lengthInfo_.value() += lengthCalculator( stops_[ i ], stops_[ i + 1 ] );
          if( type_ == Linear )
          {
               lengthInfo_.value() += lengthCalculator( stops_[ i + 1 ], stops_[ i ] );
          }
     }

     return lengthInfo_.value();
}

}
