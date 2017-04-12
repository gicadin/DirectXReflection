#pragma once
#ifndef __dgraphics_h__
#define __dgraphics_h__

class DGraphics
{
public:

	struct Vertex
	{
		Vertex() {}
		Vertex(float x, float y, float z,
			float nx, float ny, float nz,
			float u, float v)
		{
			_x = x;  _y = y;  _z = z;
			_nx = nx; _ny = ny; _nz = nz;
			_u = u;  _v = v;
		}
		float _x, _y, _z;
		float _nx, _ny, _nz;
		float _u, _v;

		static const DWORD FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
	};

	struct BoundingSphere
	{
		BoundingSphere();

		D3DXVECTOR3 _center;
		float       _radius;
	};

	struct Ray
	{
		D3DXVECTOR3 _origin;
		D3DXVECTOR3 _direction;
	};


	DGraphics(HWND g_hWnd, HINSTANCE g_hInstance) : hWnd(g_hWnd), hInstance(g_hInstance) { 
		ambientLight = TRUE;
		pointLight = directionalLight = spotLight = FALSE;
		init(); 
	}
	~DGraphics() { cleanD3D(); }

	void init();
	void initDirectInput();
	void renderFrame();
	void updateFrame();
	void cleanD3D();
	void loadMeshes();
	void setupView();
	void setupCamera();
	void setupLighting();
	void changeLights();

	void setupMirror();
	void renderMirror();
	void renderLeftMirror();
	void renderRightMirror();
	void renderBackMirror();

	Ray calcPickingRay(int x, int y);
	void transformRay(Ray* ray, D3DXMATRIX* T);
	bool raySphereIntTest(DGraphics::Ray* ray, DGraphics::BoundingSphere* sphere);
	void computeHitBox();

	void setSelectedTiger(int tiger) { selected_tiger = tiger; }

	void processInput(BOOL* pPressedKeys, float tmp);

	LPDIRECT3DDEVICE9 getDevice() { return g_d3ddev; }

	BoundingSphere			g_sphere;
	BoundingSphere*			g_spheres;
	ID3DXMesh*				g_msphere;

private:
	HWND					hWnd;
	HINSTANCE				hInstance;					
	LPDIRECT3D9				g_d3d;						// the pointer to our Direct3D interface
	LPDIRECT3DDEVICE9		g_d3ddev;					// the pointer to the device class
	D3DPRESENT_PARAMETERS	g_d3dpp;
	D3DDISPLAYMODE			g_displayMode;				

	IDirect3DVertexBuffer9* VB = 0;						// For drawing the scene and mirror
	
	IDirect3DTexture9*		g_mirrorTex = 0;
	D3DMATERIAL9			g_mirrorMtrl = WHITE_MTRL;
	D3DMATERIAL9			g_mirrorReflectMtrl = YELLOW_MTRL;

	DMesh					g_mesh;
	DMesh					g_meshPlant;
	DMesh					g_meshLamp;
	DMeshInstance*			g_pTigerInstance;
	DMeshInstance*			g_pPlantInstance;
	DMeshInstance*			g_pLampInstance;

	D3DLIGHT9				g_light;
	D3DLIGHT9				g_pointLight, g_directionalLight, g_spotLight;
	BOOL					ambientLight, pointLight, directionalLight, spotLight;				

	D3DMATERIAL9			g_material;

	LPDIRECTINPUT8			m_pDI;		//directinput for directx
	InputDevice				m_mouse;
	InputDevice				m_keyboard;

	Camera					m_camera;

	int						selected_tiger = 0;

	BOOL BuildPresentationParameters();

	
};

#endif