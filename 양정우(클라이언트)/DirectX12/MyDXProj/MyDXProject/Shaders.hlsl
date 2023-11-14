// Game Object�� ������ ���� Constant Buffer�� ����
cbuffer cbGameObjectInfo : register(b0)                 // Constant Buffer�� b0�� ���ε��� �Ѵ�.
{
    matrix gmtxWorld : packoffset(c0);                  // 4x4matrix�� ù��° ��� ����(C0)�� �����صд�.
};

// Camera�� ������ ���� Constant Buffer�� ����
cbuffer cbCameraInfo : register(b1)                     // Constant Buffer�� b1�� ���ε��� �Ѵ�.
{
    matrix gmtxView : packoffset(c0);                   // ù��° ��� ����(C0)�� �����صд�.
    matrix gmtxProjection : packoffset(c4);             // �׹�° ��� ����(C4)�� �����صд�.
};
// Camera�� ����鼭 ���� �� ���۸� �߰��Ͽ���.

// Vertex Shader�� �Է��� ���� ����ü ����
struct VS_INPUT
{
    float3 position : POSITION;
    float4 color : COLOR;
};

// Vertex Shader�� ���(Pixel Shader�� �Է�)�� ���� ����ü ����
struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

// Vertex Shader�� ����
VS_OUTPUT VSDiffused(VS_INPUT input)
{
    VS_OUTPUT output;
    
    //// ������ ��ġ ���ʹ� ������ǥ��� ǥ���Ǿ� �����Ƿ� ��ȯ���� �ʰ� �״�� ����Ѵ�
    //output.position = float4(input.position, 1.0f);
    // ������ Camera�� ���� �̻�, ���̻� ������ǥ��� ǥ���Ǿ����� ��������
    // ������ ��ȯ(World, Camera, Projection)�� ����� �Ѵ�.
    output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), gmtxProjection);
    
    // �ԷµǴ� �ȼ��� ����(Rasterizer �ܰ迡�� �����Ͽ� ���� ����)�� �״�� ���
    output.color = input.color;
    
    return (output);

}

// Pixel Shader�� �����Ѵ�.
float4 PSDiffused(VS_OUTPUT input) : SV_TARGET
{
    // �ԷµǴ� �ȼ��� ������ �״�� OM�ܰ�(Render Target)�� ���
    return (input.color);
}