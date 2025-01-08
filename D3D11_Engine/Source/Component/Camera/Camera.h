#pragma once
#include <Component/Base/Component.h>
#include <Core/Transform.h>

class Camera : public Component
{
public:
	static Camera* GetMainCamera() { return mainCam; }
	static void SetMainCamera(Camera* pCamear) { mainCam = pCamear; }

private:
	inline static Camera* mainCam{};

public:
	float FOV;
	float Near;
	float Far;

public:
	Camera();
	virtual ~Camera() override;

public:
	const Matrix& GetVM() const;
	const Matrix& GetIVM() const;
	const Matrix& GetPM() const;
	const Matrix& GetIPM() const;

public:
	void SetMainCamera() { mainCam = this; }
	Vector3 ScreenToWorldPoint(float screenX, float screenY, float distance);
	Ray ScreenPointToRay(int pointX, int pointY);

public:
	virtual void Start() override;
protected:
	virtual void FixedUpdate() override;
	virtual void Update() override;
	virtual void LateUpdate() override;

private:
	Matrix view;
	Matrix inversView;
	Matrix projection;
	Matrix inversProjection;
};