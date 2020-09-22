
#include "Precompiled.h"

Circle::Circle(const std::vector<Vector2> InVertices)
{
	// 위치 정보를 받아 중심과 반지름의 값을 구하는 로직 ( 직접 구현할 것 )
// 	Center = Vector2((InVertices.begin()->X - InVertices.at(InVertices.size() / 2).X)/2, (InVertices.begin()->Y - InVertices.at(InVertices.size() / 2).Y) / 2);
// 	Radius = (Center - InVertices.front()).Size();

	Vector2 temp = Vector2::Zero;

	for (auto iter : InVertices)
	{
		temp += iter;
	}

	Center = temp / InVertices.size();

	Radius = (Center - InVertices.front()).Size();
}
