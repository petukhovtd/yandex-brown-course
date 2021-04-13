#pragma once

#include <istream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace Json
{

class Node
          : std::variant< std::vector< Node >,
                    std::map< std::string, Node >,
                    int,
                    std::string,
                    double,
                    bool >
{
public:
     using variant::variant;

     const auto& AsArray() const
     {
          return std::get< std::vector< Node>>( *this );
     }

     const auto& AsMap() const
     {
          return std::get< std::map< std::string, Node>>( *this );
     }

     int AsInt() const
     {
          return std::get< int >( *this );
     }

     const auto& AsString() const
     {
          return std::get< std::string >( *this );
     }

     double AsDouble() const
     {
          if( auto res = std::get_if< double >( this ) )
          {
               return *res;
          }
          return std::get< int >( *this );
     }

     bool AsBool() const
     {
          return std::get< bool >( *this );
     }

     void Print( std::ostream& os ) const
     {
          if( auto res = std::get_if< std::vector< Node > >( this ))
          {
               os << '[';
               bool first = true;
               for( const Node& item: *res )
               {
                    if( first )
                    {
                         first = false;
                    }
                    else
                    {
                         os << ',';
                    }
                    item.Print( os );
               }
               os << ']';
          }
          else if( auto res = std::get_if< std::map< std::string, Node > >( this ) )
          {
               os << '{';
               bool first = true;
               for( const auto& [ key, item ]: *res )
               {
                    if( first )
                    {
                         first = false;
                    }
                    else
                    {
                         os << ',';
                    }
                    os << std::quoted( key ) << ':';
                    item.Print( os );
               }
               os << '}';
          }
          else if( auto res = std::get_if< int >( this ) )
          {
               os << *res;
          }
          else if( auto res = std::get_if< std::string >( this ) )
          {
               os << std::quoted( *res );
          }
          else if( auto res = std::get_if< double >( this ) )
          {
               os << std::setprecision( 6 ) << *res;
          }
          else if( auto res = std::get_if< bool >( this ) )
          {
               os << std::boolalpha << *res;
          }
     }
};

class Document
{
public:
     explicit Document( Node root );

     const Node& GetRoot() const;

private:
     Node root;
};

Document Load( std::istream& input );

void Save( const Document& doc, std::ostream& output );

}
