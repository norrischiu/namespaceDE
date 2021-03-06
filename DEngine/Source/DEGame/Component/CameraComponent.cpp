// CameraComponent.cpp
#include <DEGame/DEGame.h>
#include "CameraComponent.h"

// Engine include

namespace DE
{

CameraComponent::CameraComponent(const Vector3& vPos, const Vector3& vLookAt, const Vector3& vUp, const float fFov, const float fRatio, const float fZNear, const float fZFar)
	: Component(nullptr)
	, m_vPos(vPos)
	, m_vLookAt(vLookAt)
	, m_vUp(vUp)
{
	m_mPerspectiveProj = Matrix4::PerspectiveProjection(fFov, fRatio, fZNear, fZFar);
	m_Frustum = Frustum(fFov, fRatio, fZNear, fZFar);
	m_ID = ComponentID;
}

Matrix4 CameraComponent::GetViewMatrix()
{
	Vector3 pos = m_vPos, lookat = m_vLookAt;
	//pos.Transform(*m_pOwner->GetTransform());
	//lookat.Transform(*m_pOwner->GetTransform());
	return Matrix4::LookAtMatrix(pos, lookat, m_vUp);
}

Matrix4 CameraComponent::GetPerspectiveMatrix()
{
	return m_mPerspectiveProj;
}

Matrix4 CameraComponent::GetPVMatrix()
{
	return m_mPerspectiveProj * GetViewMatrix();
}

void CameraComponent::Update(float deltaTime)
{
}

};