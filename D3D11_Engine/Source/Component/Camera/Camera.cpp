#include "Camera.h"
#include <D3DCore/D3DRenderer.h>
#include <D3DCore/D3D11_GameApp.h>
#include <Math/Mathf.h>

using namespace DirectX;

Camera::Camera()
{
	FOV = 90;
	Near = 1.f;
	Far = 3000.f;
}

Camera::~Camera()
{
	if (this == mainCam)
	{
		mainCam = nullptr;
	}
}

const Matrix& Camera::GetVM() const
{
	return view;
}

const Matrix& Camera::GetIVM() const
{
	return inversView;
}

const Matrix& Camera::GetPM() const
{
	return projection;
}

const Matrix& Camera::GetIPM() const
{
	return inversProjection;
}

Vector3 Camera::ScreenToWorldPoint(float screenX, float screenY, float distance)
{
	SIZE clientSize = D3D11_GameApp::GetClientSize();

	float mouseX = screenX;
	float mouseY = screenY;
	float screenWidth = (float)clientSize.cx;
	float screenHeight = (float)clientSize.cy;

	//마우스 좌표 NDC 변환
	float ndcX = (2.0f * mouseX) / screenWidth - 1.0f;
	float ndcY = 1.0f - (2.0f * mouseY) / screenHeight; // y축 반전 처리
	float ndcZ = (distance - Near) / (Far - Near);
	XMFLOAT3 mouseNDC(ndcX, ndcY, ndcZ);

	//월드 좌표로 변환
	XMVECTOR mouseWorld = XMVector3TransformCoord(XMLoadFloat3(&mouseNDC), inversProjection);
	mouseWorld = XMVector3TransformCoord(mouseWorld, inversView);

	return mouseWorld;
}

Ray Camera::ScreenPointToRay(int pointX, int pointY)
{
	SIZE clientSize = D3D11_GameApp::GetClientSize();

	// 화면 공간에서 마우스 클릭 위치 (윈도우 좌표)
	float mouseX = (float)pointX; // 마우스 X 좌표
	float mouseY = (float)pointY; // 마우스 Y 좌표
	float screenWidth = (float)clientSize.cx; // 화면 너비
	float screenHeight = (float)clientSize.cy; // 화면 높이

	//화면에 정의된 Viewport
	D3D11_VIEWPORT& viewPort = d3dRenderer.ViewPortsVec.front();

	// NDC로 변환 (x, y는 -1에서 1 사이 값으로 변환)
	float ndcX = (2.0f * (mouseX - viewPort.TopLeftX)) / viewPort.Width - 1.0f;
	float ndcY = 1.0f - (2.0f * (mouseY - viewPort.TopLeftY)) / viewPort.Height;  // y축 반전 처리
	float nearZ = 0.f;
	float farZ = 1.0f;
	XMFLOAT3 nearPoint(ndcX, ndcY, nearZ);
	XMFLOAT3 farPoint(ndcX, ndcY, farZ);

	//월드 좌표로 변환
	XMVECTOR nearWorld = XMVector3TransformCoord(XMLoadFloat3(&nearPoint), inversProjection);
	nearWorld = XMVector3TransformCoord(nearWorld, inversView);

	XMVECTOR farWorld = XMVector3TransformCoord(XMLoadFloat3(&farPoint), inversProjection);
	farWorld = XMVector3TransformCoord(farWorld, inversView);

	// 원점과 방향 벡터 계산
	FXMVECTOR Origin = nearWorld;
	FXMVECTOR Direction = XMVector3Normalize(XMVectorSubtract(farWorld, nearWorld));

	Ray ray(Origin, Direction);
	return ray;
}

void Camera::Start()
{

}

void Camera::FixedUpdate()
{

}

void Camera::Update()
{
	if (mainCam == this)
	{
		view = XMMatrixLookToLH(transform.position, transform.Forward, transform.Up);
		inversView = XMMatrixInverse(nullptr, view);

		D3D11_VIEWPORT& mainViewPort = d3dRenderer.ViewPortsVec[0];
		projection = XMMatrixPerspectiveFovLH(FOV * Mathf::Deg2Rad, mainViewPort.Width / mainViewPort.Height, Near, Far);
		inversProjection = XMMatrixInverse(nullptr, projection);
	}
}

void Camera::LateUpdate()
{

}



