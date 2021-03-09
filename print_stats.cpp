#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

//enum class Gender
//{
//     FEMALE,
//     MALE
//};
//
//struct Person
//{
//     int age;  // возраст
//     Gender gender;  // пол
//     bool is_employed;  // имеет ли работу
//};
//
//// Это пример функции, его не нужно отправлять вместе с функцией PrintStats
//template< typename InputIt >
//int ComputeMedianAge( InputIt range_begin, InputIt range_end )
//{
//     if( range_begin == range_end )
//     {
//          return 0;
//     }
//     vector< typename InputIt::value_type > range_copy( range_begin, range_end );
//     auto middle = begin( range_copy ) + range_copy.size() / 2;
//     nth_element(
//               begin( range_copy ), middle, end( range_copy ),
//               []( const Person& lhs, const Person& rhs )
//               {
//                    return lhs.age < rhs.age;
//               }
//     );
//     return middle->age;
//}

void PrintStats( vector< Person > persons )
{
     int age, females, males, employedFemales, unemployedFemales, employedMales, unemployedMales;

     age = ComputeMedianAge( persons.begin(), persons.end() );

     auto itFemales = std::partition( persons.begin(), persons.end(), []( const Person& p )
     {
          return p.gender == Gender::FEMALE;
     } );

     females = ComputeMedianAge( persons.begin(), itFemales );

     auto itEmployedFemales = std::partition( persons.begin(), itFemales, []( const Person& p )
     {
          return p.is_employed;
     } );

     employedFemales = ComputeMedianAge( persons.begin(), itEmployedFemales );
     unemployedFemales = ComputeMedianAge( itEmployedFemales, itFemales );

     auto itMales = std::partition( persons.begin(), persons.end(), []( const Person& p )
     {
          return p.gender == Gender::MALE;
     } );

     males = ComputeMedianAge( persons.begin(), itMales );

     auto itEmployedMale = std::partition( persons.begin(), itMales, []( const Person& p )
     {
          return p.gender == Gender::MALE && p.is_employed;
     } );

     employedMales = ComputeMedianAge( persons.begin(), itEmployedMale );
     unemployedMales = ComputeMedianAge( itEmployedMale, itMales );

     std::cout << "Median age = " << age << '\n';
     std::cout << "Median age for females = " << females << '\n';
     std::cout << "Median age for males = " << males << '\n';
     std::cout << "Median age for employed females = " << employedFemales << '\n';
     std::cout << "Median age for unemployed females = " << unemployedFemales << '\n';
     std::cout << "Median age for employed males = " << employedMales << '\n';
     std::cout << "Median age for unemployed males = " << unemployedMales << std::endl;
}

int main()
{
     vector< Person > persons = {
               { 31, Gender::MALE,   false },
               { 40, Gender::FEMALE, true },
               { 24, Gender::MALE,   true },
               { 20, Gender::FEMALE, true },
               { 80, Gender::FEMALE, false },
               { 78, Gender::MALE,   false },
               { 10, Gender::FEMALE, false },
               { 55, Gender::MALE,   true },
     };
     PrintStats( persons );
     return 0;
}
