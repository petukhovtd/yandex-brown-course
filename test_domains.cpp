#include "test_runner.h"

#include <algorithm>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

using namespace std;

template <typename It>
class Range {
public:
  Range(It begin, It end) : begin_(begin), end_(end) {}
  It begin() const { return begin_; }
  It end() const { return end_; }

private:
  It begin_;
  It end_;
};

pair<string_view, optional<string_view>> SplitTwoStrict(string_view s, string_view delimiter = " ") {
  const size_t pos = s.find(delimiter);
  if (pos == s.npos) {
    return {s, nullopt};
  } else {
    return {s.substr(0, pos), s.substr(pos + delimiter.length())};
  }
}

vector<string_view> Split(string_view s, string_view delimiter = " ") {
  vector<string_view> parts;
  if (s.empty()) {
    return parts;
  }
  while (true) {
    const auto [lhs, rhs_opt] = SplitTwoStrict(s, delimiter);
    parts.push_back(lhs);
    if (!rhs_opt) {
      break;
    }
    s = *rhs_opt;
  }
  return parts;
}


class Domain {
public:
  explicit Domain(string_view text) {
    vector<string_view> parts = Split(text, ".");
    parts_reversed_.assign(rbegin(parts), rend(parts));
  }

  size_t GetPartCount() const {
    return parts_reversed_.size();
  }

  auto GetParts() const {
    return Range(rbegin(parts_reversed_), rend(parts_reversed_));
  }
  auto GetReversedParts() const {
    return Range(begin(parts_reversed_), end(parts_reversed_));
  }

  bool operator==(const Domain& other) const {
    return parts_reversed_ == other.parts_reversed_;
  }

private:
  vector<string> parts_reversed_;
};

ostream& operator<<(ostream& stream, const Domain& domain) {
  bool first = true;
  for (const string_view part : domain.GetParts()) {
    if (!first) {
      stream << '.';
    } else {
      first = false;
    }
    stream << part;
  }
  return stream;
}

// domain is subdomain of itself
bool IsSubdomain(const Domain& subdomain, const Domain& domain) {
  const auto subdomain_reversed_parts = subdomain.GetReversedParts();
  const auto domain_reversed_parts = domain.GetReversedParts();
  return
      subdomain.GetPartCount() >= domain.GetPartCount()
      && equal(begin(domain_reversed_parts), end(domain_reversed_parts),
               begin(subdomain_reversed_parts));
}

bool IsSubOrSuperDomain(const Domain& lhs, const Domain& rhs) {
  return lhs.GetPartCount() >= rhs.GetPartCount()
         ? IsSubdomain(lhs, rhs)
         : IsSubdomain(rhs, lhs);
}


class DomainChecker {
public:
  template <typename InputIt>
  DomainChecker(InputIt domains_begin, InputIt domains_end) {
    sorted_domains_.reserve(distance(domains_begin, domains_end));
    for (const Domain& domain : Range(domains_begin, domains_end)) {
      sorted_domains_.push_back(&domain);
    }
    sort(begin(sorted_domains_), end(sorted_domains_), IsDomainLess);
    sorted_domains_ = AbsorbSubdomains(move(sorted_domains_));
  }

  // Check if candidate is subdomain of some domain
  bool IsSubdomain(const Domain& candidate) const {
    const auto it = upper_bound(
        begin(sorted_domains_), end(sorted_domains_),
        &candidate, IsDomainLess);
    if (it == begin(sorted_domains_)) {
      return false;
    }
    return ::IsSubdomain(candidate, **prev(it));
  }

private:
  vector<const Domain*> sorted_domains_;

  static bool IsDomainLess(const Domain* lhs, const Domain* rhs) {
    const auto lhs_reversed_parts = lhs->GetReversedParts();
    const auto rhs_reversed_parts = rhs->GetReversedParts();
    return lexicographical_compare(
      begin(lhs_reversed_parts), end(lhs_reversed_parts),
      begin(rhs_reversed_parts), end(rhs_reversed_parts)
    );
  }

  static vector<const Domain*> AbsorbSubdomains(vector<const Domain*> domains) {
    domains.erase(
        unique(begin(domains), end(domains),
               [](const Domain* lhs, const Domain* rhs) {
                 return IsSubOrSuperDomain(*lhs, *rhs);
               }),
        end(domains)
    );
    return domains;
  }
};


vector<Domain> ReadDomains(istream& in_stream = cin) {
  vector<Domain> domains;

  size_t count;
  in_stream >> count;
  domains.reserve(count);

  for (size_t i = 0; i < count; ++i) {
    string domain_text;
    in_stream >> domain_text;
    domains.emplace_back(domain_text);
  }
  return domains;
}

vector<bool> CheckDomains(const vector<Domain>& banned_domains, const vector<Domain>& domains_to_check) {
  const DomainChecker checker(begin(banned_domains), end(banned_domains));

  vector<bool> check_results;
  check_results.reserve(domains_to_check.size());
  for (const Domain& domain_to_check : domains_to_check) {
    check_results.push_back(!checker.IsSubdomain(domain_to_check));
  }

  return check_results;
}

void PrintCheckResults(const vector<bool>& check_results, ostream& out_stream = cout) {
  for (const bool check_result : check_results) {
    out_stream << (check_result ? "Good" : "Bad") << "\n";
  }
}

template< typename T >
bool VectorCompare( const vector< T >& lhs, const vector< T >& rhs )
{
     if( lhs.size() != rhs.size() )
     {
          return false;
     }
     for( size_t i = 0; i < lhs.size(); ++i )
     {
          if( lhs[ i ] != rhs[ i ] )
          {
               return false;
          }
     }
     return true;
}

void Test1()
{
     {
          string s = "a.b.c.d";
          auto res = Split( s, "." );
          ASSERT( VectorCompare( res, { "a", "b", "c", "d" } ) );
     }
     {
          string s = "a.b_c.d";
          auto res = Split( s, "_" );
          ASSERT( VectorCompare( res, { "a.b", "c.d" } ) );
     }

}

void Test2()
{
     string domainText = "ya.ru.com.maps";
     Domain domain( domainText );
     auto rp = domain.GetReversedParts();
     vector<string> v( rp.begin(), rp.end());
     ASSERT( VectorCompare( v, { "maps", "com", "ru", "ya" } ) );
}

void Test3()
{
     Domain baned = Domain( "ya.ru" );
     Domain check( "ya.ru" );
     ASSERT( ::IsSubdomain( check, baned ) )
}

void Test4()
{
     Domain baned = Domain( "ya.ru" );
     Domain check( "ru" );
     ASSERT( !::IsSubdomain( check, baned ) )
     ASSERT( ::IsSubdomain( baned, check ) )
}

void Test5()
{
     vector< Domain > baned = { Domain( "ya.ru" ), Domain( "a.ya.ru" ), Domain( "z.ya.ru" ) };
     DomainChecker domainChecker( baned.begin(), baned.end() );
     ASSERT( domainChecker.IsSubdomain( Domain( "k.ya.ru" ) ));
     ASSERT( VectorCompare( CheckDomains( { Domain( "ya.ru" ), Domain( "a.ya.ru" ), Domain( "z.ya.ru" ) }, { Domain( "k.ya.ru" ) } ), { false } ) )
}

void Test6()
{
     ASSERT( VectorCompare( CheckDomains( { Domain( "ya.ru" ) }, { Domain( "m.ya.ru" ) } ), { false } ) )
     vector< Domain > baned = { Domain( "com" ), Domain( "ya.ru" ) };
     DomainChecker domainChecker( baned.begin(), baned.end() );
     ASSERT( domainChecker.IsSubdomain( Domain( "ya.com" ) ));
     ASSERT( domainChecker.IsSubdomain( Domain( "ru.com" ) ));
     ASSERT( domainChecker.IsSubdomain( Domain( "ya.ru.com" ) ));
     ASSERT( !domainChecker.IsSubdomain( Domain( "com.ru" ) ));
}

void Test7()
{
     vector<bool> res = { true, false, true };
     ostringstream os;
     PrintCheckResults( res, os );
     string s = "Good\n"
                "Bad\n"
                "Good\n";
     ASSERT_EQUAL( os.str(), s );
}

void Test8()
{
     string s = "3\n"
                "ru\n"
                "ya.ru\n"
                "a.v.c";
     stringstream ss( s );
     vector< Domain > res = ReadDomains( ss );
     ASSERT_EQUAL( res.size(), 3 )
     ASSERT_EQUAL( res[ 0 ], Domain( "ru" ) )
     ASSERT_EQUAL( res[ 1 ], Domain( "ya.ru" ) )
     ASSERT_EQUAL( res[ 2 ], Domain( "a.v.c" ) )
}


int main() {
  TestRunner tr;
  RUN_TEST( tr, Test1);
  RUN_TEST( tr, Test2);
  RUN_TEST( tr, Test3);
  RUN_TEST( tr, Test4);
  RUN_TEST( tr, Test5);
  RUN_TEST(tr, Test6);
  RUN_TEST(tr, Test7);
  RUN_TEST(tr, Test8);
     return 0;
  const vector<Domain> banned_domains = ReadDomains();
  const vector<Domain> domains_to_check = ReadDomains();
  PrintCheckResults(CheckDomains(banned_domains, domains_to_check));
  return 0;
}
