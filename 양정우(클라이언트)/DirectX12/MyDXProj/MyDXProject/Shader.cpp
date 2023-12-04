#include "stdafx.h"
#include "Shader.h"

CShader::CShader()
{
}

CShader::~CShader()
{
    if (m_ppd3dPipelineStates)
    {
        for (int i = 0; i < m_nPipelineStates; i++)
        {
            if (m_ppd3dPipelineStates[i])
            {
                m_ppd3dPipelineStates[i]->Release();
            }
        }
        delete[] m_ppd3dPipelineStates;
    }
}
// IA에 정점 버퍼의 구조를 알려주기 위한 구조체를 반환
D3D12_INPUT_LAYOUT_DESC CShader::CreateInputLayout()                                        // 입력 레이아웃을 파이프라인에게 알리기 위함
{
    D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
    d3dInputLayoutDesc.pInputElementDescs = NULL;

    d3dInputLayoutDesc.NumElements = 0;

    return (d3dInputLayoutDesc);
}

// Rasterizer 상태를 설정하기 위한 구조체를 반환
D3D12_RASTERIZER_DESC CShader::CreateRasterizerState()
{
    D3D12_RASTERIZER_DESC d3dRasterizerDesc;
    ::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
    d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;                                     // 랜더링된 폴리곤 면을 그릴 방법, SOLID - 실선으로 채우기 / WIREFRAME - 변(Edge)만 그림
    d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;                                      // 뒷면 제거, 앞면만 랜더링
    d3dRasterizerDesc.FrontCounterClockwise = FALSE;                                        // 반시계로 그릴겁니까? = 아니요
    d3dRasterizerDesc.DepthBias = 0;                                                        // 그림자 여드름 해결용 설정
    d3dRasterizerDesc.DepthBiasClamp = 0.0f;                                                // 그림자 여드름 해결용 설정
    d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;                                          // 그림자 여드름 해결용 설정
    d3dRasterizerDesc.DepthClipEnable = TRUE;                                               // True라면 랜더링 중 깊이값을 클립해서 뒷면이 안보인다.
    d3dRasterizerDesc.MultisampleEnable = FALSE;                                            // MSAA 안씀
    d3dRasterizerDesc.AntialiasedLineEnable = FALSE;                                        // 랜더링 되느 선들의 MSAA여부
    d3dRasterizerDesc.ForcedSampleCount = 0;                                                // 다중샘플링 사용할때 샘플수 설정
    d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;       // 보수적래스터라이제이션을 쓸것인지 쓰지않을것인지

    return (d3dRasterizerDesc);
}

// Blending 상태를 성정하기 위한 구초체 반환.
D3D12_BLEND_DESC CShader::CreateBlendState()
{
    D3D12_BLEND_DESC d3dBlendDesc;
    ::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
    d3dBlendDesc.AlphaToCoverageEnable = FALSE;                                             // 알파 커버리지 비활성화
    d3dBlendDesc.IndependentBlendEnable = FALSE;                                            // Render Target을 독립적인 블렌딩 설정을 할까에 대한 여부, False이므로 모든 Render Target에 동일한 상태를 적용함
    d3dBlendDesc.RenderTarget[0].BlendEnable = FALSE;                                       // 0번 Blend 설정시작(최대 8개)
    d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;                                     // 논리 연산 활성화 여부
    d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;                                // 소스 색상을 그대로 사용하고 대상 색상은 사용하지 않음
    d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;                              // 이
    d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;                              // 하
    d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;                           // 동
    d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;                         // 문
    d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;                         // 알파 블랜딩 작업 중 수행할 연산
    d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;                             // 논리연산 비활성화
    d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;      // Render Target 색상 쓰기 마스크, 모든 색상채널에대한 쓰기가 허용

    return(d3dBlendDesc);
}

// Depth Stencil 검사를 위한 상태를 설정하기 위한 구조체를 반환
D3D12_DEPTH_STENCIL_DESC CShader::CreateDepthStencilState()
{
    D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
    ::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
    d3dDepthStencilDesc.DepthEnable = TRUE;                                                 // 깊이 테스트를 활성화
    d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;                        // 깊이버퍼 기록설정, 모든 픽셀의 깊이 값을 기록
    d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;                             // 깊이테스팅 비교함수, 픽셀의 깊이가 현재 깊이보다 작을때만 통과
    d3dDepthStencilDesc.StencilEnable = FALSE;                                              // 스탠실은 안쓸거니까.
    d3dDepthStencilDesc.StencilReadMask = 0x00;                                             // 이 아래는
    d3dDepthStencilDesc.StencilWriteMask = 0x00;                                            // 전부
    d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;                    // 안
    d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;               // 쓸
    d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;                    // 거
    d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;                // 라
    d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;                     // 고
    d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;                // 설
    d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;                     // 정
    d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;                 // 한다

    return (d3dDepthStencilDesc);
}

// Vertex Shader 바이트 코드를 생성(컴파일)한다
D3D12_SHADER_BYTECODE CShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob)
{
    D3D12_SHADER_BYTECODE d3dShaderByteCode;
    d3dShaderByteCode.BytecodeLength = 0;
    d3dShaderByteCode.pShaderBytecode = NULL;

    return(d3dShaderByteCode);
}

// Pixel Shader 바이트 코드를 생성(컴파일)한다
D3D12_SHADER_BYTECODE CShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob)
{
    D3D12_SHADER_BYTECODE d3dShaderByteCode;
    d3dShaderByteCode.BytecodeLength = 0;
    d3dShaderByteCode.pShaderBytecode = NULL;

    return(d3dShaderByteCode);
}

// 위에서 생성한 걸 이용해서 Shader 소스 코드를 컴파일 하여서 바이트 코드 구조체를 반환한다.
D3D12_SHADER_BYTECODE CShader::CompileShaderFromFile(WCHAR* pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob** ppd3dShaderBlob)
{
    UINT nCompileFlags = 0;
#if defined(_DEBUG)
    nCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    // Scene.cpp에서 쓰던 Shaders.hlsl 파일 컴파일 함수이다.
    ::D3DCompileFromFile(pszFileName, NULL, NULL, pszShaderName, pszShaderProfile, nCompileFlags, 0, ppd3dShaderBlob, NULL);

    D3D12_SHADER_BYTECODE d3dShaderByteCode;
    d3dShaderByteCode.BytecodeLength = (*ppd3dShaderBlob)->GetBufferSize();
    d3dShaderByteCode.pShaderBytecode = (*ppd3dShaderBlob)->GetBufferPointer();

    return(d3dShaderByteCode);
}

// Graphics Pipeline의 상태 객체를 생성
void CShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dRootSignature)
{

    ID3DBlob* pd3dVertexShaderBlob = NULL, * pd3dPixelShaderBlob = NULL;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;
    ::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    d3dPipelineStateDesc.pRootSignature = pd3dRootSignature;
    d3dPipelineStateDesc.VS = CreateVertexShader(&pd3dVertexShaderBlob);
    d3dPipelineStateDesc.PS = CreatePixelShader(&pd3dPixelShaderBlob);
    d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
    d3dPipelineStateDesc.BlendState = CreateBlendState();
    d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();
    d3dPipelineStateDesc.InputLayout = CreateInputLayout();
    d3dPipelineStateDesc.SampleMask = UINT_MAX;
    d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    d3dPipelineStateDesc.NumRenderTargets = 1;
    d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    d3dPipelineStateDesc.SampleDesc.Count = 1;
    d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;                            // Camera를 만들면서 Flag값 추가.

    pd3dDevice->CreateGraphicsPipelineState(
        &d3dPipelineStateDesc,
        __uuidof(ID3D12PipelineState),
        (void **)&m_ppd3dPipelineStates[0]
    );

    if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
    if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();

    if (d3dPipelineStateDesc.InputLayout.pInputElementDescs)
    {
        delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;
    }

}


// Camera를 위한 함수들
void CShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CShader::ReleaseShaderVariables()
{
}

void CShader::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World)
{
    XMFLOAT4X4 xmf4x4World;
    XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(pxmf4x4World)));
    pd3dCommandList->SetGraphicsRoot32BitConstants(0, 16, &xmf4x4World, 0);
}

void CShader::OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
    // Pipeline에 Graphics 상태객체를 설정
    //현재 게임 객체를 렌더링 파이프라인에 설정한다.
    pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[0]);
}

void CShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    OnPrepareRender(pd3dCommandList);
}



// Diffused Shader
CPlayerShader::CPlayerShader()
{
}

CPlayerShader::~CPlayerShader()
{
}

D3D12_INPUT_LAYOUT_DESC CPlayerShader::CreateInputLayout()                                                                            // 입력 레이아웃을 파이프라인에게 알리기 위함
{
    UINT nInputElementDescs = 2;
    D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];                                 // 입력 레이아웃(정점 버퍼 원소 하나의 구조에 대한 설명) 생성

    pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
    pd3dInputElementDescs[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

    D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
    d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
    d3dInputLayoutDesc.NumElements = nInputElementDescs;

    return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CPlayerShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob)
{
    return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSDiffused", "vs_5_1", ppd3dShaderBlob));
}
D3D12_SHADER_BYTECODE CPlayerShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob)
{
    return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSDiffused", "ps_5_1", ppd3dShaderBlob));
}

void CPlayerShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
    m_nPipelineStates = 1;
    m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];
    CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
}

// 땅바닥 쉐이더
CFieldShader::CFieldShader()
{
}

CFieldShader::~CFieldShader()
{
}

D3D12_INPUT_LAYOUT_DESC CFieldShader::CreateInputLayout()                                                                            // 입력 레이아웃을 파이프라인에게 알리기 위함
{
    UINT nInputElementDescs = 2;
    D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];                                 // 입력 레이아웃(정점 버퍼 원소 하나의 구조에 대한 설명) 생성

    pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
    pd3dInputElementDescs[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

    D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
    d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
    d3dInputLayoutDesc.NumElements = nInputElementDescs;

    return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CFieldShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob)
{
    return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSDiffused", "vs_5_1", ppd3dShaderBlob));
}
D3D12_SHADER_BYTECODE CFieldShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob)
{
    return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSDiffused", "ps_5_1", ppd3dShaderBlob));
}

void CFieldShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
    m_nPipelineStates = 1;
    m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];
    CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
}

// Object Shader
CObjectsShader::CObjectsShader()
{
}

CObjectsShader::~CObjectsShader()
{
}

// Object
void CObjectsShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
    m_nObjects = 0; // 오브젝트 배열 개수

    // 가로x세로x높이가 12x12x12인 정육면체 메쉬를 생성한다.
    // CubeMesh라는 놈은 이렇게 생겼어~
    CCubeMeshDiffused *pCubeMesh = new CCubeMeshDiffused(pd3dDevice, pd3dCommandList, 12.0f, 12.0f, 12.0f);
    CRotatingObject* pRotatingObject = NULL;
    pRotatingObject = new CRotatingObject();                        // 새로운 오브젝트 생성
    pRotatingObject->SetMesh(pCubeMesh);                            // 만든 오브젝트에 대한 모양 설정
    pRotatingObject->SetPosition(0.f, 0.f, 10.f);                    // 위치는 이러이러해
    pRotatingObject->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));   // 각은 이러이러해
    pRotatingObject->SetRotationSpeed(10.0f);                       // 뺑뻉이 속도는 이러이러해
    pRotatingObject->SelectObjectRender(true);
    m_nObjects = m_nObjects + 1;

    // 내가 한번 만들어본 삼각형
    CTriangleMesh* pTriangleMesh = new CTriangleMesh(pd3dDevice, pd3dCommandList);
    CRotatingObject* pRotateTriangle = NULL;
    pRotateTriangle = new CRotatingObject();
    pRotateTriangle->SetMesh(pTriangleMesh);
    pRotateTriangle->SetPosition(0.f, 15.f, 0.f);
    pRotateTriangle->SetRotationAxis(XMFLOAT3(1.0f, 1.0f, 1.0f));
    pRotateTriangle->SetRotationSpeed(50.f);
    pRotateTriangle->SelectObjectRender(true);
    m_nObjects = m_nObjects + 1;
    
    // 뺑글뺑글 도는 플레이어 객체가 아닌 비행기 메쉬
    CAirplaneMeshDiffused* pAirplainMesh = new CAirplaneMeshDiffused(pd3dDevice, pd3dCommandList);
    CRotatingObject* pRotateAirplain = NULL;
    pRotateAirplain = new CRotatingObject();
    pRotateAirplain->SetMesh(pAirplainMesh);
    pRotateAirplain->SetPosition(10.f, 15.f, 10.f);
    pRotateAirplain->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
    pRotateAirplain->SetRotationSpeed(50.f);
    pRotateAirplain->Rotate(90.f, 0.f, 0.f);
    pRotateAirplain->SelectObjectRender(true);
    m_nObjects = m_nObjects + 1;

    // "9""구""Sphere"
    CSphereMeshDiffused* pSphereMesh = new CSphereMeshDiffused(pd3dDevice, pd3dCommandList);
    CRotatingObject* pRotateSphere = NULL;
    pRotateSphere = new CRotatingObject();
    pRotateSphere->SetMesh(pSphereMesh);
    pRotateSphere->SetPosition(-20.f, 0.f, 0.f);
    pRotateSphere->SetRotationAxis(XMFLOAT3(1.0f, 1.0f, 1.0f));
    pRotateSphere->SetRotationSpeed(200.f);
    pRotateSphere->SelectObjectRender(true);
    m_nObjects = m_nObjects + 1;

    CModelMesh* pAirModelMesh = new CModelMesh(pd3dDevice, pd3dCommandList, "Models/FlyerPlayership.bin");
    CRotatingObject* pRotateAirplainModel = NULL;
    pRotateAirplainModel = new CRotatingObject();
    pRotateAirplainModel->SetMesh(pAirModelMesh);
    pRotateAirplainModel->SetPosition(20.f, 0.f, 0.f);
    pRotateAirplainModel->SetRotationAxis(XMFLOAT3(1.0f, 0.0f, 0.0f));
    pRotateAirplainModel->SetRotationSpeed(0.0f);
    pRotateAirplainModel->SelectObjectRender(true);
    m_nObjects = m_nObjects + 1;
    
    // 오브젝트에 대한 리스트 생성
    m_ppObjects = new CGameObject * [m_nObjects];
    m_ppObjects[0] = pRotatingObject;                                           // Set한 오브젝트 값을 배열에 넣고 배열 위치를 다음으로...
    m_ppObjects[1] = pRotateTriangle;
    m_ppObjects[2] = pRotateAirplain;
    m_ppObjects[3] = pRotateSphere;
    m_ppObjects[4] = pRotateAirplainModel;
    CreateShaderVariables(pd3dDevice, pd3dCommandList);                         // 만든 애들에 대한 상수 버퍼를 만듦
}

void CObjectsShader::AnimateObjects(float fTimeElapsed)
{
    for (int j = 0; j < m_nObjects; j++)                                        // 모든 오브젝트에 대해
    {
        m_ppObjects[j]->Animate(fTimeElapsed);                                  // 반복문을 돌면서 Animate 작업을 처리
    }
}

void CObjectsShader::ReleaseObjects()
{
    if (m_ppObjects)                                                            // 아까 만든 오브젝트를
    {
        for (int j = 0; j < m_nObjects; j++)                                    // 아까 만든 오브젝트 배열들의 개수만큼
        {
            if (m_ppObjects[j]) delete m_ppObjects[j];                          // 해당배열에 오브젝트가 있으면 제거
        }
        delete[] m_ppObjects;                                                   // 담았던 틀도 깔끔하게 지운다.
    }
}


// IA Shader
D3D12_INPUT_LAYOUT_DESC CObjectsShader::CreateInputLayout()
{
    // 위에 있는 이전 CreateInputLayout()과 코드 진행이 동일하다.

    UINT nInputElementDescs = 2;
    D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

    pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
    pd3dInputElementDescs[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

    D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
    d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
    d3dInputLayoutDesc.NumElements = nInputElementDescs;

    return (d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CObjectsShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob)
{
    // 위에 있는 이전 CreateVertexShader()과 코드 진행이 동일하다.
    return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSDiffused", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CObjectsShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob)
{
    // 위에 있는 이전 CreatePixelShader()과 코드 진행이 동일하다.
    return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSDiffused", "ps_5_1", ppd3dShaderBlob));
}

// Create Shader
void CObjectsShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
    m_nPipelineStates = 1;
    m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

    // 위에 만들어진 Create Shader가 있으니 굳이 새로 코드를 짤 필요없이, 만든놈을 호출한다.
    CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
}

// Release Upload Buffer
void CObjectsShader::ReleaseUploadBuffers()
{
    if (m_ppObjects)
    {
        for (int j = 0; j < m_nObjects; j++) m_ppObjects[j]->ReleaseUploadBuffers();    // 잊지 않고 Upload Buffer도 지워준다.
    }
}

// Render
void CObjectsShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    // 위에 만들어둔 Render를 재활용 한다.
    //현재 게임 객체를 렌더링 파이프라인에 설정한다.
    CShader::Render(pd3dCommandList, pCamera);

    for (int j = 0; j < m_nObjects; j++)
    {
        if (m_ppObjects[j])
        {
            if (m_ppObjects[j]->IsObjectRender() == true)
            {
                //현재 게임 객체를 렌더링한다.
                m_ppObjects[j]->Render(pd3dCommandList, pCamera);                   // 오브젝트들을 순서대로 랜더링 해준다.
            }
        }
    }
}

// Picking!Picking!Picking!Picking!Picking!Picking!Picking!Picking!Picking!Picking!Picking!Picking!Picking!Picking!Picking!Picking!Picking!Picking!Picking!
CGameObject* CObjectsShader::PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float* pfNearHitDistance)
{
    int nIntersected = 0;
    *pfNearHitDistance = FLT_MAX;
    float fHitDistance = FLT_MAX;
    CGameObject* pSelectedObject = NULL;
    for (int j = 0; j < m_nObjects; j++)
    {
        nIntersected = m_ppObjects[j]->PickObjectByRayIntersection(xmf3PickPosition, xmf4x4View, &fHitDistance);
        if ((nIntersected > 0) && (fHitDistance < *pfNearHitDistance))
        {
            *pfNearHitDistance = fHitDistance;
            pSelectedObject = m_ppObjects[j];
        }
    }
    return(pSelectedObject);
}
