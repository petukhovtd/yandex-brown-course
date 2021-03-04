#include "Common.h"
#include "test_runner.h"

#include <sstream>

using namespace std;

class ValueExp: public Expression
{
public:
     explicit ValueExp( int value )
     : value_( value )
     {}

     ~ValueExp() override = default;

     int Evaluate() const override
     {
          return value_;
     }

     string ToString() const override
     {
          return to_string( value_ );
     }

private:
     int value_;
};

class SumExp: public Expression
{
public:
     SumExp( ExpressionPtr lhs, ExpressionPtr rhs )
     : left( move( lhs ) )
     , right( move( rhs ) )
     {}

     ~SumExp() override = default;

     int Evaluate() const override
     {
          return left->Evaluate() + right->Evaluate();
     }

     string ToString() const override
     {
          std::ostringstream ss;
          ss << '(' << left->ToString() << ")+(" << right->ToString() << ')';
          return ss.str();
     }

private:
     ExpressionPtr left;
     ExpressionPtr right;
};

class ProductExp: public Expression
{
public:
     ProductExp( ExpressionPtr lhs, ExpressionPtr rhs )
     : left( move( lhs ) )
     , right( move( rhs ) )
     {}

     ~ProductExp() override = default;

     int Evaluate() const override
     {
          return left->Evaluate() * right->Evaluate();
     }

     string ToString() const override
     {
          std::ostringstream ss;
          ss << '(' << left->ToString() << ")*(" << right->ToString() << ')';
          return ss.str();
     }

private:
     ExpressionPtr left;
     ExpressionPtr right;
};

ExpressionPtr Value( int value )
{
     return make_unique< ValueExp >( value );
}

ExpressionPtr Sum(ExpressionPtr left, ExpressionPtr right)
{
     return make_unique< SumExp >( move( left ), move( right) );
}

ExpressionPtr Product(ExpressionPtr left, ExpressionPtr right)
{
     return make_unique< ProductExp >( move( left ), move( right ) );
}

string Print(const Expression* e) {
  if (!e) {
    return "Null expression provided";
  }
  stringstream output;
  output << e->ToString() << " = " << e->Evaluate();
  return output.str();
}

void Test() {
  ExpressionPtr e1 = Product(Value(2), Sum(Value(3), Value(4)));
  ASSERT_EQUAL(Print(e1.get()), "(2)*((3)+(4)) = 14");

  ExpressionPtr e2 = Sum(move(e1), Value(5));
  ASSERT_EQUAL(Print(e2.get()), "((2)*((3)+(4)))+(5) = 19");

  ASSERT_EQUAL(Print(e1.get()), "Null expression provided");
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, Test);
  return 0;
}