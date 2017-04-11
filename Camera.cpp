#include "includefiles.h"

Camera::Camera()
{
	m_maxPitch = D3DXToRadian(89.0f);
	m_maxVelocity = 1.0f;
	m_invertY = FALSE;
	m_enableYMovement = TRUE;
	m_position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_velocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_look = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	CreateProjectionMatrix(D3DX_PI / 3.0f, 1.3f, 0.1f, 1000.0f);
	Update();
}

void Camera::CreateProjectionMatrix(float fov, float aspect, float nearPlane, float farPlane)
{
	m_fov = fov;
	m_aspect = aspect;
	m_nearPlane = nearPlane;
	m_farPlane = farPlane;
	D3DXMatrixPerspectiveFovLH(&m_projection, m_fov, m_aspect, m_nearPlane, m_farPlane);
}

void Camera::MoveForward(float units)
{
	if (m_enableYMovement)
	{
		m_velocity += m_look * units;
	}
	else
	{
		D3DXVECTOR3 moveVector(m_look.x, 0.0f, m_look.z);
		D3DXVec3Normalize(&moveVector, &moveVector);
		moveVector *= units;
		m_velocity += moveVector;
	}
}

void Camera::Strafe(float units)
{
	m_velocity += m_right * units;
}

void Camera::MoveUp(float units)
{
	if (m_enableYMovement)
	{
		m_velocity.y += units;
	}
}

void Camera::Yaw(float radians)
{
	if (radians == 0.0f)
	{
		return;
	}
	D3DXMATRIX rotation;
	D3DXMatrixRotationAxis(&rotation, &m_up, radians);
	D3DXVec3TransformNormal(&m_right, &m_right, &rotation);
	D3DXVec3TransformNormal(&m_look, &m_look, &rotation);
}

void Camera::Pitch(float radians)
{
	if (radians == 0.0f)
	{
		return;
	}

	radians = (m_invertY) ? -radians : radians;
	m_pitch -= radians;
	if (m_pitch > m_maxPitch)
	{
		radians += m_pitch - m_maxPitch;
	}
	else if (m_pitch < -m_maxPitch)
	{
		radians += m_pitch + m_maxPitch;
	}

	D3DXMATRIX rotation;
	D3DXMatrixRotationAxis(&rotation, &m_right, radians);
	D3DXVec3TransformNormal(&m_up, &m_up, &rotation);
	D3DXVec3TransformNormal(&m_look, &m_look, &rotation);
}

void Camera::Roll(float radians)
{
	if (radians == 0.0f)
	{
		return;
	}
	D3DXMATRIX rotation;
	D3DXMatrixRotationAxis(&rotation, &m_look, radians);
	D3DXVec3TransformNormal(&m_right, &m_right, &rotation);
	D3DXVec3TransformNormal(&m_up, &m_up, &rotation);
}

void Camera::Update()
{
	// Cap velocity to max velocity
	if (D3DXVec3Length(&m_velocity) > m_maxVelocity)
	{
		m_velocity = *(D3DXVec3Normalize(&m_velocity, &m_velocity)) * m_maxVelocity;
	}

	// Move the camera
	m_position += m_velocity;
	// Could decelerate here. I'll just stop completely.
	m_velocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_lookAt = m_position + m_look;

	// Calculate the new view matrix
	D3DXVECTOR3 up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&m_view, &m_position, &m_lookAt, &up);

	// Set the camera axes from the view matrix
	m_right.x = m_view._11;
	m_right.y = m_view._21;
	m_right.z = m_view._31;
	m_up.x = m_view._12;
	m_up.y = m_view._22;
	m_up.z = m_view._32;
	m_look.x = m_view._13;
	m_look.y = m_view._23;
	m_look.z = m_view._33;

	// Calculate yaw and pitch
	float lookLengthOnXZ = sqrtf(m_look.z * m_look.z + m_look.x * m_look.x);
	m_pitch = atan2f(m_look.y, lookLengthOnXZ);
	m_yaw = atan2f(m_look.x, m_look.z);
}

void Camera::SetPosition(D3DXVECTOR3* pPosition)
{
	m_position.x = pPosition->x;
	m_position.y = pPosition->y;
	m_position.z = pPosition->z;
}

void Camera::SetLookAt(D3DXVECTOR3* pLookAt)
{
	m_lookAt.x = pLookAt->x;
	m_lookAt.y = pLookAt->y;
	m_lookAt.z = pLookAt->z;
	D3DXVec3Normalize(&m_look, &(m_lookAt - m_position));
}