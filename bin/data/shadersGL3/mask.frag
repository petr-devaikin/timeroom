#version 150

// special transparency based on the mask. see expansion

// these are our textures
uniform sampler2DRect tex0;
uniform sampler2DRect mask;
uniform sampler2DRect pastImage;

// this comes from the vertex shader
in vec2 texCoordVarying;

// this is the output of the fragment shader
out vec4 outputColor;


void main()
{
    vec4 maskColor = texture(mask, texCoordVarying).rgba;
    vec4 color = texture(tex0, texCoordVarying).rgba;
    vec4 pastColor = texture(pastImage, texCoordVarying).rgba;

    if (maskColor.r == 0)
        outputColor = color;
    else
        outputColor = pastColor;
}
