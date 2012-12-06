varying vec2 texCoordsOut;
varying vec3 lightVec;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;

void main()
{
	// lookup normal from normal map, move from [0,1] to  [-1, 1] range, normalize
	vec3 normal = 2.0 * texture2D(normalMap, texCoordsOut).rgb - 1.0;
	normal = normalize(normal);

	// compute diffuse lighting
	float lambertFactor = max (dot (lightVec, normal), 0.0);
		
    gl_FragColor = texture2D(diffuseMap, texCoordsOut) * lambertFactor + texture2D(diffuseMap, texCoordsOut) * 0.1;
}