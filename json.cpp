#include <iomanip>
#include "json.h"

using namespace std;

namespace Json
{

Document::Document( Node root )
          : root( move( root ) )
{
}

const Node& Document::GetRoot() const
{
     return root;
}

Node LoadNode( istream& input );

Node LoadArray( istream& input )
{
     vector< Node > result;

     for( char c; input >> c && c != ']'; )
     {
          if( c != ',' )
          {
               input.putback( c );
          }
          result.push_back( LoadNode( input ) );
     }

     return Node( move( result ) );
}

Node LoadNumber( istream& input )
{
     bool negative = false;
     if( input.peek() == '-' )
     {
          negative = true;
          input.get();
     }
     int num = 0;
     while( isdigit( input.peek() ) )
     {
          num *= 10;
          num += input.get() - '0';
     }
     if( input.peek() != '.' )
     {
          return Node( num * ( negative? -1: 1 ) );
     }
     input.get();
     double result = num;
     double mul = 1;
     while( isdigit( input.peek() ) )
     {
          mul /= 10;
          result += ( input.get() - '0' ) * mul;
     }
     return Node( result * ( negative? -1: 1 ) );
}

Node LoadString( istream& input )
{
     string line;
     getline( input, line, '"' );
     return Node( move( line ) );
}

Node LoadDict( istream& input )
{
     map< string, Node > result;

     for( char c; input >> c && c != '}'; )
     {
          if( c == ',' )
          {
               input >> c;
          }

          string key = LoadString( input ).AsString();
          input >> c;
          result.emplace( move( key ), LoadNode( input ) );
     }

     return Node( move( result ) );
}

Node LoadBool( istream& input )
{
     string res;
     while( isalpha( input.peek() ) )
     {
          res.push_back( input.get() );
     }
     return Node( res == "true" );
}

Node LoadNode( istream& input )
{
     char c;
     input >> c;

     if( c == '[' )
     {
          return LoadArray( input );
     }
     else if( c == '{' )
     {
          return LoadDict( input );
     }
     else if( c == '"' )
     {
          return LoadString( input );
     }
     else if( c == 't' || c == 'f' )
     {
          input.putback( c );
          return LoadBool( input );
     }
     else if( isdigit( c ) || c == '-' )
     {
          input.putback( c );
          return LoadNumber( input );
     }
     else
     {
          input.putback( c );
          return LoadString( input );
     }
}

Document Load( istream& input )
{
     return Document { LoadNode( input ) };
}

void Save( const Document& doc, std::ostream& output )
{
     doc.GetRoot().Print( output );
}

}
