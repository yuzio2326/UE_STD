
cbuffer FObjectUniformBuffer : register(b0)
{
    matrix WorldMatrix;
    matrix WorldInverseTransposeMatrix;
};

cbuffer FSceneUniformBuffer : register(b1)
{
    float3 EyePosition;
    int NumRadianceMipLevels;
    matrix ViewMatrix;
    matrix ProjectionMatrix;
    matrix ViewProjectionMatrix;
}

cbuffer FLightShaderParameters : register(b2)
{
    float4 LightColor;
    float3 LightDirection;
    float FLightShaderParameters_Padding;
}

Texture2D<float4> BaseColorTexture : register(t0);
Texture2D<float3> NormalTexture : register(t1);
Texture2D<float4> MetallicTexture : register(t2);
Texture2D<float4> RoughnessTexture : register(t3);
Texture2D<float4> AmbientOcclusionTexture : register(t4);
sampler TextureSampler : register(s0);

TextureCube<float3> RadianceTexture : register(t5);
TextureCube<float3> IrradianceTexture : register(t6);
sampler IBLSampler : register(s1);

struct FVSInput
{
    float3 Position : ATTRIBUTE0;
    float3 Normal : ATTRIBUTE1;
    float2 UV : ATTRIBUTE2;
};

struct FVSOutput
{
    float4 SVPosition : SV_Position;
    float4 PositionWS : POSITIONW;
    float3 NormalWS : NORMALW;
    float2 UV : TEXCOORD0;
};

static const float PI = 3.14159265f;
static const float EPSILON = 1e-6f;

// Christian Schuler, "Normal Mapping without Precomputed Tangents", ShaderX 5, Chapter 2.6, pp. 131-140
// See also follow-up blog post: http://www.thetenthplanet.de/archives/1180
float3x3 CalculateTBN(float3 p, float3 n, float2 tex)
{
    float3 dp1 = ddx(p);
    float3 dp2 = ddy(p);
    float2 duv1 = ddx(tex);
    float2 duv2 = ddy(tex);

    float3x3 M = float3x3(dp1, dp2, cross(dp1, dp2));
    float2x3 inverseM = float2x3(cross(M[1], M[2]), cross(M[2], M[0]));
    float3 t = normalize(mul(float2(duv1.x, duv2.x), inverseM));
    float3 b = normalize(mul(float2(duv1.y, duv2.y), inverseM));
    return float3x3(t, b, n);
}

float3 TwoChannelNormalX2(float2 normal)
{
    float2 xy = 2.0f * normal - 1.0f;
    float z = sqrt(1 - dot(xy, xy));
    return float3(xy.x, xy.y, z);
}

float3 BiasX2(float3 normal)
{
    return 2.0f * normal - 1.0f;
}

float3 PeturbNormal(float3 localNormal, float3 position, float3 normal, float2 texCoord)
{
    const float3x3 TBN = CalculateTBN(position, normal, texCoord);
    return normalize(mul(localNormal, TBN));
}

// 슐리크의 프레넬 근사
// https://en.wikipedia.org/wiki/Schlick%27s_approximation
float3 Fresnel_Shlick(in float3 f0, in float3 f90, in float x)
{
    return f0 + (f90 - f0) * pow(1.f - x, 5.f);
}

// Burley B. "Disney에서의 물리 기반 셰이딩"
// SIGGRAPH 2012 강의: 영화 및 게임 제작에서의 실용적인 물리 기반 셰이딩, 2012.
float Diffuse_Burley(in float NdotL, in float NdotV, in float LdotH, in float roughness)
{
    float fd90 = 0.5f + 2.f * roughness * LdotH * LdotH;
    return Fresnel_Shlick(1, fd90, NdotL).x * Fresnel_Shlick(1, fd90, NdotV).x;
}

// GGX 스펙큘러 D (정규 분포)
// https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf
float Specular_D_GGX(in float alpha, in float NdotH)
{
    const float alpha2 = alpha * alpha;
    const float lower = (NdotH * NdotH * (alpha2 - 1)) + 1;
    return alpha2 / max(EPSILON, PI * lower * lower);
}

// 슐리크-스미스 스펙큘러 G (가시성) with Hable's LdotH 최적화
// http://www.cs.virginia.edu/~jdl/bib/appearance/analytic%20models/schlick94b.pdf
// http://graphicrants.blogspot.se/2013/08/specular-brdf-reference.html
float G_Shlick_Smith_Hable(float alpha, float LdotH)
{
    return rcp(lerp(LdotH * LdotH, 1, alpha * alpha * 0.25f));
}

// 마이크로페이스트 기반 BRDF.
//
// alpha:           러프니스 값을 제곱한 값입니다. 디즈니의 물리 기반 렌더링(PBR) 모델에서 사용되는 방식입니다.
//
// specularColor:   F0 반사율 값입니다. 비금속의 경우 0.04, 금속의 경우 RGB 값입니다. 언리얼 엔진 4에서 사용하는 모델을 따릅니다.
//
// NdotV, NdotL, LdotH, NdotH: 벡터 간의 관계입니다.
//      N - 표면 법선 벡터
//      V - 뷰 벡터
//      L - 빛 벡터
//      H - 반각 벡터 (빛 벡터와 뷰 벡터의 반각)
float3 Specular_BRDF(in float alpha, in float3 specularColor, in float NdotV, in float NdotL, in float LdotH, in float NdotH)
{
    // Specular D (microfacet normal distribution) component
    float specular_D = Specular_D_GGX(alpha, NdotH);

    // Specular Fresnel
    float3 specular_F = Fresnel_Shlick(specularColor, 1, LdotH);

    // Specular G (visibility) component
    float specular_G = G_Shlick_Smith_Hable(alpha, LdotH);

    return specular_D * specular_F * specular_G;
}


float3 ACES_Tonemap(float3 color)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return saturate((color * (a * color + b)) / (color * (c * color + d) + e));
}

// Diffuse irradiance
// 확산 조도
float3 Diffuse_IBL(in float3 N)
{
    return ACES_Tonemap(IrradianceTexture.Sample(IBLSampler, N));
}

// 메탈릭이 높으면 이부분 영향이 커짐 (낮으면 Diffuse_IBL)
// 러프니스가 높으면 더 낮은 밉맵의 CubeMapTexture를 Sample
// Approximate specular image based lighting by sampling radiance map at lower mips 
// according to roughness, then modulating by Fresnel term. 
// 러프니스에 따라 낮은 mips에서 방사 맵을 샘플링하여 스펙큘러 이미지 기반 조명을 근사화하고, 
// 그런 다음 프레넬 항으로 변조합니다.
float3 Specular_IBL(in float3 N, in float3 V, in float lodBias)
{
    float mip = lodBias * NumRadianceMipLevels;
    float3 Dir = reflect(-V, N);
    // 언리얼 스타일로 View를 돌려둔걸 고려해서 DirectX 기준으로 돌려준다
    // 샘플 이미지가 DirectX축 기준으로 고려되어 있음
    // Origin
	// FPlane(0, 0, 1, 0),
	// FPlane(1, 0, 0, 0),
	// FPlane(0, 1, 0, 0),
    
    // Inverse
	// FPlane(0, 1, 0, 0),
	// FPlane(0, 0, 1, 0),
	// FPlane(1, 0, 0, 0),
    
    // Transpose
	// FPlane(0, 0, 1, 0),
	// FPlane(1, 0, 0, 0),
	// FPlane(0, 1, 0, 0),
    
    float3x3 RotationMatrix = float3x3(
        0, 0, 1,
        1, 0, 0,
        0, 1, 0
    );
    Dir = mul(Dir, RotationMatrix);
    
    float3 Color = RadianceTexture.SampleLevel(IBLSampler, Dir, mip);
    return ACES_Tonemap(Color);
}

// Disney 스타일의 물리 기반 렌더링을 표면에 적용하기:
//
// V, N: 눈과 표면의 법선 벡터
//
// lightColor: 방향성 조명의 색상 및 강도.
//
// lightDirection: 빛의 방향
float3 LightSurface(
    in float3 V, in float3 N,
    in float3 lightColor, in float3 lightDirection,
    in float3 albedo, in float roughness, in float metallic, in float ambientOcclusion)
{
    // 비금속 재질의 고정된 반사율 값
    static const float kSpecularCoefficient = 0.04;

    const float NdotV = saturate(dot(N, V));

    roughness = max(roughness, 0.0001f);
    // Burley 러프니스 바이어스
    const float alpha = roughness * roughness;

    // 기본 색상 블렌딩
    const float3 c_diff = lerp(albedo, float3(0, 0, 0), metallic) * ambientOcclusion;
    const float3 c_spec = lerp(kSpecularCoefficient, albedo, metallic) * ambientOcclusion;

    // 출력 색상 초기화
    float3 acc_color = 0;

    // 빛 값을 누적 계산
    {
        // 빛 벡터 (빛 쪽으로)
        const float3 L = normalize(lightDirection);

        // 반각 벡터
        const float3 H = normalize(L + V);

        // 내적 값 계산
        const float NdotL = saturate(dot(N, L));
        const float LdotH = saturate(dot(L, H));
        const float NdotH = saturate(dot(N, H));

        // 확산 및 스펙큘러 요소
        float diffuse_factor = Diffuse_Burley(NdotL, NdotV, LdotH, roughness);
        float3 specular = Specular_BRDF(alpha, c_spec, NdotV, NdotL, LdotH, NdotH);

        // 방향성 조명 누적
        acc_color += NdotL * lightColor * (((c_diff * diffuse_factor) + specular));
    }

    // 확산 조도 추가
    // 의미: 이 부분은 표면에 모든 방향에서 들어오는 빛의 총합을 계산합니다.
    // 실환경적 비유: 햇빛이 창문을 통해 들어오고, 방의 벽, 바닥, 천장을 균일하게 밝히는 것을 상상해보세요. 
    // 벽의 색상이 모든 방향에서 들어오는 빛의 영향을 받습니다.
    // 이 빛은 특정한 반사 없이 부드럽게 퍼지는 확산 빛입니다.
    float3 diffuse_env = Diffuse_IBL(N);
    acc_color += c_diff * diffuse_env;

    // 스펙큘러 조도 추가
    // 의미: 이 부분은 특정 방향(뷰 방향)에서 표면이 반사하는 빛의 양을 계산합니다.
    // 실환경적 비유: 같은 방에서 금속 거울과 유리창을 상상해보세요.
    // 
    // 금속 거울: 햇빛이 금속 거울에 부딪히면, 그 빛은 특정 방향으로 강하게 반사됩니다. 
    // 금속 거울의 반사 특성은 메탈릭 파라미터에 의해 결정되며, 금속의 고유 색상과 결합하여 반사됩니다. 
    // 표면의 매끄러움(러프니스)이 낮을수록 빛이 집중해서 반사되고, 러프니스가 높을수록 빛이 여러 방향으로 퍼져 반사됩니다.
    // 
    // 유리창: 햇빛이 유리창에 부딪히면, 반사된 빛은 주로 하이라이트 형태로 나타납니다. 
    // 유리창은 비금속성 표면으로, 러프니스가 낮을수록 하이라이트가 더 집중되고, 러프니스가 높을수록 빛이 여러 방향으로 퍼져 반사됩니다. 
    // 또한, 유리창은 확산 반사를 통해 주변 환경의 빛을 부드럽게 퍼뜨립니다.
    //
    // 러프니스(Roughness)는 표면의 거칠기나 매끄러움을 나타내는 값입니다. 
    // 러프니스 값은 0에서 1 사이의 값으로 표현되며, 표면이 얼마나 부드럽거나 거친지를 결정합니다.
    // 러프니스 값이 낮을 때 (0에 가까운 값):
    // - 표면이 매우 매끄럽고 반사가 선명하게 나타납니다.
    // - 표면은 빛을 매우 집중해서 반사합니다. 거울 같은 반사 효과가 나타납니다.
    // - 예시: 거울, 매끄러운 금속 표면, 유리창 등
    //
    // 러프니스 값이 높을 때 (1에 가까운 값):
    // - 표면이 매우 거칠고 반사가 분산됩니다.
    // - 표면은 빛을 여러 방향으로 퍼뜨려서 반사합니다. 반사가 희미하고 부드럽게 나타납니다.
    // - 예시: 거친 나무 표면, 매트 페인트, 무광 금속 표면 등
    //
    // 중간 러프니스 값:
    // - 표면은 중간 정도의 매끄러움과 거칠기를 가집니다.
    // - 반사는 부분적으로 집중되며, 나머지는 분산됩니다. 자연스러운 조명 효과가 나타납니다.
    // - 예시: 약간 광택이 있는 플라스틱, 중간 광택의 금속 표면 등
    //
    // 러프니스와 PBR의 관계:
    // - 확산(Diffuse) 반사: 러프니스 값이 높을수록 확산 반사가 증가합니다. 표면에 들어오는 빛이 여러 방향으로 퍼져 부드럽게 반사됩니다.
    // - 스펙큘러(Specular) 반사: 러프니스 값이 낮을수록 스펙큘러 반사가 강하게 나타납니다. 표면이 매끄러울수록 빛이 집중되어 반사됩니다.
    // - 프레넬 효과(Fresnel Effect): 입사각에 따라 반사율이 달라지며, 러프니스 값에 따라 프레넬 효과의 강도가 달라집니다. 매끄러운 표면일수록 프레넬 효과가 더 두드러집니다.
    //
    // 실생활 예시:
    // - 매끄러운 표면: 거울을 예로 들면, 빛이 거울 표면에 부딪힐 때 반사된 빛은 한 방향으로 집중되어 반사됩니다. 거울에 비친 모습이 선명하게 보이는 이유가 바로 낮은 러프니스 값 덕분입니다.
    // - 거친 표면: 무광 페인트가 칠해진 벽을 예로 들면, 빛이 벽에 부딪힐 때 반사된 빛은 여러 방향으로 퍼져 부드럽고 희미하게 반사됩니다. 이는 높은 러프니스 값 때문에 발생합니다.

    float3 specular_env = Specular_IBL(N, V, roughness);
    acc_color += c_spec * specular_env;

    return acc_color;
}
