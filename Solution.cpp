#include <utility>
#include <unordered_map>
#include <list>
#include <set>
#include <mutex>

#include "Common.h"

using namespace std;

class LruCache
          : public ICache
{
     using Storage = list< BookPtr >;
     using Iterator = Storage::iterator;

     struct BookRank
     {
          BookPtr book;
          size_t rank;
     };

     struct NameRank
     {
          string name;
          size_t rank;

          bool operator<( const NameRank& rhs ) const
          {
               if( rank == rhs.rank )
               {
                    return name < rhs.name;
               }
               return rank < rhs.rank;
          }
     };

public:
     LruCache( shared_ptr< IBooksUnpacker > books_unpacker, const Settings& settings )
               : books_unpacker_( move( books_unpacker ) )
               , settings_( settings )
               , currentMemory_( 0 )
               , maxRank_( 0 )
     {
          // реализуйте метод
     }

     BookPtr GetBook( const string& book_name ) override
     {
          lock_guard< mutex > lock( m_ );
          auto it = byName_.find( book_name );
          if( it != byName_.end() )
          {
               if( it->second.rank == maxRank_ )
               {
                    return it->second.book;
               }
               ++maxRank_;
               size_t lastRank = it->second.rank;
               it->second.rank = maxRank_;
               ranks_.erase( { book_name, lastRank } );
               ranks_.insert( { book_name, maxRank_ } );
               return it->second.book;
          }

          unique_ptr< IBook > book = books_unpacker_->UnpackBook( book_name );
          const size_t bookSize = book->GetContent().size();

          if( bookSize > settings_.max_memory )
          {
               byName_.clear();
               ranks_.clear();
               currentMemory_ = 0;
               maxRank_ = 0;
               return BookPtr( book.release() );
          }

          BookPtr bookShared( book.release() );
          ++maxRank_;
          currentMemory_ += bookShared->GetContent().size();
          byName_[ book_name ] = { bookShared, maxRank_ };
          ranks_.insert( { book_name, maxRank_ } );

          while( !ranks_.empty() && currentMemory_ > settings_.max_memory )
          {
               auto rem = ranks_.extract( ranks_.begin() );
               auto remBook = byName_.extract( rem.value().name );
               currentMemory_ -= remBook.mapped().book->GetContent().size();
          }

          return bookShared;
     }

private:
     shared_ptr< IBooksUnpacker > books_unpacker_;
     Settings settings_;
     unordered_map< string, BookRank > byName_;
     set< NameRank > ranks_;
     size_t currentMemory_;
     size_t maxRank_;
     mutex m_;
};


unique_ptr< ICache > MakeCache(
          shared_ptr< IBooksUnpacker > books_unpacker,
          const ICache::Settings& settings
)
{
     return make_unique< LruCache >( move( books_unpacker ), settings );
}
