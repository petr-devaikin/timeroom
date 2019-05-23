#version 150

// draws outline of white objects on black background

// these are our textures
uniform sampler2DRect tex0;
uniform sampler2DRect backgroundTex;

// this comes from the vertex shader
in vec2 texCoordVarying;

// this is the output of the fragment shader
out vec4 outputColor;


void main()
{
    vec4 tex0Color = texture(tex0, texCoordVarying).rgba;
    vec4 backgroundColor = texture(backgroundTex, texCoordVarying).rgba;
    
    if (tex0Color.r != 0)
        outputColor = vec4(0, 0, 0, 1);
    else {
        float result = 0;
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                if (i == 0 && j == 0) continue;
                
                float k = 1;
                if (i * j != 0) k = 0.707;
                
                vec4 neighbourColor = texture(tex0, texCoordVarying + (i, j)).rgba;
                if (neighbourColor.r == 1) {
                    result = max(result, k);
                }
            }
        }
        
        result = max(result, backgroundColor.r);
        
        outputColor = vec4(result, result, result, 1);
    }
}
