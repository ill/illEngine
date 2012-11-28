/**
For now, kindof a ghetto port of my vertex uber shader with #defines specifically for the bill drawing
*/

//TODO: rename a bunch of stuff and use structs

//these #defines would normally be set by the shader/material system
#define POSITION_TRANSFORM
#define NORMAL_ATTRIBUTE
#define TEX_COORD_ATTRIBUTE
#define SKELETAL_ANIMATION

//this should pretty much always be here
#ifdef POSITION_TRANSFORM
uniform mat4 transformMat;
attribute vec3 positionIn;
#endif

#ifdef NORMAL_ATTRIBUTE
uniform mat3 normalMat;
attribute vec3 normalIn;
varying vec3 normalOut;
#endif

//if doing normal mapping, tangents will be needed
#ifdef TANGENT_ATTRIBUTE
attribute vec4 tangentIn;     // w coord is the handedness
varying vec3 tangentOut;
varying vec3 bitangentOut;
#endif

//if doing texture mapping, texture coordinates will be needed
#ifdef TEX_COORD_ATTRIBUTE
attribute vec2 texCoordIn;
varying vec2 texCoordOut;
#endif

//if doing skeletal animation, also send that stuff
#ifdef SKELETAL_ANIMATION

#ifndef MAX_BONES
/*
usually just allocate 64 bones

keep in mind, custom allocating different bone sizes will require 
compiling of multiple shaders instead of having 1 common shader, requiring more state changes!!!!

So avoid doing that and just use 64 bones in most cases.  Shader model 1.1 is limited to 24 though.
*/
//TODO: check if shader model 1.1?  and make the default be 24 instead of 64
#define MAX_BONES 64
#endif

uniform mat3x4 boneMatrix[MAX_BONES];

attribute uvec4 blendIndeces;
attribute vec4 blendWeights;

#endif

//TODO: vertex color and specular

void main(void) {

#ifdef SKELETAL_ANIMATION

    //position
    #ifdef POSITION_TRANSFORM
    vec3 blendPos(0.0);
    #endif
    
    //normals
    #ifdef NORMAL_ATTRIBUTE
    vec3 blendNorm(0.0);
    #endif
    
    //TODO: tangents

    for(int blendIndex = 0; blendIndex < 4; blendIndex++) {
        //position
        #ifdef POSITION_TRANSFORM
        blendPos += boneMatrix[blendIndeces[blendIndex]] * positionIn * blendWeights[blendIndex];
        #endif
        
        //normals
        #ifdef NORMAL_ATTRIBUTE
	    blendNorm += ((mat3) boneMatrix[blendIndeces[blendIndex]]) * normalIn * blendWeights[blendIndex];
	    #endif
	    
	    //TODO: tangents
    }
    
    //position
    #ifdef POSITION_TRANSFORM
    gl_Position = transformMat * vec4(blendPos, 1.0);
    #endif
    
    //normals    
    #ifdef NORMAL_ATTRIBUTE
    normalOut = normalMat * normalize(blendNorm);
    #endif
    
    //TODO: tangents

#else

    //position    
    #ifdef POSITION_TRANSFORM
    gl_Position = transformMat * vec4(positionIn, 1.0);
    #endif
    
    //normals    
    #ifdef NORMAL_ATTRIBUTE
    normalOut = normalMat * normalIn;
    #endif
    
    //tangents    
    #ifdef TANGENT_ATTRIBUTE
    tangentOut = normalMat * tangentIn.xyz;
    #endif

#endif

//bitangent
#ifdef TANGENT_ATTRIBUTE
    bitangentOut = cross(normalOut, -tangentIn.w * tangentOut);                 //TODO: figure out why my bitangent needs to actually face the other way
#endif
   
//texture coord   
#ifdef TEX_COORD_ATTRIBUTE
   texCoordOut = texCoordIn;
#endif

    //TODO: vertex color and specular
}