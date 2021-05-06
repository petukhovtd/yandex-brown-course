#include "string_parser.h"

namespace transport
{

std::pair <std::string_view, std::optional< std::string_view >>
SplitTwoStrict( std::string_view s, std::string_view delimiter )
{
     const size_t pos = s.find( delimiter );
     if( pos == s.npos )
     {
          return { s, std::nullopt };
     }
     else
     {
          return { s.substr( 0, pos ), s.substr( pos + delimiter.length() ) };
     }
}

std::pair <std::string_view, std::string_view> SplitTwo( std::string_view s, std::string_view delimiter )
{
     const auto[lhs, rhs_opt] = SplitTwoStrict( s, delimiter );
     return { lhs, rhs_opt.value_or( "" ) };
}

std::string_view ReadToken( std::string_view& s, std::string_view delimiter )
{
     const auto[lhs, rhs] = SplitTwo( s, delimiter );
     s = rhs;
     return lhs;
}

std::string_view Trim( std::string_view str )
{
     str.remove_prefix( std::min( str.find_first_not_of( " \t\r\v\n" ), str.size() ) );
     str.remove_suffix( std::min( str.size() - str.find_last_not_of( " \t\r\v\n" ) - 1, str.size() ) );
     return str;
}

}