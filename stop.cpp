#include "stop.h"
#include <math.h>

namespace transport
{

Stop::Stop( double latitude, double longitude )
          : lat( ToRadian( latitude ) )
          , lon( ToRadian( longitude ) )
{}

double Stop::ToRadian(double degree)
{
     return degree * 3.1415926535 / 180.0;
}

double CalculateLength( const Stop& rhs, const Stop& lhs )
{
     return std::acos(
               std::sin( lhs.lat ) * std::sin( rhs.lat )
               + std::cos( lhs.lat ) * std::cos( rhs.lat )
                 * std::cos( std::abs( lhs.lon - rhs.lon ) ) ) * 6371000;
}

}
