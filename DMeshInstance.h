#pragma once
#ifndef DMESHINSTANCE_H
#define DMESHINSTANCE_H

class DMeshInstance : public DWorldTransform
{
public:
	DMeshInstance() : m_pMesh(NULL) {} 
	~DMeshInstance() { release(); }

	void release() { m_pMesh = NULL; }
	void setMesh(DMesh* pMesh) { 
		release(); 
		m_pMesh = pMesh;
	}
	void render(LPDIRECT3DDEVICE9 pdevice);

private:
	DMesh* m_pMesh;

};


#endif