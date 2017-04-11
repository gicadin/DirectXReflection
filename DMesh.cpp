#include "includefiles.h"

// to do later(never) to avoid mem leaks
void DMesh::release() {
	
	return;
}

/*
	loads mesh from file
	pDevice - DirectX device 
	file - .x model
*/
BOOL DMesh::load(LPDIRECT3DDEVICE9 pDevice, char* file) {

	release(); // lol
	LPD3DXBUFFER pMaterialBuffer;
	char path[128];
	getMediaFile(file, path);

	LPCWSTR strPath = convertCharArrayToLPCWSTR(path);

	HRESULT hr = D3DXLoadMeshFromX(
								strPath, 
								D3DXMESH_MANAGED, 
								pDevice, 
								NULL, 
								&pMaterialBuffer, 
								NULL, 
								&m_numMaterials, 
								&m_pMesh);
	if (FAILED(hr))
	{
		MessageBox(0, TEXT("LoadMeshFromX() – Failed"), 0, 0);
		return FALSE;
	}

	// Store material and texture information
	D3DXMATERIAL* pMaterials = (D3DXMATERIAL*)pMaterialBuffer->GetBufferPointer();

	m_pMeshMaterials = new D3DMATERIAL9[m_numMaterials];
	m_ppMeshTextures = new LPDIRECT3DTEXTURE9[m_numMaterials];

	// Copy the materials and textures from the buffer to the member arrays
	for (DWORD i = 0; i < m_numMaterials; i++)
	{
		// Copy the material
		m_pMeshMaterials[i] = pMaterials[i].MatD3D;

		// Set the ambient color for the material (D3DX does not do this)
		m_pMeshMaterials[i].Ambient = m_pMeshMaterials[i].Diffuse;
		
		// Create the texture if it exists
		m_ppMeshTextures[i] = NULL;
		
		if (pMaterials[i].pTextureFilename)
		{
			getMediaFile(pMaterials[i].pTextureFilename, path);
			strPath = convetLPSTRtoLPCWSTR(pMaterials[i].pTextureFilename);
			if (FAILED(hr = D3DXCreateTextureFromFile(pDevice, strPath , &m_ppMeshTextures[i])))
			{
				MessageBox(0, TEXT("Failed to load mesh texture"), 0, 0);
				return FALSE;
			}
		}
	}

	// Don't need this no more!
	pMaterialBuffer->Release();

	return TRUE;
}

/*
	This searches for the file and parses the file name
	file - file to search for
	path[] - returns the path of the file
*/
BOOL DMesh::getMediaFile(char* file, char path[]) {

	char exeName[MAX_PATH] = { 0 };
	char exeFolder[MAX_PATH] = { 0 };

	// Get full executable path
	GetModuleFileName(NULL, convertCharArrayToLPCWSTR(exeFolder), MAX_PATH);
	exeFolder[MAX_PATH - 1] = 0;

	// Get pointer to beginning of executable file name
	// which is after the last slash
	char* pCutPoint = NULL;
	for (int i = 0; i < MAX_PATH; i++)
	{
		if (exeFolder[i] == '\\')
		{
			pCutPoint = &exeFolder[i + 1];
		}
	}

	if (pCutPoint)
	{
		// Copy over the exe file name
		strncpy_s(exeName,128, pCutPoint, 128);

		// Chop off the exe file name from the path so we
		// just have the exe directory
		*pCutPoint = 0;

		// Get pointer to start of the .exe extension 
		pCutPoint = NULL;
		for (int i = 0; i < MAX_PATH; i++)
		{
			if (exeName[i] == '.')
			{
				pCutPoint = &exeName[i];
			}
		}
		// Chop the .exe extension from the exe name
		if (pCutPoint)
		{
			*pCutPoint = 0;
		}

		// Add a slash
		strcat_s(exeName, "\\");
	}

	// Search all the folders in searchFolders
	if (searchFolders(file, exeFolder, exeName, path))
	{
		return TRUE;
	}

	// Search all the folders in searchFolders with media\ appended to the end
	char mediaFile[MAX_PATH] = "media\\";
	strcat_s(mediaFile, file);
	if (searchFolders(mediaFile, exeFolder, exeName, path))
	{
		return TRUE;
	}

	return FALSE;
}

/*
	This does not get used or work but its here
*/
BOOL DMesh::searchFolders(char* filename, char* exeFolder, char* exeName, char fullPath[]) {

	char* searchFolders[] =
	{
		".\\", "..\\", "..\\..\\", "%s", "%s..\\", "%s..\\..\\", "%s..\\%s", "%s..\\..\\%s"
	};

	// Look through each folder to find the file
	char currentPath[MAX_PATH] = { 0 };
	for (int i = 0; i < 8; i++)
	{
		sprintf_s(currentPath, searchFolders[i], exeFolder, exeName);
		strcat_s(currentPath, filename);

		if (GetFileAttributes(convertCharArrayToLPCWSTR(currentPath)) != INVALID_FILE_ATTRIBUTES)
		{
			strcpy_s(fullPath, 128, currentPath);
			return TRUE;
		}
	}

	return FALSE;
}

/*
	Converts a char array to LPCWSTR
	charArray to convert
	returns same string but as a LPCWSTR
*/
wchar_t* DMesh::convertCharArrayToLPCWSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}

/*
Converts a LPSTR to LPCWSTR
charArray to convert
returns same string but as a LPCWSTR
*/
wchar_t* DMesh::convetLPSTRtoLPCWSTR(char* charArray) 
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}