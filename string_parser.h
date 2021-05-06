//
// Created by timofey on 4/11/21.
//

#ifndef YANDEX_BROWN_COURSE_STRING_PARSER_H
#define YANDEX_BROWN_COURSE_STRING_PARSER_H

#include <string>
#include <optional>

namespace transport
{

std::pair <std::string_view, std::optional< std::string_view >>
SplitTwoStrict( std::string_view s, std::string_view delimiter = " " );

std::pair <std::string_view, std::string_view> SplitTwo( std::string_view s, std::string_view delimiter = " " );

std::string_view ReadToken( std::string_view& s, std::string_view delimiter = " " );

std::string_view Trim( std::string_view str );

}

#endif
