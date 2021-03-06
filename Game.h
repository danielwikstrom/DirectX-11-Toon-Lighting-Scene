//
// Game.h
//
#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "Shader.h"
#include "modelclass.h"
#include "Light.h"
#include "Input.h"
#include "Camera.h"
#include "RenderTexture.h"
#include "FireShader.h"

#define PI 3.141592
// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game final : public DX::IDeviceNotify
{
public:

    Game() noexcept(false);
    ~Game();

    

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // IDeviceNotify
    virtual void OnDeviceLost() override;
    virtual void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnWindowSizeChanged(int width, int height);
#ifdef DXTK_AUDIO
    void NewAudioDevice();
#endif

    // Properties
    void GetDefaultSize( int& width, int& height ) const;
	
private:

	struct MatrixBufferType
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;
	}; 

    void Update(DX::StepTimer const& timer);
    void Render();
	void RenderTexturePass1();
    SimpleMath::Quaternion QuaternionRotation(SimpleMath::Vector3 point, SimpleMath::Vector3 axis, float angle);
    void Clear();
    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();
    void DrawSimpleBox(Shader shader, SimpleMath::Vector3 position, float scaleX, float scaleY, float scaleZ);

    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;

	//input manager. 
	Input									m_input;
	InputCommands							m_gameInputCommands;

    // DirectXTK objects.
    std::unique_ptr<DirectX::CommonStates>                                  m_states;
    std::unique_ptr<DirectX::BasicEffect>                                   m_batchEffect;	
    std::unique_ptr<DirectX::EffectFactory>                                 m_fxFactory;
    std::unique_ptr<DirectX::SpriteBatch>                                   m_sprites;
    std::unique_ptr<DirectX::SpriteFont>                                    m_font;

	// Scene Objects
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>  m_batch;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>                               m_batchInputLayout;
	std::unique_ptr<DirectX::GeometricPrimitive>                            m_testmodel;

	//lights
	Light																	m_Light;
    Light																	m_PointLight;

	//Cameras
	Camera																	m_Camera01;
    Camera																	m_Camera02;

	//textures 
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_waterTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_noiseTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_sandTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_fireTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_fireAlpha;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_logTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_dolphinTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_skyboxTex;

	//Shaders
	Shader																	m_waterShader;
    Shader																	m_sandShader;
    Shader																	m_logShader;
    Shader																	m_dolphinShader;
    FireShader															    m_fireShader;
    Shader															        m_skyboxShader;

    // 3D Models
    ModelClass                                                              m_WaterModel;
    ModelClass                                                              m_TerrainModel;
    ModelClass                                                              m_floor;
    ModelClass                                                              m_FireQuad;
    ModelClass                                                              m_logModel;
    ModelClass                                                              m_dolphinModel;
    ModelClass                                                              m_treeModel;

	//RenderTextures
	RenderTexture*															m_FirstRenderPass;
	RECT																	m_fullscreenRect;
	RECT																	m_CameraViewRect;

    bool                                                                    TorchIsOn = false;
	


#ifdef DXTK_AUDIO
    std::unique_ptr<DirectX::AudioEngine>                                   m_audEngine;
    std::unique_ptr<DirectX::WaveBank>                                      m_waveBank;
    std::unique_ptr<DirectX::SoundEffect>                                   m_soundEffect;
    std::unique_ptr<DirectX::SoundEffectInstance>                           m_effect1;
    std::unique_ptr<DirectX::SoundEffectInstance>                           m_effect2;
#endif
    

#ifdef DXTK_AUDIO
    uint32_t                                                                m_audioEvent;
    float                                                                   m_audioTimerAcc;

    bool                                                                    m_retryDefault;
#endif

    DirectX::SimpleMath::Matrix                                             m_world;
    DirectX::SimpleMath::Matrix                                             m_view;
    DirectX::SimpleMath::Matrix                                             m_projection;

    DirectX::SimpleMath::Matrix                                             m_world2;
    DirectX::SimpleMath::Matrix                                             m_view2;
    DirectX::SimpleMath::Matrix                                             m_projection2;
};