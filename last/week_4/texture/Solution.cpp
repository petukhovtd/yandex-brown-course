#include "Common.h"
#include <iostream>
using namespace std;

// Этот файл сдаётся на проверку
// Здесь напишите реализацию необходимых классов-потомков `IShape`
class ShapeBase: public IShape
{
public:
     ~ShapeBase() override = default;

     void SetPosition( Point point ) override
     {
          position_ = point;
     }

     Point GetPosition() const override
     {
          return position_;
     }

     void SetSize( Size size ) override
     {
          size_ = size;
     }

     Size GetSize() const override
     {
          return size_;
     }

     void SetTexture( std::shared_ptr< ITexture > ptr ) override
     {
          texture_ = ptr;
     }

     ITexture* GetTexture() const override
     {
          return texture_.get();
     }

protected:
     bool InTexture( Point point ) const
     {
          if( !texture_ )
          {
               return false;
          }
          return point.x < texture_->GetSize().width && point.y < texture_->GetSize().height;
     }

protected:
     Point position_;
     Size size_;
     std::shared_ptr< ITexture > texture_;
};

class ShapeRectangle: public ShapeBase
{
public:
     unique_ptr< IShape > Clone() const override
     {
          unique_ptr< IShape > ptr = make_unique< ShapeRectangle >();
          ptr->SetPosition( position_ );
          ptr->SetSize( size_ );
          ptr->SetTexture( texture_ );
          return ptr;
     }

     void Draw( Image& image ) const final
     {
          const int imageYSize = image.size();
          const int imageXSize = imageYSize == 0? 0: image.front().size();
          if( position_.y >= imageYSize && position_.x >= imageXSize )
          {
               return;
          }

          const int yEnd = min( imageYSize, position_.y + size_.height );
          const int xEnd = min( imageXSize, position_.x + size_.width );

          for( int y = position_.y; y < yEnd; ++y )
          {
               const int yShape = y - position_.y;
               for( int x = position_.x; x < xEnd; ++x )
               {
                    const int xShape = x - position_.x;
                    if( InTexture( Point{ xShape, yShape } ) )
                    {
                         image[y][x] = texture_->GetImage()[ yShape ][ xShape ];
                    }
                    else
                    {
                         image[y][x] = '.';
                    }
               }
          }
     }
};


class ShapeEllipse: public ShapeBase
{
public:
     unique_ptr< IShape > Clone() const override
     {
          unique_ptr< IShape > ptr = make_unique< ShapeEllipse >();
          ptr->SetPosition( position_ );
          ptr->SetSize( size_ );
          ptr->SetTexture( texture_ );
          return ptr;
     }

     void Draw( Image& image ) const final
     {
          const int imageYSize = image.size();
          const int imageXSize = imageYSize == 0? 0: image.front().size();
          if( position_.y >= imageYSize && position_.x >= imageXSize )
          {
               return;
          }

          const int yEnd = min( imageYSize, position_.y + size_.height );
          const int xEnd = min( imageXSize, position_.x + size_.width );

          for( int y = position_.y; y < yEnd; ++y )
          {
               const int yShape = y - position_.y;
               for( int x = position_.x; x < xEnd; ++x )
               {
                    const int xShape = x - position_.x;
                    if( !IsPointInEllipse( Point{ xShape, yShape }, size_ ) )
                    {
                         continue;
                    }
                    if( InTexture( Point{ xShape, yShape } ) )
                    {
                         image[y][x] = texture_->GetImage()[ yShape ][ xShape ];
                    }
                    else
                    {
                         image[y][x] = '.';
                    }
               }
          }
     }
};

// Напишите реализацию функции
unique_ptr<IShape> MakeShape(ShapeType shape_type) {
     switch( shape_type )
     {
          case ShapeType::Rectangle:
               return make_unique< ShapeRectangle >();
          case ShapeType::Ellipse:
               return make_unique< ShapeEllipse >();
          default:
               return nullptr;
     }
}