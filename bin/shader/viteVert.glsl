#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 vertexUV;

uniform mat4 transform;
uniform mat4 mvp;
uniform vec3 shapeColor;
uniform mat4 textureTransform;
uniform float scaleX;
uniform float scaleY;

uniform mat4 top;
uniform mat4 bottom;

out vec2 UVcoord;
out vec4 fragColor;
out vec4 fragPos;
out vec2 scale;

void main()
{
    UVcoord = vec2((textureTransform * vec4(vertexUV, 0, 1)).xy);

	fragPos = vec4(position, 1);
    fragPos.x *= scaleX;
    fragPos.y *= scaleY;

	scale = vec2(scaleX, scaleY);
	
	fragColor = vec4(shapeColor, 1);

	vec4 pos = vec4(position, 1);
	pos = ((top * pos) * (1 - position.y) + (bottom * pos) * (position.y));

	gl_Position = mvp * transform * pos;
}


