#include "ini.h"

#include <istream>
#include <iostream>

namespace Ini
{

Section& Document::AddSection( std::string name )
{
     return sections_[ std::move( name )];
}

const Section& Document::GetSection( const std::string& name ) const
{
     return sections_.at( name );
}

size_t Document::SectionCount() const
{
     return sections_.size();
}

DocSections::const_iterator Document::begin() const
{
     return sections_.begin();
}

DocSections::const_iterator Document::end() const
{
     return sections_.end();
}

Document Load( std::istream& input )
{
     Document document;
     Section* currentSection = nullptr;
     for( std::string line; std::getline( input >> std::ws, line ); )
     {
          if ( line.front() == '[' && line.back() == ']' )
          {
               line.erase( 0, 1 );
               line.resize( line.size() - 1 );
               currentSection = &document.AddSection( std::move( line ));
               continue;
          }

          if( currentSection )
          {
               size_t delim = line.find( '=' );
               if( delim == std::string::npos )
               {
                    continue;
               }
               std::string key = line.substr( 0, delim );
               line.erase( 0, delim + 1 );
               currentSection->emplace( std::move( key ), std::move( line ));
          }
     }
     return document;
}

}