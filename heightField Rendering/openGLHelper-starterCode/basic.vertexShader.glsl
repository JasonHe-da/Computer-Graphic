#version 150

in vec3 position;
in vec4 left_p, right_p, top_p, under_p;
in vec4 color;
out vec4 col;

uniform int mode;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

void main()
{
  if(mode == 0){
    gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0f);
    col = color;
  }else if(mode == 1){
    float temp_soomth = (left_p.y + right_p.y + top_p.y + under_p.y) / 4.0f;
    vec4 outputColor = max(color, vec4(0.000000000000000001)) / max(position.y, 0.000000000000000001) * temp_soomth;
    outputColor.a = 1.0f;
    gl_Position = projectionMatrix * modelViewMatrix * vec4(position.x,temp_soomth, position.z, 1.0f);
    col = outputColor;
  }

}

