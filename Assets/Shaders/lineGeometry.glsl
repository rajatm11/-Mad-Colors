#version 400 core

layout(triangles) in;
layout(line_strip,max_vertices = 3)out;

in Vertex{
	vec4 colour;	
}IN[];

out Vertex{
	vec4 colour;	
}OUT;

void main(){

	for (int i=0;i<gl_in.length();++i){
		gl_Position = gl_in[i].gl_Position;
		OUT.colour =IN[i].colour;
		EmitVertex();
	}

	EndPrimitive();
}