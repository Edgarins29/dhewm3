/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.

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

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "sys/platform.h"
#include "renderer/tr_local.h"

#include "renderer/Image.h"

/*
================
idImage::BindFBO
================
*/
void idImage::BindFBO( void ) {
	qglBindFramebuffer( GL_FRAMEBUFFER, fboHandle );
}

/*
================
idImage::UnBindFBO
================
*/
void idImage::UnBindFBO( void ) {
	qglBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

/*
================
idImage::GenerateFrameBufferImage
================
*/
void idImage::GenerateFrameBufferImage( int width, int height ) {
	PurgeImage();

	filter = TF_LINEAR;
	allowDownSize = false;
	repeat = TR_REPEAT;
	depth = TD_HIGH_QUALITY;

	uploadHeight = height;
	uploadWidth = width;
	type = TT_2D;

	if( qglGenFramebuffers == NULL )
		return;

	// generate the texture number
	qglGenTextures( 1, &texnum );
	qglBindTexture( GL_TEXTURE_2D, texnum );
	qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	qglTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL );

	qglGenFramebuffers( 1, &fboHandle );

	BindFBO();

	qglGenRenderbuffers( 1, &fboDepthBuffer );
	qglBindRenderbuffer( GL_RENDERBUFFER, fboDepthBuffer );
	qglRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_STENCIL, width, height );
	qglFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fboDepthBuffer );
	qglFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fboDepthBuffer );

	// Attach the image to the fbo.
	qglGenRenderbuffers( 1, &fboColorBuffer );
	qglBindRenderbuffer( GL_RENDERBUFFER, fboColorBuffer );
    qglRenderbufferStorage( GL_RENDERBUFFER, GL_RGBA, width, height );
    qglFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, fboColorBuffer );

	qglFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texnum, 0 );

	// Check the FBO status to ensure it got created properly.
	GLenum status = qglCheckFramebufferStatus( GL_FRAMEBUFFER );
	if( status != GL_FRAMEBUFFER_COMPLETE ) {
		common->FatalError( "Failed to generate FBO image for %s\n", imgName );
	}

	UnBindFBO();
	qglBindTexture( GL_TEXTURE_2D, 0 );
}

/*
================
idImage::GenerateFrameBufferDepthImage
================
*/
void idImage::GenerateFrameBufferDepthImage( int width, int height ) {
	PurgeImage();

	if( qglGenFramebuffers == NULL )
		return;

	filter = TF_LINEAR;
	allowDownSize = false;
	repeat = TR_REPEAT;
	depth = TD_HIGH_QUALITY;

	uploadHeight = height;
	uploadWidth = width;
	type = TT_2D;
	qglGenFramebuffers( 1, &fboHandle );
	
	// Try to use a texture depth component
	qglGenTextures( 1, &texnum );
	qglBindTexture( GL_TEXTURE_2D, texnum );
	qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	qglTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0 );
	qglBindTexture( GL_TEXTURE_2D, 0 );
	
	// create a framebuffer object
	qglGenFramebuffers( 1, &fboHandle );
	qglBindFramebuffer( GL_FRAMEBUFFER, fboHandle );
	
	// Instruct openGL that we won't bind a color texture with the currently binded FBO
	qglDrawBuffer( GL_NONE );
	qglReadBuffer( GL_NONE );
	
	// attach the texture to FBO depth attachment point
	qglFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texnum, 0 );
	
	// check FBO status
	GLenum status = qglCheckFramebufferStatus( GL_FRAMEBUFFER );
	if( status != GL_FRAMEBUFFER_COMPLETE ) {
		common->FatalError( "Failed to generate FBO image for %s\n", imgName);
	}

	UnBindFBO();
	qglBindTexture(GL_TEXTURE_2D, 0);

}

/*
================
idImage::GenerateFrameBufferColorTargetFromFBO
================
*/
void idImage::GenerateFrameBufferColorTargetFromFBO( void ) {
	idStr parentImgName = imgName;

	int magicChr = parentImgName.Find( '/' );
	parentImgName[magicChr] = 0;

	if( qglGenFramebuffers == NULL )
		return;

	idImage *parentImage = globalImages->GetImage( parentImgName.c_str() );
	if(parentImage == NULL) {
		common->FatalError( "GenerateFrameBufferColorTargetFromFBO: Can't find FBO %s to attach color target.\n",  parentImgName.c_str());
	}

	if(parentImage->fboHandle == -1) {
		common->FatalError( "GenerateFrameBufferColorTargetFromFBO: %s is not a fbo\n", parentImgName.c_str() );
	}

	PurgeImage();

	filter = TF_LINEAR;
	allowDownSize = false;
	repeat = TR_REPEAT;
	depth = TD_HIGH_QUALITY;

	uploadHeight = parentImage->uploadHeight;
	uploadWidth = parentImage->uploadWidth;
	type = TT_2D;

	// Bind the FBO
	parentImage->BindFBO();

	qglGenRenderbuffers(1, &fboColorBuffer );
	qglBindRenderbuffer( GL_RENDERBUFFER, fboColorBuffer );    

	// Create the texture.
	qglGenTextures( 1, &texnum );
    qglBindTexture( GL_TEXTURE_2D, texnum );
	if(strstr( imgName.c_str(), "xyz")) {
		qglTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F, uploadWidth, uploadHeight, 0, GL_RGBA, GL_FLOAT, NULL );
		qglRenderbufferStorage( GL_RENDERBUFFER, GL_RGBA32F, uploadWidth, uploadHeight);
	}
	else if(strstr( imgName.c_str(), "normal")) {
		qglTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16F, uploadWidth, uploadHeight, 0, GL_RGBA, GL_FLOAT, NULL );
		qglRenderbufferStorage( GL_RENDERBUFFER, GL_RGBA16F, uploadWidth, uploadHeight );
	}
	else {
		common->FatalError( "Illegal colortarget" );
	}
    qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	qglFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1 + parentImage->numAdditionalColorTargets, GL_RENDERBUFFER, fboColorBuffer );

	// Attach the color target to the fbo.
    qglFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1 + parentImage->numAdditionalColorTargets, GL_TEXTURE_2D, texnum, 0 );

	// check FBO status
	GLenum status = qglCheckFramebufferStatus( GL_FRAMEBUFFER );
	if( status != GL_FRAMEBUFFER_COMPLETE ) {
		common->FatalError( "Failed to attach FBO colortarget for %s\n", imgName);
	}

	// Unbind the FBO.
	parentImage->UnBindFBO();

	// Assign this image as a additional color target now that it's valid. 
	parentImage->fboColorTargets[parentImage->numAdditionalColorTargets++] = this;
}

/*
================
idImage::GenerateFrameBufferCubeImage
================
*/
void idImage::GenerateFrameBufferCubeImage( int width, int height ) {
	PurgeImage();

	if( qglGenFramebuffers == NULL )
		return;

	filter = TF_LINEAR;
	allowDownSize = false;
	repeat = TR_REPEAT;
	depth = TD_HIGH_QUALITY;

	uploadHeight = width;
	uploadWidth = height;
	type = TT_CUBIC;

	qglGenTextures( 1, &depthTexNum );
	qglBindTexture( GL_TEXTURE_CUBE_MAP, depthTexNum );
	qglTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	qglTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	qglTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	qglTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	qglTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
	for ( uint face = 0; face < 6; face++ ) {
		qglTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );
	}

	// color cube map
	qglGenTextures( 1, &texnum);
	qglBindTexture( GL_TEXTURE_CUBE_MAP, texnum);
	qglTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	qglTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	qglTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	qglTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	qglTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
	for (uint face = 0; face < 6; face++) {
		qglTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, NULL );
	}

	// framebuffer object
	qglGenFramebuffers( 1, &fboHandle );
	qglBindFramebuffer( GL_FRAMEBUFFER, fboHandle );
	qglFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP, depthTexNum, 0 );
	qglFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP, texnum, 0 );

	qglDrawBuffer( GL_COLOR_ATTACHMENT0 );

	// Check the FBO status to ensure it got created properly.
	GLenum status = qglCheckFramebufferStatus( GL_FRAMEBUFFER );
	if( status != GL_FRAMEBUFFER_COMPLETE ) {
		common->FatalError( "Failed to generate FBO image for %s\n", imgName);
	}

	UnBindFBO();
	qglBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

static void R_Image_FrameBufferCubeImage( idImage *image ) {
	image->GenerateFrameBufferImage( glConfig.vidWidth, glConfig.vidHeight );
}

/*
=======================
R_CreateFrameBufferCubeImage
=======================
*/
idImage *R_CreateFrameBufferCubeImage( const char *name ) {
	return NULL;
}
