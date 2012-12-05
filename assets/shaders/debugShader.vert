attribute vec4 position;
attribute vec3 normal;
attribute vec4 boneIndices;
attribute vec4 weights;
attribute vec2 texCoords;

uniform mat4 modelViewProjectionMatrix;
uniform mat3 normalMatrix;

varying vec4 colorVarying;
varying vec2 texCoordsOut;

#define MAX_BONES 64
uniform mat4 bones[MAX_BONES];

void main()
{
	mat4 transformedMat = bones[boneIndices[0]] * weights[0];
	transformedMat += bones[boneIndices[1]] * weights[1];
	transformedMat += bones[boneIndices[2]] * weights[2];
	transformedMat += bones[boneIndices[3]] * weights[3];
	
    vec3 eyeNormal = normalize(normalMatrix * normal);
    vec3 lightPosition = modelViewProjectionMatrix * vec4(1.0, 1.0, 1.0, 1.0);
    vec4 diffuseColor = vec4(0.4, 0.4, 1.0, 1.0);
    
    float nDotVP = max(0.0, dot(eyeNormal, normalize(lightPosition)));
                 
	colorVarying = diffuseColor * nDotVP + 0.5;
	
	texCoordsOut = texCoords;
	
    gl_Position = modelViewProjectionMatrix * transformedMat * position;
}