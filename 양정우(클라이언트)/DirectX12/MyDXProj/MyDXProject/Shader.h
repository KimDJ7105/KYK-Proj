#pragma once
#include "stdafx.h"

#include "Object.h"
#include "Camera.h"


// Shader �ҽ� �ڵ带 �������ϰ� �׷��Ƚ� ���� ��ü�� ����
class CShader
{
public:
	CShader();
	virtual ~CShader();

private:
	int m_nReferences = 0;

public:
	// Count
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }
	
	// Create Drawing Pipeline
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();														// �Է� ���̾ƿ��� ���������ο��� �˸��� ����
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	// Shader
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);
	D3D12_SHADER_BYTECODE CompileShaderFromFile(
		WCHAR* pszFileName,
		LPCSTR pszShaderName,
		LPCSTR pszShaderProfile,
		ID3DBlob** ppd3dShaderBlob);
	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dRootSignature);

	// Camera�� ���� �Լ���
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World);

	// Render
	virtual void OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera *pCamera);

protected:
	// Pipeline ���� ��ü���� ����Ʈ(�迭)�̴�
	ID3D12PipelineState **m_ppd3dPipelineStates = NULL;
	int m_nPipelineStates = 0;
};


// Diffused Shader
class CPlayerShader : public CShader
{
public:
	CPlayerShader();
	virtual ~CPlayerShader();
	
	// IA
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();														// �Է� ���̾ƿ��� ���������ο��� �˸��� ����

	// Shader
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);

	// Create Shader
	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature);
};

// ���ٴ� ���鶧 ���� ���̴�
class CFieldShader : public CShader
{
public:
	CFieldShader();
	virtual ~CFieldShader();

	// IA
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();														// �Է� ���̾ƿ��� ���������ο��� �˸��� ����

	// Shader
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);

	// Create Shader
	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature);
};



// Obejct Shader - For the many Object (Optimization)
class CObjectsShader : public CShader
{
public:

	// ������ ��° ���� ���� ����ϴ�.
	// ���� �ٸ� Shader������ GameFramework������ �� �� �ִ� �Լ����� ���δ�.

	CObjectsShader();
	virtual ~CObjectsShader();

	// Object
	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void ReleaseObjects();

	// IA Shader
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);

	// Create Shader
	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature);

	// Release Upload Buffer
	virtual void ReleaseUploadBuffers();

	// Render
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	// Picking
	//���̴��� ���ԵǾ� �ִ� ��� ���� ��ü�鿡 ���� ���콺 ��ŷ�� �����Ѵ�.
	virtual CGameObject *PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float* pfNearHitDistance);

protected:
	CGameObject** m_ppObjects = NULL;
	int m_nObjects = 0;
};