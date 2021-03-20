#include <iostream>
#include <string>
#include <sstream>
#include <ctime>
#include <vector>
#include <iomanip>
#include <fstream>

#include "profile.h"

struct Data
{
     static const long int SECONDS_IN_DAY = 60 * 60 * 24;

     int year;
     int month;
     int day;

     friend std::istream& operator>>( std::istream& is, Data& data )
     {
          is >> data.year;
          is.ignore();
          is >> data.month;
          is.ignore();
          is >> data.day;
          return is;
     }

     time_t AsTimeStamp() const
     {
          std::tm t;
          t.tm_sec = 0;
          t.tm_min = 0;
          t.tm_hour = 0;
          t.tm_mday = day;
          t.tm_mon = month - 1;
          t.tm_year = year - 1900;
          t.tm_isdst = 0;
          return mktime( &t );
     }

     long int DiffInDays( const Data& other ) const
     {
          return ( other.AsTimeStamp() - AsTimeStamp() ) / SECONDS_IN_DAY;
     }

     void NextDay()
     {
          time_t tp = AsTimeStamp() + SECONDS_IN_DAY;
          std::tm* tm = localtime( &tp );
          if( !tm )
          {
               return;
          }
          day = tm->tm_mday;
          month = tm->tm_mon + 1;
          year = tm->tm_year + 1900;
     }

     friend std::ostream& operator<<( std::ostream& os, const Data& data )
     {
          os << data.year << '-' << data.month << '-' << data.day;
          return os;
     }

     bool operator<( const Data& other ) const
     {
          return std::tie( year, month, day ) < std::tie( other.year, other.month, other.day );
     }

     bool operator==( const Data& other )
     {
          return year == other.year && month == other.month && day == other.day;
     }
};

class Budget
{
public:
     using ValueType = double;

private:
     struct Bug
     {
          ValueType income;
          ValueType cost;
     };

public:
     using Storage = std::vector< Bug >;

     Budget( const Data& from, const Data& to )
               : storage_( from.DiffInDays( to ) )
               , from_( from )
     {}

     void Earn( Data from, Data to, unsigned int value )
     {
          size_t start = from_.DiffInDays( from );
          size_t days = from.DiffInDays( to );
          ValueType addInDay = value / ( ValueType ) ( days + 1 );

          for( size_t day = start; day <= ( start + days ); ++day )
          {
               storage_[ day ].income += addInDay;
          }
     }

     void Spend( Data from, Data to, unsigned int value )
     {
          size_t start = from_.DiffInDays( from );
          size_t days = from.DiffInDays( to );
          ValueType addInDay = value / ( ValueType ) ( days + 1 );

          for( size_t day = start; day <= ( start + days ); ++day )
          {
               storage_[ day ].cost += addInDay;
          }
     }

     void PayTax( Data from, Data to, unsigned int percentage )
     {
          size_t start = from_.DiffInDays( from );
          size_t end = start + from.DiffInDays( to );

          for( size_t day = start; day <= end; ++day )
          {
               storage_[ day ].income = storage_[ day ].income * ( 100.0 - percentage ) / 100.0;
          }
     }

     ValueType ComputeIncome( Data from, Data to ) const
     {
          size_t start = from_.DiffInDays( from );
          size_t end = start + from.DiffInDays( to );

          ValueType value = 0;
          for( size_t day = start; day <= end; ++day )
          {
               value += storage_[ day ].income - storage_[ day ].cost;
          }
          return value;
     }

private:
     Storage storage_;
     Data from_;
};

void Process( Budget& budget, std::istream& is, std::ostream& os )
{
     os.precision( 25 );
     int queryCount;
     is >> queryCount;
     for( int i = 0; i < queryCount; ++i )
     {
          std::string action;
          Data from {};
          Data to {};
          is >> action >> from >> to;
          if( action == "Earn" )
          {
               unsigned int value;
               is >> value;
               budget.Earn( from, to, value );
          }
          if( action == "Spend" )
          {
               unsigned int value;
               is >> value;
               budget.Spend( from, to, value );
          }
          else if( action == "PayTax" )
          {
               unsigned int percentage;
               is >> percentage;
               budget.PayTax( from, to, percentage );
          }
          else if( action == "ComputeIncome" )
          {
               os << budget.ComputeIncome( from, to ) << '\n';
          }
     }
}

void Test()
{
     std::string target = "8\n"
                          "Earn 2000-01-02 2000-01-06 20\n"
                          "ComputeIncome 2000-01-01 2001-01-01\n"
                          "PayTax 2000-01-02 2000-01-03 13\n"
                          "ComputeIncome 2000-01-01 2001-01-01\n"
                          "Spend 2000-12-30 2001-01-02 14\n"
                          "ComputeIncome 2000-01-01 2001-01-01\n"
                          "PayTax 2000-12-30 2000-12-30 13\n"
                          "ComputeIncome 2000-01-01 2001-01-01\n";
     std::stringstream ss( target );

     Budget budget( Data { .year=2000, .month=1, .day = 1 }, Data { .year=2099, .month=1, .day = 1 } );
     Process( budget, ss, std::cout );
}

int main()
{
     //     Test();
     //     return 0;

     std::ios_base::sync_with_stdio( false );
     std::cin.tie( nullptr );

     Budget budget( Data { .year=2000, .month=1, .day = 1 }, Data { .year=2200, .month=1, .day = 1 } );
     Process( budget, std::cin, std::cout );

     return 0;
}
