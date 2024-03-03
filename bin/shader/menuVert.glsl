#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 vertexUV;

uniform mat4 transform;
uniform mat4 mvp;
uniform vec3 shapeColor;
uniform mat4 textureTransform;
uniform float scaleX;
uniform float scaleY;
uniform int type;

out vec2 UVcoord;
out vec4 fragColor;
out vec4 fragPos;
out vec2 scale;
flat out int typeM;

void main()
{
    UVcoord = vec2((textureTransform * vec4(vertexUV, 0, 1)).xy);

	fragPos = vec4(position, 1);
    fragPos.x *= scaleX;
    fragPos.y *= scaleY;

	scale = vec2(scaleX, scaleY);
	
	fragColor = vec4(shapeColor, 1);

	gl_Position = mvp * transform * vec4(position, 1);

	typeM = type;
}


