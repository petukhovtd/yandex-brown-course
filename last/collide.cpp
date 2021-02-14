#include "geo2d.h"
#include "game_object.h"

#include "test_runner.h"

#include <vector>
#include <memory>

using namespace std;

template< typename T >
class Collader: public GameObject
{
public:
     bool Collide( const GameObject& that ) const override
     {
          return that.CollideWith( static_cast< const T& >( *this ) );
     }
};

#define COLLIDE_WITH_DEFENISION( type ) \
bool CollideWith( const type& that ) const override;

#define COLLIDE_WITH_DECLARATION( type, typeWith ) \
bool type::CollideWith( const typeWith& that ) const \
{ \
     return geo2d::Collide( pos_, that.GetPos() ); \
}

#define CLASS_DEF( Type, PosType ) \
class Type: public Collader< Type > \
{ \
public: \
     explicit Type( geo2d::PosType position ); \
     ~Type() override = default; \
     COLLIDE_WITH_DEFENISION( Unit ) \
     COLLIDE_WITH_DEFENISION( Building ) \
     COLLIDE_WITH_DEFENISION( Tower ) \
     COLLIDE_WITH_DEFENISION( Fence ) \
     const geo2d::PosType& GetPos() const; \
private: \
     geo2d::PosType pos_; \
};

#define CLASS_DEC( Type, PosType ) \
Type::Type( geo2d::PosType position ) \
          : pos_( position ) \
{} \
COLLIDE_WITH_DECLARATION( Type, Unit ) \
COLLIDE_WITH_DECLARATION( Type, Building ) \
COLLIDE_WITH_DECLARATION( Type, Tower ) \
COLLIDE_WITH_DECLARATION( Type, Fence ) \
const geo2d::PosType& Type::GetPos() const \
{ \
      return pos_; \
}

CLASS_DEF( Unit, Point )
CLASS_DEF( Building, Rectangle )
CLASS_DEF( Tower, Circle )
CLASS_DEF( Fence, Segment )

CLASS_DEC( Unit, Point )
CLASS_DEC( Building, Rectangle )
CLASS_DEC( Tower, Circle )
CLASS_DEC( Fence, Segment )

// Реализуйте функцию Collide из файла GameObject.h

bool Collide( const GameObject& first, const GameObject& second )
{
     return first.Collide( second );
}

void TestAddingNewObjectOnMap()
{
     // Юнит-тест моделирует ситуацию, когда на игровой карте уже есть какие-то объекты,
     // и мы хотим добавить на неё новый, например, построить новое здание или башню.
     // Мы можем его добавить, только если он не пересекается ни с одним из существующих.
     using namespace geo2d;

     const vector< shared_ptr< GameObject>> game_map = {
               make_shared< Unit >( Point { 3, 3 } ),
               make_shared< Unit >( Point { 5, 5 } ),
               make_shared< Unit >( Point { 3, 7 } ),
               make_shared< Fence >( Segment { { 7, 3 },
                                               { 9, 8 } } ),
               make_shared< Tower >( Circle { Point { 9, 4 }, 1 } ),
               make_shared< Tower >( Circle { Point { 10, 7 }, 1 } ),
               make_shared< Building >( Rectangle { { 11, 4 },
                                                    { 14, 6 } } )
     };

     for( size_t i = 0; i < game_map.size(); ++i )
     {
          Assert(
                    Collide( *game_map[ i ], *game_map[ i ] ),
                    "An object doesn't collide with itself: " + to_string( i )
          );

          for( size_t j = 0; j < i; ++j )
          {
               Assert(
                         !Collide( *game_map[ i ], *game_map[ j ] ),
                         "Unexpected collision found " + to_string( i ) + ' ' + to_string( j )
               );
          }
     }

     auto new_warehouse = make_shared< Building >( Rectangle { { 4, 3 },
                                                               { 9, 6 } } );
     ASSERT( !Collide( *new_warehouse, *game_map[ 0 ] ) );
     ASSERT( Collide( *new_warehouse, *game_map[ 1 ] ) );
     ASSERT( !Collide( *new_warehouse, *game_map[ 2 ] ) );
     ASSERT( Collide( *new_warehouse, *game_map[ 3 ] ) );
     ASSERT( Collide( *new_warehouse, *game_map[ 4 ] ) );
     ASSERT( !Collide( *new_warehouse, *game_map[ 5 ] ) );
     ASSERT( !Collide( *new_warehouse, *game_map[ 6 ] ) );

     auto new_defense_tower = make_shared< Tower >( Circle { { 8, 2 }, 2 } );
     ASSERT( !Collide( *new_defense_tower, *game_map[ 0 ] ) );
     ASSERT( !Collide( *new_defense_tower, *game_map[ 1 ] ) );
     ASSERT( !Collide( *new_defense_tower, *game_map[ 2 ] ) );
     ASSERT( Collide( *new_defense_tower, *game_map[ 3 ] ) );
     ASSERT( Collide( *new_defense_tower, *game_map[ 4 ] ) );
     ASSERT( !Collide( *new_defense_tower, *game_map[ 5 ] ) );
     ASSERT( !Collide( *new_defense_tower, *game_map[ 6 ] ) );
}

int main()
{
     TestRunner tr;
     RUN_TEST( tr, TestAddingNewObjectOnMap );
     return 0;
}
