#include <algorithm>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <sstream>

using namespace std;


bool IsSubdomain( const string& subdomain, const string& domain )
{
     if( subdomain.size() == domain.size() )
     {
          return subdomain == domain;
     }
     if( subdomain.size() < domain.size() )
     {
          return false;
     }
     // domain.size() < subdomain.size()
     size_t subdomainIndex = 0;
     size_t domainIndex = 0;
     while( domainIndex != domain.size())
     {
          if( subdomain[ subdomainIndex++ ] != domain[ domainIndex++ ] )
          {
               return false;
          }
     }

     return subdomain[ subdomainIndex ] == '.';
}


vector< string > ReadDomains( istream& is )
{
     size_t count;
     is >> count;

     vector< string > domains;
     domains.reserve( count );
     for( size_t i = 0; i < count; ++i )
     {
          string domain;
          is >> domain;
          domains.push_back( move( domain ) );
     }
     return domains;
}


int main()
{
     const vector< string > banned_domains = []()
     {
          vector< string > banned_domains = ReadDomains( cin );
          for( string& domain : banned_domains )
          {
               reverse( begin( domain ), end( domain ) );
          }
          sort( begin( banned_domains ), end( banned_domains ) );

          size_t insert_pos = 0;
          for( string& domain : banned_domains )
          {
               if( insert_pos == 0 || !IsSubdomain( domain, banned_domains[ insert_pos - 1 ] ) )
               {
                    swap( banned_domains[ insert_pos++ ], domain );
               }
          }
          banned_domains.resize( insert_pos );

          return banned_domains;
     }();

     const vector< string > domains_to_check = []()
     {
          vector< string > domains_to_check = ReadDomains( cin );
          for( string& domain : domains_to_check )
          {
               reverse( begin( domain ), end( domain ) );
          }
          return domains_to_check;
     }();

     for( const string& domain : domains_to_check )
     {
          if( const auto it = upper_bound( begin( banned_domains ), end( banned_domains ), domain );
                    it != begin( banned_domains ) && IsSubdomain( domain, *prev( it ) ) )
          {
               cout << "Bad" << endl;
          }
          else
          {
               cout << "Good" << endl;
          }
     }
     return 0;
}
