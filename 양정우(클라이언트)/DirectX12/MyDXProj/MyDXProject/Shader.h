#pragma once
#include "stdafx.h"

#include "Object.h"
#include "Camera.h"


// Shader 소스 코드를 컴파일하고 그래픽스 상태 객체를 생성
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
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();														// 입력 레이아웃을 파이프라인에게 알리기 위함
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

	// Camera를 위한 함수들
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World);

	// Render
	virtual void OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera *pCamera);

protected:
	// Pipeline 상태 객체들의 리스트(배열)이다
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
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();														// 입력 레이아웃을 파이프라인에게 알리기 위함

	// Shader
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);

	// Create Shader
	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature);
};

// 땅바닥 만들때 쓰는 쉐이더
class CFieldShader : public CShader
{
public:
	CFieldShader();
	virtual ~CFieldShader();

	// IA
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();														// 입력 레이아웃을 파이프라인에게 알리기 위함

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

	// 구조가 어째 위와 많이 비슷하다.
	// 위의 다른 Shader설정과 GameFramework에서도 볼 수 있는 함수들이 보인다.

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
	//셰이더에 포함되어 있는 모든 게임 객체들에 대한 마우스 픽킹을 수행한다.
	virtual CGameObject *PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float* pfNearHitDistance);

protected:
	CGameObject** m_ppObjects = NULL;
	int m_nObjects = 0;
};