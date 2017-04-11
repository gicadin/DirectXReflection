#include "includefiles.h"
/*
	Sets the material of the model
	Sets the texture of the model

	pDevice - directx device
*/
void DMeshInstance::render(LPDIRECT3DDEVICE9 pDevice) {
	if (pDevice && m_pMesh)
	{
		pDevice->SetTransform(D3DTS_WORLD, GetTransform());
		DWORD numMaterials = m_pMesh->GetNumMaterials();
		for (DWORD i = 0; i < numMaterials; i++)
		{
			pDevice->SetMaterial(m_pMesh->GetMeshMaterial(i));
			pDevice->SetTexture(0, m_pMesh->GetMeshTexture(i));
			m_pMesh->GetMesh()->DrawSubset(i);
		}
	}
}