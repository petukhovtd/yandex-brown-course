#ifndef YANDEX_BROWN_COURSE_BOOKING_H
#define YANDEX_BROWN_COURSE_BOOKING_H

#include <iostream>

namespace RAII
{

template< class Provider >
class Booking
{
public:
     Booking( Provider* provider, int id )
     : provider_( provider )
     , id_( id )
     {}

     Booking( const Booking< Provider >& ) = delete;

     Booking( Booking< Provider >&& other ) noexcept
     {
          provider_ = other.provider_;
          id_ = other.id_;
          other.provider_ = nullptr;
          other.id_ = 0;
     }

     Booking< Provider >& operator=( const Booking< Provider >& ) = delete;

     Booking< Provider >& operator=( Booking< Provider >&& other ) noexcept
     {
          End();
          provider_ = other.provider_;
          id_ = other.id_;
          other.provider_ = nullptr;
          other.id_ = 0;
          return *this;
     }

     ~Booking()
     {
          End();
     }

private:
     void End()
     {
          if( provider_ )
          {
               provider_->CancelOrComplete( *this );
          }
     }

private:
     Provider* provider_;
     int id_;
};

}

#endif
