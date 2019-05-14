#version 150

// special transparency based on the mask. see expansion

// these are our textures
uniform sampler2DRect tex0;
uniform sampler2DRect tex0Depth;
uniform sampler2DRect background0;
uniform sampler2DRect background0Depth;
uniform sampler2DRect background1;
uniform sampler2DRect background1Depth;
uniform sampler2DRect background2;
uniform sampler2DRect background2Depth;

// this comes from the vertex shader
in vec2 texCoordVarying;

// this is the output of the fragment shader
out vec4 outputColor;


void main()
{
    vec4 tex0Color = texture(tex0, texCoordVarying).rgba;
    vec4 tex0DepthValue = texture(tex0Depth, texCoordVarying).rgba;
    
    vec4 background0Color = texture(background0, texCoordVarying).rgba;
    vec4 background0DepthValue = texture(background0Depth, texCoordVarying).rgba;
    vec4 background1Color = texture(background1, texCoordVarying).rgba;
    vec4 background1DepthValue = texture(background1Depth, texCoordVarying).rgba;
    vec4 background2Color = texture(background2, texCoordVarying).rgba;
    vec4 background2DepthValue = texture(background2Depth, texCoordVarying).rgba;

    outputColor = tex0Color;
    float outputDepth = tex0DepthValue.r;
    
    if (background0DepthValue.r < outputDepth) {
        outputColor = background0Color;
        outputDepth = background0DepthValue.r;
    }
    if (background1DepthValue.r < outputDepth) {
        outputColor = background1Color;
        outputDepth = background1DepthValue.r;
    }
    if (background2DepthValue.r < outputDepth) {
        outputColor = background2Color;
        outputDepth = background2DepthValue.r;
    }
}
