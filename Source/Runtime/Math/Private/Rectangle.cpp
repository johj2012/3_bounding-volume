#include "Precompiled.h"

Rectangle::Rectangle(const std::vector<Vector2> InVertices)
{
	float temp = InVertices.begin()->X + InVertices.begin()->Y;
	float temp2 = 0.f;
	Min = *InVertices.begin();
	Max = *InVertices.begin();
	for (auto iter = InVertices.begin() + 1; iter < InVertices.end(); iter++)
	{
		temp2 = iter->X + iter->Y;
		if (temp < temp2)
			Max = *iter;
		else
			Min = *iter;
	}
}