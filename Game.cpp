//
// Game.cpp
//

#include "pch.h"
#include "Game.h"


//toreorganise
#include <fstream>

extern void ExitGame();

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
}

Game::~Game()
{
#ifdef DXTK_AUDIO
    if (m_audEngine)
    {
        m_audEngine->Suspend();
    }
#endif
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{

	m_input.Initialise(window);

    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

	m_fullscreenRect.left = 0;
	m_fullscreenRect.top = 0;
	m_fullscreenRect.right = 800;
	m_fullscreenRect.bottom = 600;

	m_CameraViewRect.left = 500;
	m_CameraViewRect.top = 0;
	m_CameraViewRect.right = 800;
	m_CameraViewRect.bottom = 240;

	//setup light
	m_Light.setAmbientColour(0.02f, 0.05f, 0.06f, 1.0f);
	m_Light.setDiffuseColour(0.3f, 0.3f, 0.3f, 1.0f);
	m_Light.setPosition(0.0f, 10.0f, 0.0f);
	m_Light.setDirection(0.0f, -1.0f, 0.0f);

    //setup point light
    m_PointLight.setDiffuseColour(1.0f, 0.5f, 0.25f, 1.0f);
    m_PointLight.setPosition(0.0f, 0.0f, 4.0f);

	//setup camera
	m_Camera01.setPosition(Vector3(00.0f, 4.0f, 10.0f));
	m_Camera01.setRotation(Vector3(0.0f, -180.0f, 90.0f));	//orientation is -90 becuase zero will be looking up at the sky straight up. 
	m_Camera02.setPosition(Vector3(0.0f, 7.0f, 0.0f));
	m_Camera02.setRotation(Vector3(0.0f, -180.0f, 180.0f));	//orientation is -90 becuase zero will be looking up at the sky straight up. 



	
#ifdef DXTK_AUDIO
    // Create DirectXTK for Audio objects
    AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
    eflags = eflags | AudioEngine_Debug;
#endif

    m_audEngine = std::make_unique<AudioEngine>(eflags);

    m_audioEvent = 0;
    m_audioTimerAcc = 10.f;
    m_retryDefault = false;

    m_waveBank = std::make_unique<WaveBank>(m_audEngine.get(), L"adpcmdroid.xwb");

    m_soundEffect = std::make_unique<SoundEffect>(m_audEngine.get(), L"MusicMono_adpcm.wav");
    m_effect1 = m_soundEffect->CreateInstance();
    m_effect2 = m_waveBank->CreateInstance(10);

    m_effect1->Play(true);
    m_effect2->Play();
#endif
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
	//take in input
	m_input.Update();								//update the hardware
	m_gameInputCommands = m_input.getGameInput();	//retrieve the input for our game
	
	//Update all game objects
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

	//Render all game content. 
    Render();

#ifdef DXTK_AUDIO
    // Only update audio engine once per frame
    if (!m_audEngine->IsCriticalError() && m_audEngine->Update())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
#endif

	
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
	//note that currently.  Delta-time is not considered in the game object movement. 
    float deltaTime = float(timer.GetElapsedSeconds());
    float rotationSpeed = m_Camera01.getRotationSpeed() * deltaTime;
    float movementSpeed = m_Camera01.getMoveSpeed() * deltaTime;
    //note that currently.  Delta-time is not considered in the game object movement. 
    if (m_gameInputCommands.rotLeft)
    {
        Vector3 rotation = m_Camera01.getRotation();
        rotation.y = rotation.y += rotationSpeed;
        m_Camera01.setRotation(rotation);
    }
    if (m_gameInputCommands.rotRight)
    {
        Vector3 rotation = m_Camera01.getRotation();
        rotation.y = rotation.y -= rotationSpeed;
        
        m_Camera01.setRotation(rotation);
    }
    if (m_gameInputCommands.rotUp)
    {
        Vector3 rotation = m_Camera01.getRotation();
        rotation.z = rotation.z -= rotationSpeed;
        if (rotation.z <= 0.5f)
            rotation.z = 0.5f;
        m_Camera01.setRotation(rotation);
    }
    if (m_gameInputCommands.rotDown)
    {
        Vector3 rotation = m_Camera01.getRotation();
        rotation.z = rotation.z += rotationSpeed;
        if (rotation.z >= 179.5f)
            rotation.z = 179.5f;
        m_Camera01.setRotation(rotation);
    }
    if (m_gameInputCommands.forward)
    {
        Vector3 position = m_Camera01.getPosition(); //get the position
        position += (m_Camera01.getForward() * movementSpeed); //add the forward vector
        m_Camera01.setPosition(position);
    }
    if (m_gameInputCommands.back)
    {
        Vector3 position = m_Camera01.getPosition(); //get the position
        position -= (m_Camera01.getForward() * movementSpeed); //add the forward vector
        m_Camera01.setPosition(position);
    }
    if (m_gameInputCommands.right)
    {
        Vector3 position = m_Camera01.getPosition(); //get the position
        position += (m_Camera01.getRight() * movementSpeed); //add the forward vector
        m_Camera01.setPosition(position);
    }
    if (m_gameInputCommands.left)
    {
        Vector3 position = m_Camera01.getPosition(); //get the position
        position -= (m_Camera01.getRight() * movementSpeed); //add the forward vector
        m_Camera01.setPosition(position);
    }

	m_Camera01.Update();	//camera update.

	m_view = m_Camera01.getCameraMatrix();

	m_Camera02.Update();	//camera update.

	m_view2 = m_Camera02.getCameraMatrix();
	m_world = Matrix::Identity;

#ifdef DXTK_AUDIO
    m_audioTimerAcc -= (float)timer.GetElapsedSeconds();
    if (m_audioTimerAcc < 0)
    {
        if (m_retryDefault)
        {
            m_retryDefault = false;
            if (m_audEngine->Reset())
            {
                // Restart looping audio
                m_effect1->Play(true);
            }
        }
        else
        {
            m_audioTimerAcc = 4.f;

            m_waveBank->Play(m_audioEvent++);

            if (m_audioEvent >= 11)
                m_audioEvent = 0;
        }
    }
#endif

  
	if (m_input.Quit())
	{
		ExitGame();
	}
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{	
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTargetView = m_deviceResources->GetRenderTargetView();
	auto depthTargetView = m_deviceResources->GetDepthStencilView();
    
    // Draw Text to the screen
    m_sprites->Begin();
		m_font->DrawString(m_sprites.get(), L"DirectXTK Demo Window", XMFLOAT2(10, 10), Colors::Yellow);
    m_sprites->End();

	//Set Rendering states. 
	context->OMSetDepthStencilState(m_states->DepthDefault(), 0);


	/////////////////////////////////////////////////////////////draw our scene normally. 

    //Render inner faces of skybox
	context->RSSetState(m_states->CullNone());
    //RenderSkybox
    m_world = SimpleMath::Matrix::Identity;
    SimpleMath::Matrix skyboxScale = SimpleMath::Matrix::CreateScale(40.0f, 40.0f, 40.0f);
    m_world = m_world * skyboxScale;

    m_skyboxShader.EnableShader(context);
    m_skyboxShader.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, &m_PointLight, 0.0f, Vector2(0.05f, 0.05f), m_skyboxTex.Get());
    m_skyBoxModel.Render(context);


    context->RSSetState(m_states->CullClockwise());

    // Prepare transforms for island model
    m_world = SimpleMath::Matrix::Identity;
    SimpleMath::Matrix islandOffset = SimpleMath::Matrix::CreateTranslation(1.0f, -8.5f, 2.0f);
    SimpleMath::Matrix islandScale = SimpleMath::Matrix::CreateScale(0.4f, 0.4f, 0.4f);
    SimpleMath::Matrix islandPosition = islandScale * islandOffset;
    m_world = m_world * islandPosition;

    //render the island model
    m_terrainShader.EnableShader(context);
    m_terrainShader.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, &m_PointLight, 0.0f, Vector2(0.05f, 0.05f), m_rockTexture.Get());
    m_TerrainModel.Render(context);

    // Prepare transforms for logs model
    m_world = SimpleMath::Matrix::Identity;
    SimpleMath::Matrix logOffset = SimpleMath::Matrix::CreateTranslation(0.0f, 1.3f, 0.0f);
    SimpleMath::Matrix logScale = SimpleMath::Matrix::CreateScale(0.3f, 0.3f, 0.3f);
    SimpleMath::Matrix logPosition = logScale * logOffset;
    m_world = m_world * logPosition;

    //render the logs model
    m_logShader.EnableShader(context);
    m_logShader.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, &m_PointLight, 0.0f, Vector2(0.05f, 0.05f), m_logTexture.Get());
    m_logModel.Render(context);


    float radiansPerSec = 3.1415f * 0.25f;
    float radiansPerFrame = radiansPerSec / m_timer.GetFramesPerSecond();
    float currentRotation = radiansPerFrame * m_timer.GetFrameCount();

    // Prepare transforms for dolphins model
    m_world = SimpleMath::Matrix::Identity;
    SimpleMath::Vector3 dolphinPos = Vector3(0.0f, -2.0f, 12.0f);
    SimpleMath::Matrix dolphinRotZ = SimpleMath::Matrix::CreateRotationZ(-15 * (3.1415f/180.0f));
    SimpleMath::Matrix dolphinRotY = SimpleMath::Matrix::CreateRotationY(currentRotation);
    SimpleMath::Matrix dolphinScale = SimpleMath::Matrix::CreateScale(1.0f, 1.0f, 1.0f);

    //Calculate dolphins new position using Quaterions
    Quaternion newQ = QuaternionRotation(dolphinPos, Vector3(0, 1, 0), currentRotation);
    Vector3 newP = Vector3(newQ.x, newQ.y, newQ.z);
    SimpleMath::Matrix dolphinOffset = SimpleMath::Matrix::CreateTranslation(newP);
    SimpleMath::Matrix dolphinTransform = dolphinRotZ * dolphinRotY * dolphinScale * dolphinOffset;
    m_world = m_world * dolphinTransform;



    //render the dolphins model
    m_dolphinShader.EnableShader(context);
    m_dolphinShader.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, &m_PointLight, 0.0f, Vector2(0.05f, 0.05f), m_dolphinTex.Get());
    m_dolphinModel.Render(context);

    // Prepare transforms for sea floor
    m_world = SimpleMath::Matrix::Identity;
    SimpleMath::Matrix seafloorOffset = SimpleMath::Matrix::CreateTranslation(0.0f, -30.0f, 0.0f);
    SimpleMath::Matrix seaFloorScale = SimpleMath::Matrix::CreateScale(1000.0f, 1.0f, 1000.0f);
    SimpleMath::Matrix seaFloorPos =seaFloorScale * seafloorOffset;
    m_world = m_world * seaFloorPos;

    //render the sea floor
    m_seafloorShader.EnableShader(context);
    m_seafloorShader.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, &m_PointLight, 10.0f, Vector2(100, 100), m_sandTexture.Get());
    m_floor.Render(context);

    //Prepare transforms for water
	m_world = SimpleMath::Matrix::Identity;
	SimpleMath::Matrix waterPosition = SimpleMath::Matrix::CreateTranslation(0.0f, -1.0f, 0.0f);
    SimpleMath::Matrix waterScale = SimpleMath::Matrix::CreateScale(0.2f, 1.0f, 0.2f);
	m_world = m_world * waterScale * waterPosition;



	//render the water model
    context->OMSetBlendState(m_states->AlphaBlend(), nullptr, 0xFFFFFFFF);
	m_waterShader.EnableShader(context);
	m_waterShader.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, &m_PointLight, m_timer.GetTotalSeconds(), Vector2(30,30), m_waterTexture.Get());
	m_WaterModel.Render(context);


    //Transform matrix for fire
    m_world = SimpleMath::Matrix::Identity;
    SimpleMath::Vector3 firePos = Vector3(00.0f, 4.2f, 0.0f);
    SimpleMath::Matrix fireOffset = SimpleMath::Matrix::CreateTranslation(firePos);
    SimpleMath::Matrix fireRotationX = SimpleMath::Matrix::CreateRotationX(-3.1415f/2);
    //SimpleMath::Matrix fireRotationY = SimpleMath::Matrix::CreateRotationY(3.1415f);
    SimpleMath::Matrix fireScale = SimpleMath::Matrix::CreateScale(0.01f, 0.0075f, 0.01f);

    //Look at camera behaviour
    SimpleMath::Vector3 fireForward = SimpleMath::Vector3(0, 0, 1);
    SimpleMath::Vector3 fireToCamVector = firePos - m_Camera01.getPosition();
    fireToCamVector = SimpleMath::Vector3(fireToCamVector.x, 0, fireToCamVector.z);
    float camMod = fireToCamVector.Length();
    float fireMod = fireForward.Length();
    float dotProd = fireToCamVector.Dot(fireForward);
    float cosA = dotProd / (camMod * fireMod);
    float angle = acos(cosA);

    //When the x part of the vector is positive, the rotation angle is negative and viceversa
    angle *= (abs(fireToCamVector.x) / fireToCamVector.x);
    SimpleMath::Matrix fireRotationY = SimpleMath::Matrix::CreateRotationY(angle);
    m_world = m_world * fireRotationX * fireRotationY * fireScale * fireOffset;



    //Render fire
    m_fireShader.EnableShader(context);
    m_fireShader.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, m_timer.GetTotalSeconds(), Vector2(1, 1), m_fireTexture.Get(), m_noiseTexture.Get(), m_fireAlpha.Get());
    m_FireQuad.Render(context);

	//std::wstring rotation = std::to_wstring();
	//const wchar_t* rochar = s.c_str();

	//// Draw Text to the screen
	//m_sprites->Begin();
	//m_font->DrawString(m_sprites.get(), rochar, XMFLOAT2(10, 0), Colors::Yellow);
	//m_sprites->End();







    context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
	///////////////////////////////////////draw our sprite with the render texture displayed on it. 


    // Show the new frame.
    m_deviceResources->Present();
}

Quaternion Game::QuaternionRotation(Vector3 point, Vector3 axis, float angle)
{
    Quaternion p = Quaternion(point.x, point.y, point.z, 1.0f);
    Quaternion q = Quaternion::CreateFromAxisAngle(axis, angle);
    Quaternion qc = q;
    q.Conjugate();
    Quaternion rotationQ = Quaternion::Concatenate(p, q);
    rotationQ = Quaternion::Concatenate(qc, rotationQ);
    return rotationQ;
}

//void Game::RenderTexturePass1()
//{
//	auto context = m_deviceResources->GetD3DDeviceContext();
//	auto renderTargetView = m_deviceResources->GetRenderTargetView();
//	auto depthTargetView = m_deviceResources->GetDepthStencilView();
//	// Set the render target to be the render to texture.
//	m_FirstRenderPass->setRenderTarget(context);
//	// Clear the render to texture.
//	m_FirstRenderPass->clearRenderTarget(context, 0.0f, 0.0f, 1.0f, 1.0f);
//
//	// Turn our shaders on,  set parameters
//    m_waterShader.EnableShader(context);
//	//m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_view2, &m_projection, &m_Light, m_texture1.Get());
//    //m_waterShader.SetShaderParameters(context, &m_world, &m_view2, &m_projection, &m_Light);
//
//	//render our model
//    m_WaterModel.Render(context);
//
//	//prepare transform for second object. 
//	SimpleMath::Matrix newPosition = SimpleMath::Matrix::CreateTranslation(2.0f, 0.0f, 0.0f);
//	m_world = m_world * newPosition;
//
//
//	//prepare transform for floor object. 
//	//m_world = SimpleMath::Matrix::Identity; //set world back to identity
//	//SimpleMath::Matrix newPosition2 = SimpleMath::Matrix::CreateTranslation(0.0f, -0.6f, 0.0f);
//	//m_world = m_world * newPosition2;
//
//
//	// Reset the render target back to the original back buffer and not the render to texture anymore.	
//	context->OMSetRenderTargets(1, &renderTargetView, depthTargetView);
//}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}

#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
}

void Game::OnDeactivated()
{
}

void Game::OnSuspending()
{
#ifdef DXTK_AUDIO
    m_audEngine->Suspend();
#endif
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

#ifdef DXTK_AUDIO
    m_audEngine->Resume();
#endif
}

void Game::OnWindowMoved()
{
    auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();
}

#ifdef DXTK_AUDIO
void Game::NewAudioDevice()
{
    if (m_audEngine && !m_audEngine->IsAudioDevicePresent())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
}
#endif

// Properties
void Game::GetDefaultSize(int& width, int& height) const
{
    width = 800;
    height = 600;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto device = m_deviceResources->GetD3DDevice();

    m_states = std::make_unique<CommonStates>(device);
    m_fxFactory = std::make_unique<EffectFactory>(device);
    m_sprites = std::make_unique<SpriteBatch>(context);
    m_font = std::make_unique<SpriteFont>(device, L"SegoeUI_18.spritefont");
	m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(context);

	//setup our models
	m_WaterModel.InitializeModel(device, "models/plane.obj");
    m_TerrainModel.InitializeModel(device, "models/terrain.obj");
    m_floor.InitializeBox(device, 1.0f, 1.0f, 1.0f);
    m_FireQuad.InitializeModel(device, "models/plane.obj");
    m_logModel.InitializeModel(device, "models/logs.obj");
    m_dolphinModel.InitializeModel(device, "models/dolphins.obj");
    m_skyBoxModel.InitializeModel(device, "models/skybox.obj");


	//m_BasicModel2.InitializeModel(device,"drone.obj");
	//m_BasicModel3.InitializeBox(device, 10.0f, 0.1f, 10.0f);	//box includes dimensions

	//load and set up our Vertex and Pixel Shaders
    m_waterShader.InitStandard(device, L"Water_vs.cso", L"Water_ps.cso");
    m_terrainShader.InitStandard(device, L"light_vs.cso", L"light_ps.cso");
    m_seafloorShader.InitStandard(device, L"Water_vs.cso", L"Water_ps.cso");
    m_logShader.InitStandard(device, L"light_vs.cso", L"light_ps.cso");
    m_dolphinShader.InitStandard(device, L"light_vs.cso", L"light_ps.cso");
    m_fireShader.InitStandard(device, L"fire_vs.cso", L"fire_ps.cso");
    m_skyboxShader.InitStandard(device, L"light_vs.cso", L"light_ps.cso");

	//load Textures
	CreateDDSTextureFromFile(device, L"textures/waterTile.dds",		nullptr, m_waterTexture.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"textures/TerrainGreen.dds", nullptr, m_terrainTexture.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"textures/noise.dds", nullptr, m_noiseTexture.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"textures/sandTile.dds", nullptr, m_sandTexture.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"textures/SandNormal.dds", nullptr, m_sandNormal.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"textures/rock.dds", nullptr, m_rockTexture.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"textures/rockNormal.dds", nullptr, m_rockNormal.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"textures/fireTexture.dds", nullptr, m_fireTexture.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"textures/fireAlpha.dds", nullptr, m_fireAlpha.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"textures/woodBrown.dds", nullptr, m_logTexture.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"textures/dolphinsTexture.dds", nullptr, m_dolphinTex.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"textures/OutputCube.dds", nullptr, m_skyboxTex.ReleaseAndGetAddressOf());

	//Initialise Render to texture
	m_FirstRenderPass = new RenderTexture(device, 800, 600, 1, 2);	//for our rendering, We dont use the last two properties. but.  they cant be zero and they cant be the same. 

}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    auto size = m_deviceResources->GetOutputSize();
    float aspectRatio = float(size.right) / float(size.bottom);
    float fovAngleY = 70.0f * XM_PI / 180.0f;

    // This is a simple example of change that can be made when the app is in
    // portrait or snapped view.
    if (aspectRatio < 1.0f)
    {
        fovAngleY *= 2.0f;
    }

    // This sample makes use of a right-handed coordinate system using row-major matrices.
    m_projection = Matrix::CreatePerspectiveFieldOfView(
        fovAngleY,
        aspectRatio,
        0.01f,
        100.0f
    );
}


void Game::OnDeviceLost()
{
    m_states.reset();
    m_fxFactory.reset();
    m_sprites.reset();
    m_font.reset();
	m_batch.reset();
	m_testmodel.reset();
    m_batchInputLayout.Reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
