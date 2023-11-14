// Game Object의 정보를 위한 Constant Buffer를 선언
cbuffer cbGameObjectInfo : register(b0)                 // Constant Buffer를 b0에 바인딩을 한다.
{
    matrix gmtxWorld : packoffset(c0);                  // 4x4matrix를 첫번째 상수 버퍼(C0)에 저장해둔다.
};

// Camera의 정보를 위한 Constant Buffer를 선언
cbuffer cbCameraInfo : register(b1)                     // Constant Buffer를 b1에 바인딩을 한다.
{
    matrix gmtxView : packoffset(c0);                   // 첫번째 상수 버퍼(C0)에 저장해둔다.
    matrix gmtxProjection : packoffset(c4);             // 네번째 상수 버퍼(C4)에 저장해둔다.
};
// Camera를 만들면서 위의 두 버퍼를 추가하였다.

// Vertex Shader의 입력을 위한 구조체 선언
struct VS_INPUT
{
    float3 position : POSITION;
    float4 color : COLOR;
};

// Vertex Shader의 출력(Pixel Shader의 입력)을 위한 구조체 선언
struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

// Vertex Shader를 정의
VS_OUTPUT VSDiffused(VS_INPUT input)
{
    VS_OUTPUT output;
    
    //// 정점의 위치 벡터는 투영좌표계로 표현되어 있으므로 변환하지 않고 그대로 출력한다
    //output.position = float4(input.position, 1.0f);
    // 하지만 Camera를 만든 이상, 더이상 투영좌표계로 표현되어있지 않음으로
    // 정점을 변환(World, Camera, Projection)을 해줘야 한다.
    output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), gmtxProjection);
    
    // 입력되는 픽셀의 색상(Rasterizer 단계에서 보간하여 얻은 색상)을 그대로 출력
    output.color = input.color;
    
    return (output);

}

// Pixel Shader를 정의한다.
float4 PSDiffused(VS_OUTPUT input) : SV_TARGET
{
    // 입력되는 픽셀의 색상을 그대로 OM단계(Render Target)로 출력
    return (input.color);
}