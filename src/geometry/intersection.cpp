#include "intersection.h"

namespace flat
{
namespace geometry
{
namespace intersection
{

float rectangleToRectangleDistance(const flat::AABB2& a, const flat::AABB2& b, flat::Vector2* direction)
{
	flat::Vector2 aClosestPoint;
	flat::Vector2 bClosestPoint;

	if (a.max.x < b.min.x)
	{
		if (a.max.y < b.min.y)
		{
			aClosestPoint = a.max;
			bClosestPoint = b.min;
		}
		else if (a.min.y > b.max.y)
		{
			aClosestPoint = flat::Vector2(a.max.x, a.min.y);
			bClosestPoint = flat::Vector2(b.min.x, b.max.y);
		}
		else
		{
			// not really the closest points but same distance and directions
			aClosestPoint = a.max;
			bClosestPoint = flat::Vector2(b.min.x, a.max.y);
		}
	}
	else if (a.min.x > b.max.x)
	{
		if (a.max.y < b.min.y)
		{
			aClosestPoint = flat::Vector2(a.min.x, a.max.y);
			bClosestPoint = flat::Vector2(b.max.x, b.min.y);
		}
		else if (a.min.y > b.max.y)
		{
			aClosestPoint = a.min;
			bClosestPoint = b.max;
		}
		else
		{
			// not really the closest points but same distance and directions
			aClosestPoint = a.min;
			bClosestPoint = flat::Vector2(b.max.x, a.min.y);
		}
	}
	else
	{
		if (a.max.y < b.min.y)
		{
			// not really the closest points but same distance and directions
			aClosestPoint = a.max;
			bClosestPoint = flat::Vector2(a.max.x, b.min.y);
		}
		else if (a.min.y > b.max.y)
		{
			// not really the closest points but same distance and directions
			aClosestPoint = a.min;
			bClosestPoint = flat::Vector2(a.min.x, b.max.y);
		}
		else
		{
			// rectangles overlap: return 0 and 0-length direction
			if (direction != nullptr)
			{
				*direction = flat::Vector2(0.f, 0.f);
			}
			return 0.f;
		}
	}

	const flat::Vector2 difference = bClosestPoint - aClosestPoint;
	if (direction != nullptr)
	{
		*direction = flat::normalize(difference);
	}
	return flat::length(difference);
}

flat::Vector2 closestPointOnRectangle(const flat::AABB2& rectangle, const flat::Vector2& point)
{
	if (point.x < rectangle.min.x)
	{
		if (point.y < rectangle.min.y)
		{
			return rectangle.min;
		}
		else if (point.y > rectangle.max.y)
		{
			return flat::Vector2(rectangle.min.x, rectangle.max.y);
		}
		else
		{
			return flat::Vector2(rectangle.min.x, point.y);
		}
	}
	else if (point.x > rectangle.max.x)
	{
		if (point.y < rectangle.min.y)
		{
			return flat::Vector2(rectangle.max.x, rectangle.min.y);
		}
		else if (point.y > rectangle.max.y)
		{
			return rectangle.max;
		}
		else
		{
			return flat::Vector2(rectangle.max.x, point.y);
		}
	}
	else
	{
		if (point.y < rectangle.min.y)
		{
			return flat::Vector2(point.x, rectangle.min.y);
		}
		else if (point.y > rectangle.max.y)
		{
			return flat::Vector2(point.x, rectangle.max.y);
		}
		else
		{
			return point;
		}
	}
}

float circleToRectangleDistance(const flat::AABB2& rectangle, const flat::Vector2& circleCenter, float circleRadius, flat::Vector2* direction)
{
	const flat::Vector2 closestPoint = closestPointOnRectangle(rectangle, circleCenter);
	const flat::Vector2 difference = circleCenter - closestPoint;
	if (direction != nullptr)
	{
		*direction = flat::normalize(difference);
	}
	return flat::length(difference) - circleRadius;
}

} // intersection
} // geometry
} // flat
