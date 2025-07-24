#include "common/cameraUniforms.glsl"
#include "common/lightSources.glsl"

struct Material {
  sampler2D diffuse;
  sampler2D specular;
  sampler2D emission;
  float shininess;
};

uniform Material uMaterial;
uniform DirectionalLight uDirectionalLight;
uniform PointLight uPointLight;
uniform Spotlight uSpotlight;

in VS_OUT {
  vec3 position;
  vec3 normal;
  vec2 texCoord;
} fsIn;

out vec4 outColor;

float diffuse(in vec3 normal, in vec3 lightDirection) {
  return max(dot(normal, lightDirection), 0);
}

float specular(in vec3 normal, in vec3 lightDirection) {
  vec3 viewDirection = normalize(uCamera.position - fsIn.position);
  vec3 reflectDirection = reflect(-lightDirection, normal);
  float angularDifference = max(dot(viewDirection, reflectDirection), 0);
  return pow(angularDifference, uMaterial.shininess);
}

LightColors directionalLight(in DirectionalLight light, vec3 normal) {
  vec3 lightDirection = normalize(-light.direction);

  LightColors colors;
  colors.ambient = light.colors.ambient;
  colors.diffuse = light.colors.diffuse * diffuse(normal, lightDirection);
  colors.specular = light.colors.specular * specular(normal, lightDirection);
  return colors;
}

LightColors pointLight(in PointLight light, vec3 normal) {
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

LightColors spotlight(in Spotlight light, vec3 normal) {
  vec3 lightDirection = normalize(light.position - fsIn.position);
  float theta = dot(lightDirection, normalize(-light.direction));
  float epsilon = light.cutOff - light.outerCutOff;
  float intensity = step(light.outerCutOff, theta) * clamp((theta - light.outerCutOff) / epsilon, 0, 1);

  LightColors colors;
  colors.ambient = light.colors.ambient * intensity;
  colors.diffuse = light.colors.diffuse * intensity * diffuse(normal, lightDirection);
  colors.specular = light.colors.specular * intensity * specular(normal, lightDirection);
  return colors;
}

void main() {
  vec4 materialDiffuse = texture(uMaterial.diffuse, fsIn.texCoord);
  vec3 materialSpecular = texture(uMaterial.specular, fsIn.texCoord).rgb;
  vec3 materialEmission = texture(uMaterial.emission, fsIn.texCoord).rgb;

  vec3 normal = normalize(fsIn.normal);
  if (!gl_FrontFacing) {
    normal = -normal;
  }

  LightColors directionalLightColors = directionalLight(uDirectionalLight, normal);
  LightColors pointLightColors = pointLight(uPointLight, normal);
  LightColors spotlightColors = spotlight(uSpotlight, normal);

  vec3 combinedAmbient = directionalLightColors.ambient + pointLightColors.ambient + spotlightColors.ambient;
  vec3 combinedDiffuse = directionalLightColors.diffuse + pointLightColors.diffuse + spotlightColors.diffuse;
  vec3 combinedSpecular = directionalLightColors.specular + pointLightColors.specular + spotlightColors.specular;

  vec4 result = materialDiffuse * vec4(combinedAmbient, 1) +
                materialDiffuse * vec4(combinedDiffuse, 1) +
                vec4(materialSpecular * combinedSpecular, 0) +
                vec4(materialEmission * step(1, 1 - materialSpecular), 0);
  outColor = result;
}
