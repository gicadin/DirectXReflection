#include "includefiles.h"

/*
	This initializes directX 
*/
void DGraphics::init()
{
	// Creates a directX context
	g_d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (!g_d3d) {
		MessageBox(0, TEXT("Direct3DCreate9() – Failed"), 0, 0);
		return;
	}
	// Get display mode
	g_d3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &g_displayMode);

	/* 
		This is directX way of checking whether directX can use a pure device on ur graphics card
		Otherwise it uses directX software to replicate hardware support
		This is proper initialization 
	*/
	// Check for hardware T & L
	D3DCAPS9 D3DCaps;
	g_d3d->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &D3DCaps);

	DWORD vertexProcessing = 0;
	if (D3DCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) {
		vertexProcessing = D3DCREATE_HARDWARE_VERTEXPROCESSING;
		// Check for pure device 
		if (D3DCaps.DevCaps & D3DDEVCAPS_PUREDEVICE) {
			vertexProcessing |= D3DCREATE_PUREDEVICE;
		}
	}
	else {
		vertexProcessing = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

	// Populate directX device struct
	if (!BuildPresentationParameters()) {
		MessageBox(0, TEXT("Unable to find a valid DepthStencil Format"), 0, 0);
		return ;
	}

	// Create the device 
	if (FAILED(g_d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
		hWnd, vertexProcessing, &g_d3dpp, &g_d3ddev))) {

		MessageBox(0, TEXT("CreateDevice() – Failed"), 0, 0);
		return;
	}

	// Turn on the zbuffer
	g_d3ddev->SetRenderState(D3DRS_ZENABLE, TRUE);


	setupMirror();		// Do mirror initialization
	setupCamera();		// Do camera initialization
	setupLighting();	// Do lightning initialization
}

// Initialize directX keyboard input 
void DGraphics::initDirectInput() 
{
	m_pDI = NULL;
	// Initialize DirectInput
	if (FAILED(DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_pDI, NULL)))
	{
		MessageBox(0, TEXT("DirectInput8() - Failed"), 0, 0);
		return ;
	}
	// Initialize keyboard input with direct input
	if (!m_keyboard.init(m_pDI, hWnd, DIT_KEYBOARD))
	{
		MessageBox(0, TEXT("Keyboard Issues - Failed"), 0, 0);
		//return;
	}

}

// Process current frame for rendering
void DGraphics::renderFrame() 
{
	// Create the camera view
	g_d3ddev->SetTransform(D3DTS_VIEW, m_camera.GetViewMatrix());

	// clear the window to a deep grey
	g_d3ddev->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, D3DCOLOR_XRGB(100, 100, 100), 1.0f, 0);

	g_d3ddev->BeginScene();    // begins the 3D scene

	updateFrame();

	D3DXMATRIX I;
	D3DXMatrixIdentity(&I);
	g_d3ddev->SetTransform(D3DTS_WORLD, &I);

	g_d3ddev->SetStreamSource(0, VB, 0, sizeof(Vertex));
	g_d3ddev->SetFVF(DGraphics::Vertex::FVF);

	// draw the mirror
	g_d3ddev->SetMaterial(&g_mirrorMtrl);
	g_d3ddev->SetTexture(0, g_mirrorTex);
	g_d3ddev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 8);

	// do 3D rendering on the back buffer here
	// render the models
	g_pTigerInstance[0].render(g_d3ddev);
	g_pTigerInstance[1].render(g_d3ddev);
	g_pPlantInstance[0].render(g_d3ddev);
	g_pPlantInstance[1].render(g_d3ddev);
	g_pLampInstance[0].render(g_d3ddev);

	computeHitBox();

	// Render the mirror
	renderMirror();
	renderLeftMirror();
	renderRightMirror();
	renderBackMirror();

	g_d3ddev->EndScene();    // ends the 3D scene

	g_d3ddev->Present(NULL, NULL, NULL, NULL);   // displays the created frame on the screen
}

/*
	Recalculates each frame
	-Does came updates
	-Reads keyboard input
	-Processes keyboard
*/
void DGraphics::updateFrame()
{
	
	m_camera.Update();

	m_keyboard.read();
	
	BOOL* pPressedKeys = m_keyboard.GetKeys();

	processInput(pPressedKeys, 0.0001);

}

/*
	Clean up resources after ourselves as good people
*/
void DGraphics::cleanD3D()
{
	g_d3ddev->Release();    // close and release the 3D device
	g_d3d->Release();    // close and release Direct3D
}

/*
	Build DirectX device struct parameters 
*/
BOOL DGraphics::BuildPresentationParameters() 
{
	ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));    // clear out the struct for use

	D3DFORMAT adapterFormat = (WINDOWED) ? g_displayMode.Format : D3DFMT_X8R8G8B8;
	if (SUCCEEDED(g_d3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, adapterFormat,
		D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24S8))) {
		g_d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	}
	else if (SUCCEEDED(g_d3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, adapterFormat,
		D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24X8))) {
		g_d3dpp.AutoDepthStencilFormat = D3DFMT_D24X8;
	}
	else if (SUCCEEDED(g_d3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, adapterFormat,
		D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D16))) {
		g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	}
	else {
		return false;
	}

	g_d3dpp.BackBufferWidth            = (WINDOWED) ? 0 : g_displayMode.Width;
	g_d3dpp.BackBufferHeight           = (WINDOWED) ? 0 : g_displayMode.Height;
	g_d3dpp.BackBufferFormat           = adapterFormat;
	g_d3dpp.BackBufferCount            = 1;
	g_d3dpp.MultiSampleType            = D3DMULTISAMPLE_NONE;
	g_d3dpp.MultiSampleQuality         = 0;
	g_d3dpp.SwapEffect                 = D3DSWAPEFFECT_DISCARD;
	g_d3dpp.hDeviceWindow              = hWnd;
	g_d3dpp.Windowed                   = WINDOWED;
	g_d3dpp.EnableAutoDepthStencil     = TRUE;
	g_d3dpp.FullScreen_RefreshRateInHz = (WINDOWED) ? 0 : g_displayMode.RefreshRate;
	g_d3dpp.PresentationInterval       = D3DPRESENT_INTERVAL_IMMEDIATE;

	return TRUE;
}

/*
	Loads meshes from file (needs to be .x)
	Sets the mesh to the instance
*/
void DGraphics::loadMeshes() {
	g_mesh.load(g_d3ddev, "tiger.x");
	
	// If the tiger does get initialized properly release it
	if (g_pTigerInstance)
		for (int i = 0; i < 2; i++) 
			g_pTigerInstance[i].release();
		
	g_pTigerInstance = new DMeshInstance[2];
	
	g_pTigerInstance[0].setMesh(&g_mesh); 

	g_pTigerInstance[0].SetXPosition(5.0f);
	g_pTigerInstance[0].SetYPosition(0.0f);
	g_pTigerInstance[0].ScaleAbs(3.0f, 3.0f, 3.0f);

	g_pTigerInstance[1].setMesh(&g_mesh);

	g_pTigerInstance[1].SetXPosition(-8.0f);
	g_pTigerInstance[1].SetYPosition(0.0f);
	g_pTigerInstance[1].SetZPosition(24.0f);
	g_pTigerInstance[1].ScaleAbs(3.0f, 3.0f, 3.0f);

	g_meshPlant.load(g_d3ddev, "plant.x");

	if (g_pPlantInstance)
		for (int i = 0; i < 1; i++)
			g_pPlantInstance[i].release();

	g_pPlantInstance = new DMeshInstance[2];

	g_pPlantInstance[0].setMesh(&g_meshPlant);

	g_pPlantInstance[0].SetXPosition(-7.0f);
	g_pPlantInstance[0].SetYPosition(0.0f);
	g_pPlantInstance[0].SetZPosition(2.5f);
	g_pPlantInstance[0].ScaleAbs(3.0f, 3.0f, 3.0f);

	g_pPlantInstance[1].setMesh(&g_meshPlant);

	g_pPlantInstance[1].SetXPosition(9.0f);
	g_pPlantInstance[1].SetYPosition(0.0f);
	g_pPlantInstance[1].SetZPosition(4.0f);
	g_pPlantInstance[1].ScaleAbs(3.0f, 3.0f, 3.0f);

	g_meshLamp.load(g_d3ddev, "lamp.x");

	if (g_pLampInstance)
		for (int i = 0; i < 1; i++)
			g_pLampInstance[i].release();

	g_pLampInstance = new DMeshInstance[1];

	g_pLampInstance[0].setMesh(&g_meshLamp);

	g_pLampInstance[0].SetXPosition(0.0f);
	g_pLampInstance[0].SetYPosition(0.0f);
	g_pLampInstance[0].SetZPosition(0.0f);
	g_pLampInstance[0].ScaleAbs(1.0f, 1.0f, 1.0f);

	/* Picking stuff */

	g_spheres = new BoundingSphere[2];

	BYTE* v = 0;
	g_mesh.GetMesh()->LockVertexBuffer(0, (void**)&v);

	D3DXComputeBoundingSphere(
		(D3DXVECTOR3*)v,
		g_mesh.GetMesh()->GetNumVertices(),
		D3DXGetFVFVertexSize(g_mesh.GetMesh()->GetFVF()),
		&g_sphere._center,
		&g_sphere._radius);

	g_mesh.GetMesh()->UnlockVertexBuffer();

	D3DXCreateSphere(g_d3ddev, g_sphere._radius, 20, 20, &g_msphere, 0);

}

/*
	THIS FUNCTION DOES NOTHING JUST HOLDS OLD CODE
*/
void DGraphics::setupView() {
	/*
	// For our world matrix, we will just leave it as the identity
	D3DXMATRIXA16 matWorld;
	//D3DXMatrixRotationY(&matWorld, timeGetTime() / 1000.0f);
	g_d3ddev->SetTransform(D3DTS_WORLD, &matWorld);

	// Set up our view matrix. A view matrix can be defined given an eye point,
	// a point to lookat, and a direction for which way is up. Here, we set the
	// eye five units back along the z-axis and up three units, look at the 
	// origin, and define "up" to be in the y-direction.
	D3DXVECTOR3 vEyePt(0.0f, 3.0f, -5.0f);
	D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
	D3DXMATRIXA16 matView;
	D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
	g_d3ddev->SetTransform(D3DTS_VIEW, &matView);

	// For the projection matrix, we set up a perspective transform (which
	// transforms geometry from 3D view space to 2D viewport space, with
	// a perspective divide making objects smaller in the distance). To build
	// a perpsective transform, we need the field of view (1/4 pi is common),
	// the aspect ratio, and the near and far clipping planes (which define at
	// what distances geometry should be no longer be rendered).
	D3DXMATRIXA16 matProj;
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f);
	g_d3ddev->SetTransform(D3DTS_PROJECTION, &matProj);
	*/
	/*
	// Set transforms
	D3DXVECTOR3 cameraPosition(0.0f, 8.0f, -15.0f);
	D3DXVECTOR3 cameraTarget(0.0f, 0.0f, 5.0f);
	D3DXVECTOR3 cameraUp(0.0f, 1.0f, 0.0f);
	D3DXMATRIX viewMatrix;
	D3DXMatrixLookAtLH(&viewMatrix, &cameraPosition, &cameraTarget, &cameraUp);
	g_d3ddev->SetTransform(D3DTS_VIEW, &viewMatrix);
	D3DXMATRIX projection;
	float aspect = (float)1.33337;
	D3DXMatrixPerspectiveFovLH(&projection, D3DX_PI / 3.0f, aspect, 0.1f, 1000.0f);
	g_d3ddev->SetTransform(D3DTS_PROJECTION, &projection);
	*/
	
}

/*
	This initializes the directX camera instance
*/
void DGraphics::setupCamera()
{
	m_camera.SetMaxVelocity(100.0f);
	m_camera.SetPosition(new D3DXVECTOR3(0.0f, 3.0f, -15.0f));
	m_camera.SetLookAt(new D3DXVECTOR3(0.0f, 3.0f, 0.0f));
	m_camera.Update();
	g_d3ddev->SetTransform(D3DTS_PROJECTION, m_camera.GetProjectionMatrix());

	return;
}

/*
	Initializes 3 lights
	-Direct Light from the top right
	-Spot Light from the left
	-Point Light at the lamp post
*/
void DGraphics::setupLighting() {

	// Turn on ambient lighting 
	//g_d3ddev->SetRenderState(D3DRS_AMBIENT, 0xffffffff);

	// Set render states
	/*

	g_d3ddev->LightEnable(0, TRUE);
	g_d3ddev->SetLight(0, &g_light);
	*/

	ZeroMemory(&g_pointLight, sizeof(g_pointLight));
	g_pointLight.Type = D3DLIGHT_POINT;    // make the light type 'point light'
	g_pointLight.Diffuse = D3DXCOLOR(1.5f, 1.5f, 1.5f, 1.0f);
	g_pointLight.Position = D3DXVECTOR3(-2.0f, 5.0f, 3.0f);
	g_pointLight.Range = 100.0f;    // a range of 100
	g_pointLight.Attenuation0 = 0.0f;    // no constant inverse attenuation
	g_pointLight.Attenuation1 = 0.0525f;    // only .125 inverse attenuation
	g_pointLight.Attenuation2 = 0.0f;    // no square inverse attenuation

	/*
	ZeroMemory(&g_spotLight, sizeof(g_spotLight));
	g_spotLight.Type = D3DLIGHT_SPOT;
	g_spotLight.Diffuse = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
	g_spotLight.Position = D3DXVECTOR3(0.0f, 0.0f, -5.0f);
	g_spotLight.Direction = D3DXVECTOR3(15.0f, 0.0f, 0.0f);
	g_spotLight.Range = 100.0f;    // a range of 100
	g_spotLight.Attenuation0 = 0.25f;    // no constant inverse attenuation
	g_spotLight.Attenuation1 = 0.0;    // only .125 inverse attenuation
	g_spotLight.Attenuation2 = 0.0f;    // no square inverse attenuation
	g_spotLight.Phi = D3DXToRadian(200.0f);    // set the outer cone to 30 degrees
	g_spotLight.Theta = D3DXToRadian(155.0f);    // set the inner cone to 10 degrees
	g_spotLight.Falloff = 1.0f;    // use the typical falloff
	*/
	ZeroMemory(&g_spotLight, sizeof(g_spotLight));
	g_spotLight.Type = D3DLIGHT_SPOT;
	g_spotLight.Diffuse = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
	g_spotLight.Position = D3DXVECTOR3(-5.0f, 0.0f, -5.0f);
	g_spotLight.Direction = D3DXVECTOR3(255.0f, 0.0f, 0.0f);
	g_spotLight.Range = 100.0f;    // a range of 100
	g_spotLight.Attenuation0 = 0.25f;    // no constant inverse attenuation
	g_spotLight.Attenuation1 = 0.0;    // only .125 inverse attenuation
	g_spotLight.Attenuation2 = 0.0f;    // no square inverse attenuation
	g_spotLight.Phi = D3DXToRadian(200.0f);    // set the outer cone to 30 degrees
	g_spotLight.Theta = D3DXToRadian(75.0f);    // set the inner cone to 10 degrees
	g_spotLight.Falloff = 1.0f;    // use the typical falloff

	ZeroMemory(&g_directionalLight, sizeof(g_directionalLight));
	g_directionalLight.Type = D3DLIGHT_DIRECTIONAL;
	g_directionalLight.Diffuse = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
	g_directionalLight.Direction = D3DXVECTOR3(-10.0f, 10.0f, 10.0f); //-1.0f, -0.3f, -1.0f
	
	g_d3ddev->SetLight(1, &g_pointLight);
	g_d3ddev->SetLight(2, &g_spotLight);
	g_d3ddev->SetLight(3, &g_directionalLight);
	g_d3ddev->LightEnable(1, FALSE);
	g_d3ddev->LightEnable(2, FALSE);
	g_d3ddev->LightEnable(3, TRUE);


	g_d3ddev->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	g_d3ddev->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	g_d3ddev->SetRenderState(D3DRS_LIGHTING, TRUE);
	g_d3ddev->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(100, 100, 100));
	g_d3ddev->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
	
}

// This is empty
void DGraphics::changeLights()
{
	
}

/*
	Processes keyboard input for:
	-Movement
	-Camera
	-Selection
*/
void DGraphics::processInput(BOOL* pPressedKeys, float tmp)
{
	float cameraSpeed = 5.0f;

	// Tigger Movment
	if (pPressedKeys[DIK_SPACE])
	{
		g_pTigerInstance[selected_tiger].RotateRel(D3DXToRadian(0.01 * -0.5f), D3DXToRadian(0.01 * -0.5f), 0.0f);
	}
	if (pPressedKeys[DIK_W])
	{
		g_pTigerInstance[selected_tiger].TranslateRel(0.0f, 0.001f, 0.0f);
	}
	if (pPressedKeys[DIK_A])
	{
		g_pTigerInstance[selected_tiger].TranslateRel(-0.001f, 0.0f, 0.0f);
	}
	if (pPressedKeys[DIK_S])
	{
		g_pTigerInstance[selected_tiger].TranslateRel(0.0f, -0.001f, 0.0f);
	}
	if (pPressedKeys[DIK_D])
	{
		g_pTigerInstance[selected_tiger].TranslateRel(0.001f, 0.0f, 0.0f);
	}
	if (pPressedKeys[DIK_Q])
	{
		g_pTigerInstance[selected_tiger].TranslateRel(0.0f, 0.0f, -0.001f);
	}
	if (pPressedKeys[DIK_E])
	{
		g_pTigerInstance[selected_tiger].TranslateRel(0.0f, 0.0f, 0.001f);
	}
	
	// Camera Movement
	if (pPressedKeys[DIK_I])
	{
		m_camera.MoveForward(0.005f);
	}
	if (pPressedKeys[DIK_K])
	{
		m_camera.MoveForward(-0.005f);
	}
	if (pPressedKeys[DIK_L])
	{
		m_camera.Strafe(0.005f);
	}
	if (pPressedKeys[DIK_J])
	{
		m_camera.Strafe(-0.005f);
	}
	if (pPressedKeys[DIK_U])
	{
		m_camera.Yaw(-0.00025f);
	}
	if (pPressedKeys[DIK_O])
	{
		m_camera.Yaw(0.00025f);
	}
	
	// Light management
	if (pPressedKeys[DIK_1])
	{
		g_d3ddev->LightEnable(1, TRUE);
	}
	if (pPressedKeys[DIK_2])
	{
		g_d3ddev->LightEnable(1, FALSE);
	}
	if (pPressedKeys[DIK_3])
	{
		g_d3ddev->LightEnable(2, TRUE);
	}
	if (pPressedKeys[DIK_4])
	{
		//spotLight = (spotLight) ? FALSE : TRUE;
		g_d3ddev->LightEnable(2, FALSE);
	}
	if (pPressedKeys[DIK_5])
	{
		g_d3ddev->LightEnable(3, TRUE);
	}
	if (pPressedKeys[DIK_6])
	{
		g_d3ddev->LightEnable(3, FALSE);
	}
	
	// Select tigger for movement
	if (pPressedKeys[DIK_F1])
	{
		selected_tiger = 0;
	}
	if (pPressedKeys[DIK_F2])
	{
		selected_tiger = 1;
	}

	if (pPressedKeys[DIK_ESCAPE])
	{
		PostQuitMessage(0);
	}
}

void DGraphics::renderMirror()
{
	if (m_camera.GetPosition()->z > -2.5)
		return;

	//
	// Draw Mirror quad to stencil buffer ONLY.  In this way
	// only the stencil bits that correspond to the mirror will
	// be on.  Therefore, the reflected teapot can only be rendered
	// where the stencil bits are turned on, and thus on the mirror 
	// only.
	//
	g_d3ddev->SetRenderState(D3DRS_STENCILENABLE, true);
	g_d3ddev->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
	g_d3ddev->SetRenderState(D3DRS_STENCILREF, 1);
	g_d3ddev->SetRenderState(D3DRS_STENCILMASK, 0xffffffff);
	g_d3ddev->SetRenderState(D3DRS_STENCILWRITEMASK, 0xffffffff);
	g_d3ddev->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
	g_d3ddev->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
	g_d3ddev->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);

	// disable writes to the depth and back buffers
	g_d3ddev->SetRenderState(D3DRS_ZWRITEENABLE, false);
	g_d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	g_d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
	g_d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	// draw the mirror to the stencil buffer
	g_d3ddev->SetStreamSource(0, VB, 0, sizeof(Vertex));
	g_d3ddev->SetFVF(Vertex::FVF);
	g_d3ddev->SetMaterial(&g_mirrorMtrl);
	g_d3ddev->SetTexture(0, g_mirrorTex);
	D3DXMATRIX I;
	D3DXMatrixIdentity(&I);
	g_d3ddev->SetTransform(D3DTS_WORLD, &I);
	
	g_d3ddev->DrawPrimitive(D3DPT_TRIANGLELIST, 18, 2);	

	// re-enable depth writes
	g_d3ddev->SetRenderState(D3DRS_ZWRITEENABLE, true);

	// only draw reflected to the pixels where the mirror was drawn to.
	g_d3ddev->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
	g_d3ddev->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);

	if (g_pTigerInstance->GetZPosition() < -2.5)
	{

		// position reflection
		D3DXMATRIX W, translationMatrix, R, rotationMatrix;
		D3DXPLANE plane(0.0f, 0.0f, -1.0f, 0.0f); // xy plane
		D3DXMatrixReflect(&R, &plane);

		D3DXMatrixTranslation(&translationMatrix,
			g_pTigerInstance->GetXPosition(),
			g_pTigerInstance->GetYPosition(),
			g_pTigerInstance->GetZPosition()
		);

		D3DXMatrixRotationYawPitchRoll(&rotationMatrix,
			g_pTigerInstance->GetXRotation(),
			g_pTigerInstance->GetYRotation(),
			g_pTigerInstance->GetZRotation()
		);

		W = rotationMatrix * translationMatrix * R;

		// clear depth buffer and blend the reflected object with the mirror
		g_d3ddev->Clear(0, 0, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
		g_d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
		g_d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);

		// Finally, draw the reflected teapot
		g_d3ddev->SetTransform(D3DTS_WORLD, &W);
		g_d3ddev->SetMaterial(&g_mirrorReflectMtrl);
		g_d3ddev->SetTexture(0, 0);

		g_d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		g_mesh.GetMesh()->DrawSubset(0);

	}
	
	

	// Restore render states.
	g_d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	g_d3ddev->SetRenderState(D3DRS_STENCILENABLE, false);
	g_d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	
}

void DGraphics::renderLeftMirror()
{

	if (m_camera.GetPosition()->x > -2.5)
		return;

	//
	// Draw Mirror quad to stencil buffer ONLY.  In this way
	// only the stencil bits that correspond to the mirror will
	// be on.  Therefore, the reflected teapot can only be rendered
	// where the stencil bits are turned on, and thus on the mirror 
	// only.
	//
	g_d3ddev->SetRenderState(D3DRS_STENCILENABLE, true);
	g_d3ddev->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
	g_d3ddev->SetRenderState(D3DRS_STENCILREF, 2);
	g_d3ddev->SetRenderState(D3DRS_STENCILMASK, 0xffffffff);
	g_d3ddev->SetRenderState(D3DRS_STENCILWRITEMASK, 0xffffffff);
	g_d3ddev->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
	g_d3ddev->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
	g_d3ddev->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);

	// disable writes to the depth and back buffers
	g_d3ddev->SetRenderState(D3DRS_ZWRITEENABLE, false);
	g_d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	g_d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
	g_d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	// draw the mirror to the stencil buffer
	g_d3ddev->SetStreamSource(0, VB, 0, sizeof(Vertex)	);
	g_d3ddev->SetFVF(Vertex::FVF);
	g_d3ddev->SetMaterial(&g_mirrorMtrl);
	g_d3ddev->SetTexture(0, g_mirrorTex);
	D3DXMATRIX I;
	D3DXMatrixIdentity(&I);
	g_d3ddev->SetTransform(D3DTS_WORLD, &I);

	//g_d3ddev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 8);
	g_d3ddev->DrawPrimitive(D3DPT_TRIANGLELIST, 12, 2);

	// re-enable depth writes
	g_d3ddev->SetRenderState(D3DRS_ZWRITEENABLE, true);

	// only draw reflected to the pixels where the mirror was drawn to.
	g_d3ddev->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
	g_d3ddev->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);


	if (g_pTigerInstance->GetXPosition() < -2.5f)
	{
		// position reflection
		D3DXMATRIX W, translationMatrix, R, rotationMatrix;
		D3DXPLANE plane(-1.0f, 0.0f, 0.0f, 0.0f); // yz plane
		D3DXMatrixReflect(&R, &plane);

		D3DXMatrixTranslation(&translationMatrix,
			g_pTigerInstance->GetXPosition(),
			g_pTigerInstance->GetYPosition(),
			g_pTigerInstance->GetZPosition()
		);

		D3DXMatrixRotationYawPitchRoll(&rotationMatrix,
			g_pTigerInstance->GetXRotation(),
			g_pTigerInstance->GetYRotation(),
			g_pTigerInstance->GetZRotation()
		);

		W = rotationMatrix * translationMatrix * R;

		// clear depth buffer and blend the reflected object with the mirror
		g_d3ddev->Clear(0, 0, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
		g_d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
		g_d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);

		// Finally, draw the reflected teapot
		g_d3ddev->SetTransform(D3DTS_WORLD, &W);
		g_d3ddev->SetMaterial(&g_mirrorReflectMtrl);
		g_d3ddev->SetTexture(0, 0);

		g_d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		g_mesh.GetMesh()->DrawSubset(0);

	}
	

	// Restore render states.
	g_d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	g_d3ddev->SetRenderState(D3DRS_STENCILENABLE, false);
	g_d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

void DGraphics::renderRightMirror()
{

	if (m_camera.GetPosition()->x < 2.5)
		return;

	//
	// Draw Mirror quad to stencil buffer ONLY.  In this way
	// only the stencil bits that correspond to the mirror will
	// be on.  Therefore, the reflected teapot can only be rendered
	// where the stencil bits are turned on, and thus on the mirror 
	// only.
	//
	g_d3ddev->SetRenderState(D3DRS_STENCILENABLE, true);
	g_d3ddev->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
	g_d3ddev->SetRenderState(D3DRS_STENCILREF, 3);
	g_d3ddev->SetRenderState(D3DRS_STENCILMASK, 0xffffffff);
	g_d3ddev->SetRenderState(D3DRS_STENCILWRITEMASK, 0xffffffff);
	g_d3ddev->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
	g_d3ddev->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
	g_d3ddev->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);

	// disable writes to the depth and back buffers
	g_d3ddev->SetRenderState(D3DRS_ZWRITEENABLE, false);
	g_d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	g_d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
	g_d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	// draw the mirror to the stencil buffer
	g_d3ddev->SetStreamSource(0, VB, 0, sizeof(Vertex));
	g_d3ddev->SetFVF(Vertex::FVF);
	g_d3ddev->SetMaterial(&g_mirrorMtrl);
	g_d3ddev->SetTexture(0, g_mirrorTex);
	D3DXMATRIX I;
	D3DXMatrixIdentity(&I);
	g_d3ddev->SetTransform(D3DTS_WORLD, &I);

	//g_d3ddev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 8);
	g_d3ddev->DrawPrimitive(D3DPT_TRIANGLELIST, 6, 2);

	// re-enable depth writes
	g_d3ddev->SetRenderState(D3DRS_ZWRITEENABLE, true);

	// only draw reflected to the pixels where the mirror was drawn to.
	g_d3ddev->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
	g_d3ddev->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);

	if (g_pTigerInstance->GetXPosition() > 2.5)
	{
		// position reflection
		D3DXMATRIX W, translationMatrix, R, rotationMatrix;
		D3DXPLANE plane(1.0f, 0.0f, 0.0f, 0.0f); // yz plane
		D3DXMatrixReflect(&R, &plane);

		D3DXMatrixTranslation(&translationMatrix,
			g_pTigerInstance->GetXPosition(),
			g_pTigerInstance->GetYPosition(),
			g_pTigerInstance->GetZPosition()
		);

		D3DXMatrixRotationYawPitchRoll(&rotationMatrix,
			g_pTigerInstance->GetXRotation(),
			g_pTigerInstance->GetYRotation(),
			g_pTigerInstance->GetZRotation()
		);

		W = rotationMatrix * translationMatrix * R;

		// clear depth buffer and blend the reflected object with the mirror
		g_d3ddev->Clear(0, 0, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
		g_d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
		g_d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);

		// Finally, draw the reflected teapot
		g_d3ddev->SetTransform(D3DTS_WORLD, &W);
		g_d3ddev->SetMaterial(&g_mirrorReflectMtrl);
		g_d3ddev->SetTexture(0, 0);

		g_d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		g_mesh.GetMesh()->DrawSubset(0);

	}
	
	// Restore render states.
	g_d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	g_d3ddev->SetRenderState(D3DRS_STENCILENABLE, false);
	g_d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

void DGraphics::renderBackMirror()
{

	if (m_camera.GetPosition()->z < 2.5)
		return;

	//
	// Draw Mirror quad to stencil buffer ONLY.  In this way
	// only the stencil bits that correspond to the mirror will
	// be on.  Therefore, the reflected teapot can only be rendered
	// where the stencil bits are turned on, and thus on the mirror 
	// only.
	//
	g_d3ddev->SetRenderState(D3DRS_STENCILENABLE, true);
	g_d3ddev->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
	g_d3ddev->SetRenderState(D3DRS_STENCILREF, 4);
	g_d3ddev->SetRenderState(D3DRS_STENCILMASK, 0xffffffff);
	g_d3ddev->SetRenderState(D3DRS_STENCILWRITEMASK, 0xffffffff);
	g_d3ddev->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
	g_d3ddev->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
	g_d3ddev->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);

	// disable writes to the depth and back buffers
	g_d3ddev->SetRenderState(D3DRS_ZWRITEENABLE, false);
	g_d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	g_d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
	g_d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	// draw the mirror to the stencil buffer
	g_d3ddev->SetStreamSource(0, VB, 0, sizeof(Vertex));
	g_d3ddev->SetFVF(Vertex::FVF);
	g_d3ddev->SetMaterial(&g_mirrorMtrl);
	g_d3ddev->SetTexture(0, g_mirrorTex);
	D3DXMATRIX I;
	D3DXMatrixIdentity(&I);
	g_d3ddev->SetTransform(D3DTS_WORLD, &I);

	//g_d3ddev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 8);
	g_d3ddev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);

	// re-enable depth writes
	g_d3ddev->SetRenderState(D3DRS_ZWRITEENABLE, true);

	// only draw reflected to the pixels where the mirror was drawn to.
	g_d3ddev->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
	g_d3ddev->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
	
	if (g_pTigerInstance->GetZPosition() > 2.5)
	{
		// position reflection
		D3DXMATRIX W, translationMatrix, R, rotationMatrix;
		D3DXPLANE plane(0.0f, 0.0f, 1.0f, 0.0f); // yz plane
		D3DXMatrixReflect(&R, &plane);

		D3DXMatrixTranslation(&translationMatrix,
			g_pTigerInstance->GetXPosition(),
			g_pTigerInstance->GetYPosition(),
			g_pTigerInstance->GetZPosition()
		);

		D3DXMatrixRotationYawPitchRoll(&rotationMatrix,
			g_pTigerInstance->GetXRotation(),
			g_pTigerInstance->GetYRotation(),
			g_pTigerInstance->GetZRotation()
		);

		W = rotationMatrix * translationMatrix * R;

		// clear depth buffer and blend the reflected object with the mirror
		g_d3ddev->Clear(0, 0, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
		g_d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
		g_d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);

		// Finally, draw the reflected teapot
		g_d3ddev->SetTransform(D3DTS_WORLD, &W);
		g_d3ddev->SetMaterial(&g_mirrorReflectMtrl);
		g_d3ddev->SetTexture(0, 0);

		g_d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		g_mesh.GetMesh()->DrawSubset(0);

	}

	
	// Restore render states.
	g_d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	g_d3ddev->SetRenderState(D3DRS_STENCILENABLE, false);
	g_d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

void DGraphics::setupMirror()
{
	g_d3ddev->CreateVertexBuffer(
		24 * sizeof(Vertex),
		0, // usage
		Vertex::FVF,
		D3DPOOL_MANAGED,
		&VB,
		0);

	Vertex* v = 0;

	VB->Lock(0, 0, (void**)&v, 0);

	// mirror back face
	v[5] = Vertex(-2.5f, 0.0f, 2.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	v[4] = Vertex(-2.5f, 5.0f, 2.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	v[3] = Vertex(2.5f, 5.0f, 2.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);

	v[2] = Vertex(-2.5f, 0.0f, 2.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	v[1] = Vertex(2.5f, 5.0f, 2.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
	v[0] = Vertex(2.5f, 0.0f, 2.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

	// mirror right face
	v[11] = Vertex(2.5f, 5.0f, 2.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[10] = Vertex(2.5f, 5.0f, -2.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[9] = Vertex(2.5f, 0.0f, -2.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	v[8] = Vertex(2.5f, 0.0f, -2.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[7] = Vertex(2.5f, 0.0f, 2.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[6] = Vertex(2.5f, 5.0f, 2.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// mirror left face
	v[12] = Vertex(-2.5f, 5.0f, 2.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[13] = Vertex(-2.5f, 5.0f, -2.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[14] = Vertex(-2.5f, 0.0f, -2.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	v[15] = Vertex(-2.5f, 0.0f, -2.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[16] = Vertex(-2.5f, 0.0f, 2.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[17] = Vertex(-2.5f, 5.0f, 2.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// mirror front face 
	v[18] = Vertex(-2.5f, 0.0f, -2.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[19] = Vertex(-2.5f, 5.0f, -2.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[20] = Vertex(2.5f, 5.0f, -2.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);

	v[21] = Vertex(-2.5f, 0.0f, -2.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[22] = Vertex(2.5f, 5.0f, -2.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[23] = Vertex(2.5f, 0.0f, -2.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	VB->Unlock();

	D3DXCreateTextureFromFile(g_d3ddev, TEXT("ice.bmp"), &g_mirrorTex);

	g_d3ddev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	g_d3ddev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	g_d3ddev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
}


/* Ray picking stuff beyond this point */
DGraphics::Ray DGraphics::calcPickingRay(int x, int y)
{
	float px = 0.0f;
	float py = 0.0f;

	D3DVIEWPORT9 vp;
	g_d3ddev->GetViewport(&vp);

	D3DXMATRIX proj;
	g_d3ddev->GetTransform(D3DTS_PROJECTION, &proj);

	px = (((2.0f*x) / vp.Width) - 1.0f) / proj(0, 0);
	py = (((-2.0f*y) / vp.Height) + 1.0f) / proj(1, 1);

	DGraphics::Ray ray;
	ray._origin = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	ray._direction = D3DXVECTOR3(px, py, 1.0f);

	return ray;
}

void DGraphics::transformRay(DGraphics::Ray* ray, D3DXMATRIX* T)
{
	// transform the ray's origin, w = 1.
	D3DXVec3TransformCoord(
		&ray->_origin,
		&ray->_origin,
		T);

	// transform the ray's direction, w = 0.
	D3DXVec3TransformNormal(
		&ray->_direction,
		&ray->_direction,
		T);

	// normalize the direction
	D3DXVec3Normalize(&ray->_direction, &ray->_direction);
}

bool DGraphics::raySphereIntTest(DGraphics::Ray* ray, DGraphics::BoundingSphere* sphere)
{
	D3DXVECTOR3 v = ray->_origin - sphere->_center;

	float b = 2.0f * D3DXVec3Dot(&ray->_direction, &v);
	float c = D3DXVec3Dot(&v, &v) - (sphere->_radius * sphere->_radius);

	// find the discriminant
	float discriminant = (b * b) - (4.0f * c);

	// test for imaginary number
	if (discriminant < 0.0f)
		return false;

	discriminant = sqrtf(discriminant);

	float s0 = (-b + discriminant) / 2.0f;
	float s1 = (-b - discriminant) / 2.0f;

	// if a solution is >= 0, then we intersected the sphere
	if (s0 >= 0.0f || s1 >= 0.0f)
		return true;

	return false;
}

void DGraphics::computeHitBox()
{
	D3DXMATRIX World;
	static float r = 0.0f;
	static float v = 1.0f;
	static float angle = 0.0f;

	D3DXMatrixTranslation(&World, g_pTigerInstance[selected_tiger].GetXPosition(),
		g_pTigerInstance[selected_tiger].GetYPosition(),
		g_pTigerInstance[selected_tiger].GetZPosition());
	

	// transfrom the bounding sphere to match the teapots position in the
	// world.
	
	g_sphere._center = D3DXVECTOR3(g_pTigerInstance[selected_tiger].GetXPosition(),
		g_pTigerInstance[selected_tiger].GetYPosition(),
		g_pTigerInstance[selected_tiger].GetZPosition());
	
	g_d3ddev->SetTransform(D3DTS_WORLD, &World);

	// Render the bounding sphere with alpha blending so we can see 
	// through it.
	g_d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	g_d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	g_d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	D3DMATERIAL9 blue = BLUE_MTRL;
	blue.Diffuse.a = 0.0125f; // 25% opacity
	g_d3ddev->SetMaterial(&blue);
	g_msphere->DrawSubset(0);

	g_d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

}

DGraphics::BoundingSphere::BoundingSphere()
{
	_radius = 0.0f;
}