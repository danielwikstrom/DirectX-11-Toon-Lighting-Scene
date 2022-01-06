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
    void Clear();
    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

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

	//Cameras
	Camera																	m_Camera01;
    Camera																	m_Camera02;

	//textures 
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_waterTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_terrainTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_noiseTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_sandTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_sandNormal;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_rockTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_rockNormal;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_fireTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_fireAlpha;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_logTexture;

	//Shaders
	Shader																	m_waterShader;
    Shader																	m_terrainShader;
    Shader																	m_seafloorShader;
    Shader																	m_logShader;
    FireShader															    m_fireShader;

    // 3D Models
    ModelClass                                                              m_WaterModel;
    ModelClass                                                              m_TerrainModel;
    ModelClass                                                              m_CharacterModel;
    ModelClass                                                              m_floor;
    ModelClass                                                              m_FireQuad;
    ModelClass                                                              m_logModel;

	//RenderTextures
	RenderTexture*															m_FirstRenderPass;
	RECT																	m_fullscreenRect;
	RECT																	m_CameraViewRect;


	


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