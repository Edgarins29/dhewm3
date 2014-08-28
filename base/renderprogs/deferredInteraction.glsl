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

VertexProgram

	void main( void ) {
		gl_Position = ftransform();
	}

EndVertexProgram

FragmentProgram

	uniform sampler2D   	u_normalTexture;
	uniform sampler2D   	u_diffuseTexture;
	uniform sampler2D   	u_specularTexture;
	uniform sampler2D   	u_vertTexture;
//	uniform sampler2D   	u_lightFalloffTexture;
//	uniform sampler2D   	u_lightProjectionTexture;
	
	uniform vec3	 		u_lightOrigin;  
	uniform vec3 			u_lightColor;	
	uniform vec3 			u_viewOrigin; 

	void main( void ) {
		// calculate the screen texcoord in the 0.0 to 1.0 range
		ivec2 tc = ivec2( int( gl_FragCoord.x ), int( gl_FragCoord.y ) );		
	
		// compute normal in world space
		vec3 N = 2.0 * ( texelFetch( u_normalTexture, tc, 0 ).xyz - 0.5 );
		
		// compute vertex position in world space
		vec4 P = texelFetch( u_vertTexture, tc, 0 ).xyzw;

		// compute light direction in world space
		vec3 L = normalize( u_lightOrigin - P.xyz );

		// compute view direction in world space
		vec3 V = normalize( u_viewOrigin - P.xyz );

		// compute half angle in world space
		vec3 H = normalize( L + V );
		
		// compute the diffuse term
		vec4 diffuse = texelFetch( u_diffuseTexture, tc, 0 );
		diffuse.rgb *= u_lightColor * clamp( dot( N, L ), 0.0, 1.0 );

		// compute the specular term
		vec4 S = texelFetch( u_specularTexture, tc, 0 );
		vec3 specular = S.rgb * u_lightColor * pow( clamp( dot( N, H ), 0.0, 1.0 ), S.a );
		
		// TODO: compute attenuation
		//vec4 lightProjection = texture2DProj( u_lightProjectionTexture, var_TexLightProj.xyw );  
		//vec4 lightFalloff = texture2D( u_lightFalloffTexture, var_TexLightFalloff );
		
		// compute final color
		vec4 color = diffuse;
		color.rgb += specular;
		//color *= lightProjection;
		//color *= lightFalloff;
		
		gl_FragColor = color;
		//gl_FragDepth = P.w;
	}

EndFragmentProgram
