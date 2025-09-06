#include "common/cameraUniforms.glsl"
#include "common/lightSources.glsl"

struct Material {
  sampler2D diffuse;
  sampler2D specular;
  sampler2D emission;
  float shininess;
};

uniform Material uMaterial;
uniform float uDebugBiasMultiplier;

uniform sampler2DArrayShadow uDirectionalLightShadowMaps;
uniform sampler2DArrayShadow uPointLightShadowMaps;
uniform sampler2DArrayShadow uSpotlightShadowMaps;

#if HAS_GEOMETRY_SHADER
in GS_OUT {
  vec3 position;
  vec3 normal;
  vec2 texCoord;
} fsIn;
#else
in VS_OUT {
  vec3 position;
  vec3 normal;
  vec2 texCoord;
} fsIn;
#endif

out vec4 outColor;

float diffuse(in vec3 normal, in vec3 lightDirection) {
  return max(dot(normal, lightDirection), 0);
}

float specular(in vec3 normal, in vec3 lightDirection) {
  vec3 viewDirection = normalize(uCamera.position - fsIn.position);
  vec3 halfwayDirection = normalize(lightDirection + viewDirection);
  float angularDifference = max(dot(normal, halfwayDirection), 0);
  return pow(angularDifference, uMaterial.shininess);
}

float shadow(vec4 fragPosLightSpace, float cosTheta, in sampler2DArrayShadow shadowMap, uint lightIndex) {
  vec3 projectedPosition = fragPosLightSpace.xyz * 0.5 + 0.5;
  if (projectedPosition.z > 1) {
    return 0;
  }

  float bias = max(0.005 * (1 - cosTheta), 0.001)/* + uDebugBiasMultiplier*/;
//  bias = 0;
  float w = projectedPosition.z - bias;
  vec2 texelSize = 1.0 / textureSize(shadowMap, 0).xy;
  const mat3 falloffKernel = mat3(0.25, 0.50, 0.25,
                                  0.50, 1.00, 0.50,
                                  0.25, 0.50, 0.25) * 0.25;

  float ret = 0;
  for (int x = -1; x <= 1; ++x) {
    for (int y = -1; y <= 1; ++y) {
      vec4 texCoords = vec4(projectedPosition.xy + vec2(x, y) * texelSize, lightIndex, w);
      float shade = texture(shadowMap, texCoords);
      float fallOff = falloffKernel[x + 1][y + 1];
      ret += shade * fallOff;
    }
  }
  return ret;
}

LightColors directionalLight(uint lightIndex, vec3 normal) {
  DirectionalLight light = uDirectionalLights.sources[lightIndex];

  vec3 lightDirection = normalize(-light.direction);
  vec4 fragPosLightSpace = light.view.projection * light.view.view * vec4(fsIn.position, 1);
  fragPosLightSpace.xyz /= fragPosLightSpace.w;
  float visibility = 1 - shadow(fragPosLightSpace, dot(normal, lightDirection), uDirectionalLightShadowMaps, lightIndex);

  LightColors colors;
  colors.ambient = light.colors.ambient;
  colors.diffuse = light.colors.diffuse * visibility * diffuse(normal, lightDirection);
  colors.specular = light.colors.specular * visibility * specular(normal, lightDirection);
  return colors;
}

LightColors pointLight(uint lightIndex, vec3 normal) {
  PointLight light = uPointLights.sources[lightIndex];

  vec3 lightDirection = normalize(light.position - fsIn.position);
  float distance = distance(light.position, fsIn.position);
  float attenuation = 1 / (light.constant +
                           light.linear * distance +
                           light.quadratic * distance * distance);

  LightColors colors;
  colors.ambient = light.colors.ambient * attenuation;
  colors.diffuse = light.colors.diffuse * attenuation * diffuse(normal, lightDirection);
  colors.specular = light.colors.specular * attenuation * specular(normal, lightDirection);
  return colors;
}

LightColors spotlight(uint lightIndex, vec3 normal) {
  Spotlight light = uSpotlights.sources[lightIndex];

  float distance = distance(light.position, fsIn.position);
  vec3 lightDirection = normalize(light.position - fsIn.position);
  float theta = dot(lightDirection, normalize(-light.direction));
  float visibility = step(light.outerCutOff, theta);
  float epsilon = light.cutOff - light.outerCutOff;
  float intensity = clamp((theta - light.outerCutOff) / epsilon, 0, 1);

  // TODO - fix spotlight shadows
  vec4 fragPosLightSpace = light.view.projection * light.view.view * vec4(fsIn.position, 1);
//  fragPosLightSpace.xyz /= fragPosLightSpace.w;
//  fragPosLightSpace.z = 1 - fragPosLightSpace.z;
//  fragPosLightSpace.z = linearizeDepth(fragPosLightSpace.z) / light.view.zMax;
  if (visibility == 1) {
    visibility *= 1 - shadow(fragPosLightSpace, dot(normal, lightDirection), uSpotlightShadowMaps, lightIndex);
  }

  LightColors colors;
  colors.ambient = light.colors.ambient * visibility * intensity;
  colors.diffuse = light.colors.diffuse * visibility * intensity * diffuse(normal, lightDirection);
//  colors.diffuse = fragPosLightSpace.zzz * 0.1;
//  colors.diffuse = normal * 0.5 + 0.5;
  colors.specular = light.colors.specular * visibility * intensity * specular(normal, lightDirection);
  return colors;
}

void main() {
  vec4 materialDiffuse = texture(uMaterial.diffuse, fsIn.texCoord);
  vec3 materialSpecular = texture(uMaterial.specular, fsIn.texCoord).rgb;
  vec3 materialEmission = texture(uMaterial.emission, fsIn.texCoord).rgb;

  vec3 normal = normalize(fsIn.normal);
  if (!gl_FrontFacing) {
    normal *= -1;
  }

  vec3 combinedAmbient = vec3(0);
  vec3 combinedDiffuse = vec3(0);
  vec3 combinedSpecular = vec3(0);

  for (uint i = 0; i < uDirectionalLights.count; ++i) {
    LightColors directionalLightColors = directionalLight(i, normal);
    combinedAmbient += directionalLightColors.ambient;
    combinedDiffuse += directionalLightColors.diffuse;
    combinedSpecular += directionalLightColors.specular;
  }

  for (uint i = 0; i < uPointLights.count; ++i) {
    LightColors pointLightColors = pointLight(i, normal);
    combinedAmbient += pointLightColors.ambient;
    combinedDiffuse += pointLightColors.diffuse;
    combinedSpecular += pointLightColors.specular;
  }

  for (uint i = 0; i < uSpotlights.count; ++i) {
    LightColors spotlightColors = spotlight(i, normal);
    combinedAmbient += spotlightColors.ambient;
    combinedDiffuse += spotlightColors.diffuse;
    combinedSpecular += spotlightColors.specular;
  }

  vec4 result = materialDiffuse * vec4(combinedAmbient, 1) +
                materialDiffuse * vec4(combinedDiffuse, 1) +
                vec4(materialSpecular * combinedSpecular, 0) +
                vec4(materialEmission * step(1, 1 - materialSpecular), 0);
  outColor = result;
}
