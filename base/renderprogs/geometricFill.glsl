/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.
Copyright (C) 2014 Pat 'raynorpat' Raynor <raynorpat@gmail.com>

This file is part of the Doom 3 GPL Source Code ("Doom 3 Source Code").

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms.
You should have received a copy of these additional terms immediately following the terms and
conditions of the GNU General Public License which accompanied the Doom 3 Source Code.
If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you
may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

varying vec4          	var_Vertex; 

varying vec2            var_TexDiffuse; 
varying vec2            var_TexNormal; 
varying vec2            var_TexSpecular; 
varying mat3     		var_TangentToWorldMatrix; 

VertexProgram

	attribute vec4      attr_TexCoord;
	attribute vec4      attr_Tangent;
	attribute vec4      attr_Binormal;
	attribute vec4      attr_Normal;

	uniform mat4		u_matview;
	
	void main( void ) {
		gl_Position = ftransform();

		// transform position into world space 
		var_Vertex = ( u_matview * gl_Vertex );

		// diffuse map texgen     
		var_TexDiffuse.xy = attr_TexCoord.st;

		// specular map texgen 
		var_TexSpecular.xy = attr_TexCoord.st;

		// normal map texgen 
		var_TexNormal.xy = attr_TexCoord.st;
	
		// construct tangent-space-to-world-space 3x3 matrix 
		var_TangentToWorldMatrix = mat3( attr_Tangent.x, attr_Binormal.x, attr_Normal.x,
										attr_Tangent.y, attr_Binormal.y, attr_Normal.y,
										attr_Tangent.z, attr_Binormal.z, attr_Normal.z );
	}
	
EndVertexProgram

FragmentProgram

	uniform sampler2D   	u_normalTexture;      
	uniform sampler2D   	u_diffuseTexture;   
	uniform sampler2D   	u_specularTexture;

	void main( void ) {
		vec4 diffuse = texture2D( u_diffuseTexture, var_TexDiffuse.st );
		vec3 specular = texture2D( u_specularTexture, var_TexSpecular.st ).rgb;
		
		// compute normal in tangent space from normalmap
		vec3 N = 2.0 * ( texture2D( u_normalTexture, var_TexNormal.st ).xyz - 0.5 );
		
		// transform normal into world space
		N = var_TangentToWorldMatrix * N;
		
		// convert normal back to [0,1] color space
		N = N * 0.5 + 0.5;
		
		// output to buffers
		gl_FragData[0] = diffuse;
		gl_FragData[1] = vec4( N, 0.0 );
		gl_FragData[2] = vec4( specular, 2.0 );
		gl_FragData[3] = vec4( var_Vertex.xyz, gl_FragCoord.z );
	}
	
EndFragmentProgram