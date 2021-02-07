#include "test_runner.h"

#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <memory>
#include <utility>

using namespace std;

struct Record {
     string id;
     string title;
     string user;
     int timestamp;
     int karma;
};

class Database
{
     using RecordPtr = std::shared_ptr< Record >;
     using UserToId = std::multimap< std::string, RecordPtr >;
     using KarmaToId = std::multimap< int, RecordPtr >;
     using TimeStampToId = std::multimap< int, RecordPtr >;

     struct RecordIndexer
     {
          RecordPtr record;
          UserToId::iterator userIt;
          KarmaToId::iterator karmaIt;
          TimeStampToId::iterator timeStampIt;

          explicit RecordIndexer( RecordPtr in )
          : record( std::move( in ))
          {}
     };

     using MainStorage = std::unordered_map< std::string , RecordIndexer >;

public:
     bool Put( const Record& record )
     {
          RecordPtr recordPtr = std::make_shared< RecordPtr::element_type >( record );
          std::string id = record.id;
          auto [it, res] = mainStorage_.emplace( id, RecordIndexer( recordPtr ) );
          if( res )
          {
               it->second.timeStampIt = timeStampToId_.emplace( recordPtr->timestamp, recordPtr );
               it->second.karmaIt = karmaToId_.emplace( recordPtr->karma, recordPtr );
               it->second.userIt = userToId_.emplace( recordPtr->user, recordPtr );
          }
          return res;
     }

     const Record* GetById( const string& id ) const
     {
          auto it = mainStorage_.find( id );
          if( it == mainStorage_.end() )
          {
               return nullptr;
          }
          return it->second.record.get();
     }

     bool Erase( const string& id )
     {
          auto it = mainStorage_.find( id );
          if( it == mainStorage_.end() )
          {
               return false;
          }
          const RecordIndexer& recordIndexer = it->second;
          timeStampToId_.erase( recordIndexer.timeStampIt );
          karmaToId_.erase( recordIndexer.karmaIt );
          userToId_.erase( recordIndexer.userIt );
          mainStorage_.erase( it );
          return true;
     }

     template< typename Callback >
     void RangeByTimestamp( int low, int high, Callback callback ) const
     {
          auto from = timeStampToId_.lower_bound( low );
          auto to = timeStampToId_.upper_bound( high );
          for( ; from != to; ++from )
          {
               if( !callback( *from->second ) )
               {
                    break;
               }
          }
     }

     template< typename Callback >
     void RangeByKarma( int low, int high, Callback callback ) const
     {
          auto from = karmaToId_.lower_bound( low );
          auto to = karmaToId_.upper_bound( high );
          for( ; from != to; ++from )
          {
               if( !callback( *from->second ) )
               {
                    break;
               }
          }
     }

     template <typename Callback>
     void AllByUser(const string& user, Callback callback) const
     {
          auto [ from, to ] = userToId_.equal_range( user );
          for( ; from != to; ++from )
          {
               if( !callback( *from->second ) )
               {
                    break;
               }
          }
     }

private:
     MainStorage mainStorage_;
     TimeStampToId timeStampToId_;
     KarmaToId karmaToId_;
     UserToId userToId_;
};

void TestRangeBoundaries() {
     const int good_karma = 1000;
     const int bad_karma = -10;

     Database db;
     db.Put({"id1", "Hello there", "master", 1536107260, good_karma});
     db.Put({"id2", "O>>-<", "general2", 1536107260, bad_karma});

     int count = 0;
     db.RangeByKarma(bad_karma, good_karma, [&count](const Record&) {
          ++count;
          return true;
     });

     ASSERT_EQUAL(2, count);
}

void TestSameUser() {
     Database db;
     db.Put({"id1", "Don't sell", "master", 1536107260, 1000});
     db.Put({"id2", "Rethink life", "master", 1536107260, 2000});
     db.Put({"id3", "sfsdfdsf", "master", 1536107262, 3000});
     db.Erase( "id3" );

     int count = 0;
     db.AllByUser("master", [&count](const Record&) {
          ++count;
          return true;
     });

     ASSERT_EQUAL(2, count);
}

void TestReplacement() {
     const string final_body = "Feeling sad";

     Database db;
     db.Put({"id", "Have a hand", "not-master", 1536107260, 10});
     db.Erase("id");
     db.Put({"id", final_body, "not-master", 1536107260, -10});

     auto record = db.GetById("id");
     ASSERT(record != nullptr);
     ASSERT_EQUAL(final_body, record->title);
}

int main() {
     TestRunner tr;
     RUN_TEST(tr, TestRangeBoundaries);
     RUN_TEST(tr, TestSameUser);
     RUN_TEST(tr, TestReplacement);
     return 0;
}