#ifndef YANDEX_BROWN_COURSE_STOP_H
#define YANDEX_BROWN_COURSE_STOP_H

namespace transport
{

struct Stop
{
     Stop( double latitude, double longitude );

     double lat;
     double lon;

private:
     static double ToRadian( double degree );
};

double CalculateLength( const Stop& rhs, const Stop& lhs );

}

#endif
