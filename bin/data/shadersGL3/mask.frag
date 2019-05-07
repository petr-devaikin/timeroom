#version 150

// special transparency based on the mask. see expansion

// these are our textures
uniform sampler2DRect tex0;
uniform sampler2DRect background;
uniform sampler2DRect tex0Depth;
uniform sampler2DRect backgroundDepth;

// this comes from the vertex shader
in vec2 texCoordVarying;

// this is the output of the fragment shader
out vec4 outputColor;


void main()
{
    vec4 tex0Color = texture(tex0, texCoordVarying).rgba;
    vec4 tex0DepthValue = texture(tex0Depth, texCoordVarying).rgba;
    vec4 backgroundColor = texture(background, texCoordVarying).rgba;
    vec4 backgroundDepthValue = texture(backgroundDepth, texCoordVarying).rgba;

    if (backgroundDepthValue.r > tex0DepthValue.r)
        outputColor = tex0Color;
    else
        outputColor = backgroundColor;
}
