#version 150

// special transparency based on the mask. see expansion

// these are our textures
uniform sampler2DRect tex0;
uniform sampler2DRect tex0Depth;
uniform sampler2DRect tex1;
uniform sampler2DRect tex1Depth;

// this comes from the vertex shader
in vec2 texCoordVarying;

// this is the output of the fragment shader
out vec4 outputColor;


void main()
{
    vec4 tex0Color = texture(tex0, texCoordVarying).rgba;
    vec4 tex0DepthValue = texture(tex0Depth, texCoordVarying).rgba;
    
    vec4 tex1Color = texture(tex1, texCoordVarying).rgba;
    vec4 tex1DepthValue = texture(tex1Depth, texCoordVarying).rgba;
    
    if (tex0DepthValue.r < tex1DepthValue.r) {
        outputColor = tex0Color;
    }
    else {
        outputColor = tex1Color;
    }
}
