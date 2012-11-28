attribute vec4 position;
attribute vec3 normal;

uniform mat4 modelViewProjectionMatrix;
uniform mat3 normalMatrix;

varying vec4 colorVarying;

void main()
{
    vec3 eyeNormal = normalize(normalMatrix * normal);
    vec3 lightPosition = modelViewProjectionMatrix * vec4(1.0, 1.0, 1.0, 1.0);
    vec4 diffuseColor = vec4(0.4, 0.4, 1.0, 1.0);
    
    float nDotVP = max(0.0, dot(eyeNormal, normalize(lightPosition)));
                 
	colorVarying = diffuseColor * nDotVP + 0.5;
    
    gl_Position = modelViewProjectionMatrix * position;
}