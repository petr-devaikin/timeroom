#version 150

// special transparency based on the mask. see expansion

// these are our textures
uniform sampler2DRect tex0;
uniform sampler2DRect tex1;

// this comes from the vertex shader
in vec2 texCoordVarying;

// this is the output of the fragment shader
out vec4 outputColor;


void main()
{
    vec4 tex0Color = texture(tex0, texCoordVarying).rgba;
    vec4 tex1Color = texture(tex1, texCoordVarying).rgba;
    
    if (tex0Color.r < tex1Color.r)
        outputColor = tex0Color;
    else
        outputColor = tex1Color;
}
