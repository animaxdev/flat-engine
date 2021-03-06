#ifndef FLAT_GEOMETRY_RECTANGLE_H
#define FLAT_GEOMETRY_RECTANGLE_H

#include "geometry/polygon.h"

namespace flat
{
namespace geometry
{

class Rectangle : public Polygon
{
	public:
		Rectangle(const Vector2& position, const Vector2& size);
		Rectangle();
		~Rectangle() override;
		
		void setPositionSize(const Vector2& position, const Vector2& size);
		
		void setSize(const Vector2& size);
		Vector2 getSize() const;
		
		void setPosition(const Vector2& position);
		const Vector2& getPosition() const;
		
		void draw(video::Attribute vertexAttribute, video::Attribute uvAttribute = 0) const;
		
		bool contains(const Vector2& v);
		
};

} // geometry
} // flat

#endif // FLAT_GEOMETRY_RECTANGLE_H


