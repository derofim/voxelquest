#version 330

uniform float heightOfNearPlane;
uniform float FOV;
uniform vec2 clipDist;
uniform vec2 bufferDim;
uniform vec3 cameraPos;


uniform mat4 modelviewInverse;
uniform mat4 modelview;
uniform mat4 proj;



$

layout(location = 0) in vec4 vposition;
layout(location = 1) in vec4 vtexcoord;

out vec4 worldPos;

void main() {
	
	worldPos = vec4(vposition.xyz,1.0);
	vec4 screenPos = 
		proj*modelview*worldPos;
		//worldPos;
	
	gl_PointSize = (heightOfNearPlane / pow(screenPos.w,0.5))*0.2;
		
	gl_Position = screenPos;
	
}

$

in vec4 worldPos;

layout(location = 0) out vec4 FragColor0;

void main() {

	FragColor0 = vec4(
		//mod((worldPos.xyz+0.01)/32.0,1.0),
		(sin(worldPos.xyz/16.0)+1.0)*0.5*
		vec3(
			1.0-distance(worldPos.xyz,cameraPos)/70.0	
		),
		1.0
	);

}
