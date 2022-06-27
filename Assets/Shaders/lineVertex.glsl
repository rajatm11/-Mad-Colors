#version 400 core

uniform mat4 modelMatrix 	= mat4(1.0f);
uniform mat4 viewMatrix 	= mat4(1.0f);
uniform mat4 projMatrix 	= mat4(1.0f);
uniform vec4 objectColour = vec4(1,1,1,1);
uniform bool hasVertexColours = false;

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colour;

out Vertex
{
	vec4 colour;
} OUT;

void main(void)
{
	mat4 mvp 		  = (projMatrix * viewMatrix * modelMatrix);
	OUT.colour		= objectColour;
	if(hasVertexColours) {
		OUT.colour		= objectColour * colour;
	}
	gl_Position		= mvp * vec4(position, 1.0);
}