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
// IA�� ���� ������ ������ �˷��ֱ� ���� ����ü�� ��ȯ
D3D12_INPUT_LAYOUT_DESC CShader::CreateInputLayout()                                        // �Է� ���̾ƿ��� ���������ο��� �˸��� ����
{
    D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
    d3dInputLayoutDesc.pInputElementDescs = NULL;

    d3dInputLayoutDesc.NumElements = 0;

    return (d3dInputLayoutDesc);
}

// Rasterizer ���¸� �����ϱ� ���� ����ü�� ��ȯ
D3D12_RASTERIZER_DESC CShader::CreateRasterizerState()
{
    D3D12_RASTERIZER_DESC d3dRasterizerDesc;
    ::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
    d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;                                     // �������� ������ ���� �׸� ���, SOLID - �Ǽ����� ä��� / WIREFRAME - ��(Edge)�� �׸�
    d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;                                      // �޸� ����, �ո鸸 ������
    d3dRasterizerDesc.FrontCounterClockwise = FALSE;                                        // �ݽð�� �׸��̴ϱ�? = �ƴϿ�
    d3dRasterizerDesc.DepthBias = 0;                                                        // �׸��� ���帧 �ذ�� ����
    d3dRasterizerDesc.DepthBiasClamp = 0.0f;                                                // �׸��� ���帧 �ذ�� ����
    d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;                                          // �׸��� ���帧 �ذ�� ����
    d3dRasterizerDesc.DepthClipEnable = TRUE;                                               // True��� ������ �� ���̰��� Ŭ���ؼ� �޸��� �Ⱥ��δ�.
    d3dRasterizerDesc.MultisampleEnable = FALSE;                                            // MSAA �Ⱦ�
    d3dRasterizerDesc.AntialiasedLineEnable = FALSE;                                        // ������ �Ǵ� ������ MSAA����
    d3dRasterizerDesc.ForcedSampleCount = 0;                                                // ���߻��ø� ����Ҷ� ���ü� ����
    d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;       // �����������Ͷ������̼��� �������� ��������������

    return (d3dRasterizerDesc);
}

// Blending ���¸� �����ϱ� ���� ����ü ��ȯ.
D3D12_BLEND_DESC CShader::CreateBlendState()
{
    D3D12_BLEND_DESC d3dBlendDesc;
    ::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
    d3dBlendDesc.AlphaToCoverageEnable = FALSE;                                             // ���� Ŀ������ ��Ȱ��ȭ
    d3dBlendDesc.IndependentBlendEnable = FALSE;                                            // Render Target�� �������� ���� ������ �ұ ���� ����, False�̹Ƿ� ��� Render Target�� ������ ���¸� ������
    d3dBlendDesc.RenderTarget[0].BlendEnable = FALSE;                                       // 0�� Blend ��������(�ִ� 8��)
    d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;                                     // �� ���� Ȱ��ȭ ����
    d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;                                // �ҽ� ������ �״�� ����ϰ� ��� ������ ������� ����
    d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;                              // ��
    d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;                              // ��
    d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;                           // ��
    d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;                         // ��
    d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;                         // ���� ���� �۾� �� ������ ����
    d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;                             // ������ ��Ȱ��ȭ
    d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;      // Render Target ���� ���� ����ũ, ��� ����ä�ο����� ���Ⱑ ���

    return(d3dBlendDesc);
}

// Depth Stencil �˻縦 ���� ���¸� �����ϱ� ���� ����ü�� ��ȯ
D3D12_DEPTH_STENCIL_DESC CShader::CreateDepthStencilState()
{
    D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
    ::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
    d3dDepthStencilDesc.DepthEnable = TRUE;                                                 // ���� �׽�Ʈ�� Ȱ��ȭ
    d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;                        // ���̹��� ��ϼ���, ��� �ȼ��� ���� ���� ���
    d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;                             // �����׽��� ���Լ�, �ȼ��� ���̰� ���� ���̺��� �������� ���
    d3dDepthStencilDesc.StencilEnable = FALSE;                                              // ���Ľ��� �Ⱦ��Ŵϱ�.
    d3dDepthStencilDesc.StencilReadMask = 0x00;                                             // �� �Ʒ���
    d3dDepthStencilDesc.StencilWriteMask = 0x00;                                            // ����
    d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;                    // ��
    d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;               // ��
    d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;                    // ��
    d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;                // ��
    d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;                     // ��
    d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;                // ��
    d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;                     // ��
    d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;                 // �Ѵ�

    return (d3dDepthStencilDesc);
}

// Vertex Shader ����Ʈ �ڵ带 ����(������)�Ѵ�
D3D12_SHADER_BYTECODE CShader::CreateVertexShader(ID3DBlob** ppd3dShaderBlob)
{
    D3D12_SHADER_BYTECODE d3dShaderByteCode;
    d3dShaderByteCode.BytecodeLength = 0;
    d3dShaderByteCode.pShaderBytecode = NULL;

    return(d3dShaderByteCode);
}

// Pixel Shader ����Ʈ �ڵ带 ����(������)�Ѵ�
D3D12_SHADER_BYTECODE CShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob)
{
    D3D12_SHADER_BYTECODE d3dShaderByteCode;
    d3dShaderByteCode.BytecodeLength = 0;
    d3dShaderByteCode.pShaderBytecode = NULL;

    return(d3dShaderByteCode);
}

// ������ ������ �� �̿��ؼ� Shader �ҽ� �ڵ带 ������ �Ͽ��� ����Ʈ �ڵ� ����ü�� ��ȯ�Ѵ�.
D3D12_SHADER_BYTECODE CShader::CompileShaderFromFile(WCHAR* pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob** ppd3dShaderBlob)
{
    UINT nCompileFlags = 0;
#if defined(_DEBUG)
    nCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    // Scene.cpp���� ���� Shaders.hlsl ���� ������ �Լ��̴�.
    ::D3DCompileFromFile(pszFileName, NULL, NULL, pszShaderName, pszShaderProfile, nCompileFlags, 0, ppd3dShaderBlob, NULL);

    D3D12_SHADER_BYTECODE d3dShaderByteCode;
    d3dShaderByteCode.BytecodeLength = (*ppd3dShaderBlob)->GetBufferSize();
    d3dShaderByteCode.pShaderBytecode = (*ppd3dShaderBlob)->GetBufferPointer();

    return(d3dShaderByteCode);
}

// Graphics Pipeline�� ���� ��ü�� ����
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
    d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;                            // Camera�� ����鼭 Flag�� �߰�.

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


// Camera�� ���� �Լ���
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
    // Pipeline�� Graphics ���°�ü�� ����
    //���� ���� ��ü�� ������ ���������ο� �����Ѵ�.
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

D3D12_INPUT_LAYOUT_DESC CPlayerShader::CreateInputLayout()                                                                            // �Է� ���̾ƿ��� ���������ο��� �˸��� ����
{
    UINT nInputElementDescs = 2;
    D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];                                 // �Է� ���̾ƿ�(���� ���� ���� �ϳ��� ������ ���� ����) ����

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

// ���ٴ� ���̴�
CFieldShader::CFieldShader()
{
}

CFieldShader::~CFieldShader()
{
}

D3D12_INPUT_LAYOUT_DESC CFieldShader::CreateInputLayout()                                                                            // �Է� ���̾ƿ��� ���������ο��� �˸��� ����
{
    UINT nInputElementDescs = 2;
    D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];                                 // �Է� ���̾ƿ�(���� ���� ���� �ϳ��� ������ ���� ����) ����

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
    m_nObjects = 0; // ������Ʈ �迭 ����

    // ����x����x���̰� 12x12x12�� ������ü �޽��� �����Ѵ�.
    // CubeMesh��� ���� �̷��� �����~
    CCubeMeshDiffused *pCubeMesh = new CCubeMeshDiffused(pd3dDevice, pd3dCommandList, 12.0f, 12.0f, 12.0f);
    CRotatingObject* pRotatingObject = NULL;
    pRotatingObject = new CRotatingObject();                        // ���ο� ������Ʈ ����
    pRotatingObject->SetMesh(pCubeMesh);                            // ���� ������Ʈ�� ���� ��� ����
    pRotatingObject->SetPosition(0.f, 0.f, 10.f);                    // ��ġ�� �̷��̷���
    pRotatingObject->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));   // ���� �̷��̷���
    pRotatingObject->SetRotationSpeed(10.0f);                       // ���R�� �ӵ��� �̷��̷���
    pRotatingObject->SelectObjectRender(true);
    m_nObjects = m_nObjects + 1;

    // ���� �ѹ� ���� �ﰢ��
    CTriangleMesh* pTriangleMesh = new CTriangleMesh(pd3dDevice, pd3dCommandList);
    CRotatingObject* pRotateTriangle = NULL;
    pRotateTriangle = new CRotatingObject();
    pRotateTriangle->SetMesh(pTriangleMesh);
    pRotateTriangle->SetPosition(0.f, 15.f, 0.f);
    pRotateTriangle->SetRotationAxis(XMFLOAT3(1.0f, 1.0f, 1.0f));
    pRotateTriangle->SetRotationSpeed(50.f);
    pRotateTriangle->SelectObjectRender(true);
    m_nObjects = m_nObjects + 1;
    
    // ���ۻ��� ���� �÷��̾� ��ü�� �ƴ� ����� �޽�
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

    // "9""��""Sphere"
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
    
    // ������Ʈ�� ���� ����Ʈ ����
    m_ppObjects = new CGameObject * [m_nObjects];
    m_ppObjects[0] = pRotatingObject;                                           // Set�� ������Ʈ ���� �迭�� �ְ� �迭 ��ġ�� ��������...
    m_ppObjects[1] = pRotateTriangle;
    m_ppObjects[2] = pRotateAirplain;
    m_ppObjects[3] = pRotateSphere;
    m_ppObjects[4] = pRotateAirplainModel;
    CreateShaderVariables(pd3dDevice, pd3dCommandList);                         // ���� �ֵ鿡 ���� ��� ���۸� ����
}

void CObjectsShader::AnimateObjects(float fTimeElapsed)
{
    for (int j = 0; j < m_nObjects; j++)                                        // ��� ������Ʈ�� ����
    {
        m_ppObjects[j]->Animate(fTimeElapsed);                                  // �ݺ����� ���鼭 Animate �۾��� ó��
    }
}

void CObjectsShader::ReleaseObjects()
{
    if (m_ppObjects)                                                            // �Ʊ� ���� ������Ʈ��
    {
        for (int j = 0; j < m_nObjects; j++)                                    // �Ʊ� ���� ������Ʈ �迭���� ������ŭ
        {
            if (m_ppObjects[j]) delete m_ppObjects[j];                          // �ش�迭�� ������Ʈ�� ������ ����
        }
        delete[] m_ppObjects;                                                   // ��Ҵ� Ʋ�� ����ϰ� �����.
    }
}


// IA Shader
D3D12_INPUT_LAYOUT_DESC CObjectsShader::CreateInputLayout()
{
    // ���� �ִ� ���� CreateInputLayout()�� �ڵ� ������ �����ϴ�.

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
    // ���� �ִ� ���� CreateVertexShader()�� �ڵ� ������ �����ϴ�.
    return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSDiffused", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CObjectsShader::CreatePixelShader(ID3DBlob** ppd3dShaderBlob)
{
    // ���� �ִ� ���� CreatePixelShader()�� �ڵ� ������ �����ϴ�.
    return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSDiffused", "ps_5_1", ppd3dShaderBlob));
}

// Create Shader
void CObjectsShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
    m_nPipelineStates = 1;
    m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

    // ���� ������� Create Shader�� ������ ���� ���� �ڵ带 © �ʿ����, ������� ȣ���Ѵ�.
    CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
}

// Release Upload Buffer
void CObjectsShader::ReleaseUploadBuffers()
{
    if (m_ppObjects)
    {
        for (int j = 0; j < m_nObjects; j++) m_ppObjects[j]->ReleaseUploadBuffers();    // ���� �ʰ� Upload Buffer�� �����ش�.
    }
}

// Render
void CObjectsShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    // ���� ������ Render�� ��Ȱ�� �Ѵ�.
    //���� ���� ��ü�� ������ ���������ο� �����Ѵ�.
    CShader::Render(pd3dCommandList, pCamera);

    for (int j = 0; j < m_nObjects; j++)
    {
        if (m_ppObjects[j])
        {
            if (m_ppObjects[j]->IsObjectRender() == true)
            {
                //���� ���� ��ü�� �������Ѵ�.
                m_ppObjects[j]->Render(pd3dCommandList, pCamera);                   // ������Ʈ���� ������� ������ ���ش�.
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
