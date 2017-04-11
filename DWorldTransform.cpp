#include "includefiles.h"

void DWorldTransform::reset()
{
	D3DXMatrixIdentity(&m_translate);
	D3DXMatrixIdentity(&m_rotate);
	D3DXMatrixIdentity(&m_scale);
	D3DXMatrixIdentity(&m_transform);

	m_rotationX = m_rotationY = m_rotationZ = 0.0f;
}

void DWorldTransform::TranslateAbs(float x, float y, float z)
{
	m_translate._41 = x;
	m_translate._42 = y;
	m_translate._43 = z;
}

void DWorldTransform::TranslateRel(float x, float y, float z)
{
	m_translate._41 += x;
	m_translate._42 += y;
	m_translate._43 += z;
}

void DWorldTransform::RotateAbs(float x, float y, float z)
{
	m_rotationX = x;
	m_rotationY = y;
	m_rotationZ = z;
	D3DXMatrixRotationYawPitchRoll(&m_rotate, y, x, z);
}

void DWorldTransform::RotateRel(float x, float y, float z)
{
	m_rotationX += x;
	m_rotationY += y;
	m_rotationZ += z;
	D3DXMatrixRotationYawPitchRoll(&m_rotate, m_rotationY, m_rotationX, m_rotationZ);
}

void DWorldTransform::ScaleAbs(float x, float y, float z)
{
	m_scale._11 = x;
	m_scale._22 = y;
	m_scale._33 = z;
}

void DWorldTransform::ScaleRel(float x, float y, float z)
{
	m_scale._11 += x;
	m_scale._22 += y;
	m_scale._33 += z;
}

D3DXMATRIX* DWorldTransform::GetTransform()
{
	m_transform = m_scale * m_rotate * m_translate;
	return &m_transform;
}