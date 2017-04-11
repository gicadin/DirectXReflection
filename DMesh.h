#pragma once
#ifndef DMESH_H
#define DMESH_H

class DMesh {

public:
	DMesh() : m_pMesh(NULL), m_numMaterials(0), m_pMeshMaterials(NULL), m_ppMeshTextures(NULL) {}
	~DMesh() { release(); }

	BOOL load(LPDIRECT3DDEVICE9 pDevice, char* file);
	void release();

	LPD3DXMESH GetMesh() { return m_pMesh; }
	DWORD GetNumMaterials() { return m_numMaterials; }
	D3DMATERIAL9* GetMeshMaterial(int i) { return &m_pMeshMaterials[i]; }
	LPDIRECT3DTEXTURE9 GetMeshTexture(int i) { return m_ppMeshTextures[i]; }


private:

	LPD3DXMESH m_pMesh;
	DWORD m_numMaterials;
	D3DMATERIAL9 *m_pMeshMaterials;
	LPDIRECT3DTEXTURE9 *m_ppMeshTextures;

	BOOL getMediaFile(char* file, char path[]);
	BOOL searchFolders(char* filename, char* exeFolder, char* exeName, char fullPath[]);
	wchar_t* convertCharArrayToLPCWSTR(const char* charArray);
	wchar_t* convetLPSTRtoLPCWSTR(char* charArray);
};

#endif
