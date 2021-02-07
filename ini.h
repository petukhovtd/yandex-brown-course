#ifndef YANDEX_BROWN_COURSE_INI_H
#define YANDEX_BROWN_COURSE_INI_H

#include <string>
#include <unordered_map>

namespace Ini
{

using Section = std::unordered_map< std::string, std::string >;
using DocSections = std::unordered_map< std::string, Section >;

class Document
{
public:
     Section& AddSection( std::string name );

     const Section& GetSection( const std::string& name ) const;

     size_t SectionCount() const;

     DocSections::const_iterator begin() const;

     DocSections::const_iterator end() const;

private:
     std::unordered_map< std::string, Section > sections_;
};

Document Load( std::istream& input );

}

#endif
