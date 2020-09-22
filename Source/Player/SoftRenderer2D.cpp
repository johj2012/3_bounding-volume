
#include "Precompiled.h"
#include "SoftRenderer.h"
using namespace CK::DD;

// �׸��� �׸���
void SoftRenderer::DrawGrid2D()
{
	// �׸��� ����
	LinearColor gridColor(LinearColor(0.8f, 0.8f, 0.8f, 0.3f));

	// ���� ���� ���
	Vector2 viewPos = _GameEngine.GetCamera().GetTransform().GetPosition();
	Vector2 extent = Vector2(_ScreenSize.X * 0.5f, _ScreenSize.Y * 0.5f);

	// ���� �ϴܿ������� ���� �׸���
	int xGridCount = _ScreenSize.X / _Grid2DUnit;
	int yGridCount = _ScreenSize.Y / _Grid2DUnit;

	// �׸��尡 ���۵Ǵ� ���ϴ� ��ǥ �� ���
	Vector2 minPos = viewPos - extent;
	Vector2 minGridPos = Vector2(ceilf(minPos.X / (float)_Grid2DUnit), ceilf(minPos.Y / (float)_Grid2DUnit)) * (float)_Grid2DUnit;
	ScreenPoint gridBottomLeft = ScreenPoint::ToScreenCoordinate(_ScreenSize, minGridPos - viewPos);

	for (int ix = 0; ix < xGridCount; ++ix)
	{
		_RSI->DrawFullVerticalLine(gridBottomLeft.X + ix * _Grid2DUnit, gridColor);
	}

	for (int iy = 0; iy < yGridCount; ++iy)
	{
		_RSI->DrawFullHorizontalLine(gridBottomLeft.Y - iy * _Grid2DUnit, gridColor);
	}

	// ������ ����
	ScreenPoint worldOrigin = ScreenPoint::ToScreenCoordinate(_ScreenSize, -viewPos);
	_RSI->DrawFullHorizontalLine(worldOrigin.Y, LinearColor::Red);
	_RSI->DrawFullVerticalLine(worldOrigin.X, LinearColor::Green);
}


// ���� ����
void SoftRenderer::Update2D(float InDeltaSeconds)
{
	static float moveSpeed = 100.f;

	InputManager input = _GameEngine.GetInputManager();

	// �÷��̾� ���� ������Ʈ�� Ʈ������
	Transform& playerTransform = _GameEngine.FindGameObject(GameEngine::PlayerKey).GetTransform();
	playerTransform.AddPosition(Vector2(input.GetXAxis(), input.GetYAxis()) * moveSpeed * InDeltaSeconds);

	// �÷��̾ ����ٴϴ� ī�޶��� Ʈ������
	static float thresholdDistance = 1.f;
	Transform& cameraTransform = _GameEngine.GetCamera().GetTransform();
	Vector2 playerPosition = playerTransform.GetPosition();
	Vector2 prevCameraPosition = cameraTransform.GetPosition();
	if ((playerPosition - prevCameraPosition).SizeSquared() < thresholdDistance * thresholdDistance)
	{
		cameraTransform.SetPosition(playerPosition);
	}
	else
	{
		static float lerpSpeed = 2.f;
		float ratio = lerpSpeed * InDeltaSeconds;
		ratio = Math::Clamp(ratio, 0.f, 1.f);
		Vector2 newCameraPosition = prevCameraPosition + (playerPosition - prevCameraPosition) * ratio;
		cameraTransform.SetPosition(newCameraPosition);
	}
}

// ������ ����
void SoftRenderer::Render2D()
{
	// ���� �׸���
	DrawGrid2D();

	// ī�޶��� �� ���
	Matrix3x3 viewMat = _GameEngine.GetCamera().GetViewMatrix();

	// ��ü �׸� ��ü�� ��
	size_t totalObjectCount = _GameEngine.GetGameObject().size();
	size_t circleCulledObjectCount = 0;
	size_t rectangleCulledObjectCount = 0;
	size_t renderingObjectCount = 0;

	// ī�޶��� ���� �� �ٿ��
	Circle cameraCircleBound(_GameEngine.GetCamera().GetCircleBound());
	CK::Rectangle cameraRectangleBound(_GameEngine.GetCamera().GetRectangleBound());

	// �ǵ������� ª�� ����
	cameraCircleBound.Radius = 250.f;

	// �����ϰ� ������ ��� ���� ������Ʈ��
	for (auto it = _GameEngine.GoBegin(); it != _GameEngine.GoEnd(); ++it)
	{
		GameObject& gameObject = *it->get();
		const Mesh& mesh = _GameEngine.GetMesh(gameObject.GetMeshKey());
		Transform& transform = gameObject.GetTransform();
		Matrix3x3 finalMat = viewMat * transform.GetModelingMatrix();

		size_t vertexCount = mesh._Vertices.size();
		size_t indexCount = mesh._Indices.size();
		size_t triangleCount = indexCount / 3;

		// ������Ʈ�� �� �ٿ�� ����
		Circle goCircleBound(mesh.GetCircleBound());
		CK::Rectangle goRectangleBound(mesh.GetRectangleBound());

		// �޽��� �ٿ�� ���� ������ �����ͼ� �� ��ǥ��� ��ȯ�� ���ϱ� ( �����ϵ� ������ ���� ������ ��. )
		goCircleBound.Center *= finalMat;

		goCircleBound.Radius *= transform.GetScale().Max();

		/*�簢�� ������ �� ��ǥ��� ��ȯ*/
		goRectangleBound.Min *= finalMat;
		goRectangleBound.Max *= finalMat;

		// �� �ٿ�� ������ ��ġ�� ������ �׸��⿡�� ����
		if (!cameraCircleBound.Intersect(goCircleBound))
		{
			
			if (!cameraRectangleBound.Intersect(goRectangleBound))
			{
				rectangleCulledObjectCount++;
				continue;
			}
			else
			{
				circleCulledObjectCount++;
				
			}
		}

// 		if (!cameraRectangleBound.Intersect(goRectangleBound))
// 		{
// 			rectangleCulledObjectCount++;
// 			continue;
// 		}
//  		if (!cameraCircleBound.Intersect(goCircleBound))
// 		{
// 			circleCulledObjectCount++;
// 			continue;
// 		}



		renderingObjectCount++;

		// �������� ����� ���� ���ۿ� �ε��� ���� ����
		Vector2* vertices = new Vector2[vertexCount];
		std::memcpy(vertices, &mesh._Vertices[0], sizeof(Vector2) * vertexCount);
		int* indices = new int[indexCount];
		std::memcpy(indices, &mesh._Indices[0], sizeof(int) * indexCount);

		// �� ������ ����� ����
		for (int vi = 0; vi < vertexCount; ++vi)
		{
			vertices[vi] = finalMat * vertices[vi];
		}

		// ��ȯ�� ������ �մ� �� �׸���
		for (int ti = 0; ti < triangleCount; ++ti)
		{
			int bi = ti * 3;
			_RSI->DrawLine(vertices[indices[bi]], vertices[indices[bi + 1]], gameObject.GetColor());
			_RSI->DrawLine(vertices[indices[bi]], vertices[indices[bi + 2]], gameObject.GetColor());
			_RSI->DrawLine(vertices[indices[bi + 1]], vertices[indices[bi + 2]], gameObject.GetColor());
		}

		delete[] vertices;
		delete[] indices;
	}

	_RSI->PushStatisticText("Total Objects : " + std::to_string(totalObjectCount));
	_RSI->PushStatisticText("Culled by Circle : " + std::to_string(circleCulledObjectCount));
	_RSI->PushStatisticText("Culled by Rectangle : " + std::to_string(rectangleCulledObjectCount));
	_RSI->PushStatisticText("Rendering Objects : " + std::to_string(renderingObjectCount));

}
