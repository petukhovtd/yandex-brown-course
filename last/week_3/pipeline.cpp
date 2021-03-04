#include "test_runner.h"
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace std;


struct Email
{
     string from;
     string to;
     string body;

     static unique_ptr< Email > FromStream( istream& in )
     {
          string infrom;
          string into;
          string inbody;

          getline( in, infrom );
          if( !in )
          {
               return nullptr;
          }
          getline( in, into );
          if( !in )
          {
               return nullptr;
          }
          getline( in, inbody );
          if( !in )
          {
               return nullptr;
          }

          unique_ptr< Email > ptr = make_unique< Email >();
          ptr->from = move( infrom );
          ptr->to = move( into );
          ptr->body = move( inbody );
          return ptr;
     }

     friend ostream& operator<<( ostream& out, Email const& email )
     {
          out << email.from << '\n'
              << email.to << '\n'
              << email.body << '\n';
          return out;
     }
};


class Worker
{
public:
     virtual ~Worker() = default;

     virtual void Process( unique_ptr< Email > email ) = 0;

     virtual void Run()
     {
          // только первому worker-у в пайплайне нужно это имплементировать
          throw logic_error( "Unimplemented" );
     }

protected:
     // реализации должны вызывать PassOn, чтобы передать объект дальше
     // по цепочке обработчиков
     void PassOn( unique_ptr< Email > email ) const
     {
          if( next_ )
          {
               next_->Process( move( email ) );
          }
     }

public:
     void SetNext( unique_ptr< Worker > next )
     {
          next_ = move( next );
     }

private:
     unique_ptr< Worker > next_;
};


class Reader
          : public Worker
{
public:
     explicit Reader( istream& in )
               : in_( in )
     {}

     ~Reader() override = default;

     void Process( unique_ptr< Email > email ) override
     {
          PassOn( move( email ) );
     }

     void Run() override
     {
          while( in_ )
          {
               unique_ptr< Email > email = Email::FromStream( in_ );
               if ( email )
               {
                    PassOn( move( email ) );
               }
          }
     }

private:
     istream& in_;
};


class Filter
          : public Worker
{
public:
     using Function = function< bool( const Email& ) >;

     explicit Filter( Function function )
     : function_( move(function) )
     {}

     ~Filter() override = default;

     void Process( unique_ptr< Email > email ) override
     {
          if( function_( *email ) )
          {
               PassOn( move( email ) );
          }
     }

     void Run() override
     {
          Worker::Run();
     }

public:
     Function function_;
};


class Copier
          : public Worker
{
public:
     explicit Copier( string copy_to )
     : copy_to_( move( copy_to ))
     {}

     ~Copier() override = default;

     void Process( unique_ptr< Email > email ) override
     {
          if( email->to == copy_to_ )
          {
               PassOn( move( email ) );
               return;
          }

          unique_ptr< Email > copy = make_unique< Email >();
          copy->from = email->from;
          copy->to = copy_to_;
          copy->body = email->body;
          PassOn( move( email ) );
          PassOn( move( copy ) );
     }

     void Run() override
     {
          Worker::Run();
     }

private:
     string copy_to_;
};


class Sender
          : public Worker
{
public:
     explicit Sender( ostream& out )
     : out_( out )
     {}

     ~Sender() override = default;

     void Process( unique_ptr< Email > email ) override
     {
          out_ << *email;
          PassOn( move( email ) );
     }

     void Run() override
     {
          Worker::Run();
     }

private:
     ostream& out_;
};


// реализуйте класс
class PipelineBuilder
{
public:
     // добавляет в качестве первого обработчика Reader
     explicit PipelineBuilder( istream& in )
     {
          workers_.push_back( make_unique< Reader >( in ) );
     }

     // добавляет новый обработчик Filter
     PipelineBuilder& FilterBy( Filter::Function filter )
     {
          workers_.push_back( make_unique< Filter >( move( filter ) ) );
          return *this;
     }

     // добавляет новый обработчик Copier
     PipelineBuilder& CopyTo( string recipient )
     {
          workers_.push_back( make_unique< Copier >( move( recipient ) ) );
          return *this;
     }

     // добавляет новый обработчик Sender
     PipelineBuilder& Send( ostream& out )
     {
          workers_.push_back( make_unique< Sender >( out ) );
          return *this;
     }

     // возвращает готовую цепочку обработчиков
     unique_ptr< Worker > Build()
     {
          if( workers_.empty() )
          {
               return nullptr;
          }

          while( workers_.size() != 1 )
          {
               unique_ptr< Worker > last = move( workers_.back() );
               workers_.pop_back();
               workers_.back()->SetNext( move( last ) );
          }
          return move( workers_.front() );
     }

private:
     vector< unique_ptr< Worker > > workers_;
};


void TestSanity()
{
     string input = (
               "erich@example.com\n"
               "richard@example.com\n"
               "Hello there\n"

               "erich@example.com\n"
               "ralph@example.com\n"
               "Are you sure you pressed the right button?\n"

               "ralph@example.com\n"
               "erich@example.com\n"
               "I do not make mistakes of that kind\n"
     );
     istringstream inStream( input );
     ostringstream outStream;

     PipelineBuilder builder( inStream );
     builder.FilterBy( []( const Email& email )
                       {
                            return email.from == "erich@example.com";
                       } );
     builder.CopyTo( "richard@example.com" );
     builder.Send( outStream );
     auto pipeline = builder.Build();

     pipeline->Run();

     string expectedOutput = (
               "erich@example.com\n"
               "richard@example.com\n"
               "Hello there\n"

               "erich@example.com\n"
               "ralph@example.com\n"
               "Are you sure you pressed the right button?\n"

               "erich@example.com\n"
               "richard@example.com\n"
               "Are you sure you pressed the right button?\n"
     );

     ASSERT_EQUAL( expectedOutput, outStream.str() );
}

void TestReader() {
     string input = "\n\n\n\n";
     istringstream inStream(input);
     ostringstream outStream;

     PipelineBuilder builder(inStream);
     builder.Send(outStream);
     auto pipeline = builder.Build();

     pipeline->Run();

     string expectedOutput = "\n\n\n";

     ASSERT_EQUAL(expectedOutput, outStream.str());
}

int main()
{
     TestRunner tr;
     RUN_TEST( tr, TestSanity );
     RUN_TEST( tr, TestReader );
     return 0;
}
