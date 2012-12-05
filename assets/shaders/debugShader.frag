varying vec4 colorVarying;
varying vec2 texCoordsOut;

uniform sampler2D diffuseMap;

void main()
{
    gl_FragColor = colorVarying * texture2D(diffuseMap, texCoordsOut);
}