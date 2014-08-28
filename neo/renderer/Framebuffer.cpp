/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 2014 Robert Beckebans

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "tr_local.h"
#include "Framebuffer.h"

idList<Framebuffer*>	Framebuffer::framebuffers;

globalFramebuffers_t globalFramebuffers;

static void R_ListFramebuffers_f( const idCmdArgs& args )
{
}

Framebuffer::Framebuffer( const char* name, int w, int h )
{
	fboName = name;
	
	frameBuffer = 0;
	
	memset( frameBufferTexture, 0, sizeof( frameBufferTexture ) );

	memset( colorBuffers, 0, sizeof( colorBuffers ) );
	colorFormat = 0;
	
	depthBuffer = 0;
	depthFormat = 0;
	
	stencilBuffer = 0;
	stencilFormat = 0;
	
	width = w;
	height = h;
	
	qglGenFramebuffers( 1, &frameBuffer );
	
	framebuffers.Append( this );
}

void Framebuffer::Init()
{
	cmdSystem->AddCommand( "listFramebuffers", R_ListFramebuffers_f, CMD_FL_RENDERER, "lists framebuffers" );
	
	backEnd.glState.currentFramebuffer = NULL;

	//
	// deferredRender FBO for the lighting pass
	//
	globalFramebuffers.deferredRenderFBO = new Framebuffer( "_deferredRender", glConfig.vidWidth, glConfig.vidHeight );
	globalFramebuffers.deferredRenderFBO->Bind();
			
	qglGenTextures( 1, &globalFramebuffers.deferredRenderFBO->frameBufferTexture[0] );
	qglBindTexture( GL_TEXTURE_2D, globalFramebuffers.deferredRenderFBO->frameBufferTexture[0] );
	qglTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, glConfig.vidWidth, glConfig.vidHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
	qglTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	qglTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	qglTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	qglTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	qglFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, globalFramebuffers.deferredRenderFBO->frameBufferTexture[0], 0 );
	qglBindTexture(GL_TEXTURE_2D, 0);

	//globalFramebuffers.deferredRenderFBO->AddColorBuffer( GL_RGBA, 0 );
	//globalFramebuffers.deferredRenderFBO->AttachImage2D( GL_TEXTURE_2D, globalImages->deferredLightingFBOImage, 0 );

	globalFramebuffers.deferredRenderFBO->AddDepthBuffer( GL_DEPTH_COMPONENT24 );

	globalFramebuffers.deferredRenderFBO->Check();

	//
	// geometricRender FBO as G-Buffer for deferred shading
	//
	globalFramebuffers.geometricRenderFBO = new Framebuffer( "_geometricRender", glConfig.vidWidth, glConfig.vidHeight );
	globalFramebuffers.geometricRenderFBO->Bind();
	
	qglGenTextures( 1, &globalFramebuffers.geometricRenderFBO->frameBufferTexture[0] );
	qglBindTexture( GL_TEXTURE_2D, globalFramebuffers.geometricRenderFBO->frameBufferTexture[0] );
	qglTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16F, glConfig.vidWidth, glConfig.vidHeight, 0, GL_RGBA, GL_FLOAT, NULL );
	qglTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	qglTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	qglTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	qglTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	qglFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, globalFramebuffers.geometricRenderFBO->frameBufferTexture[0], 0 );
	qglBindTexture(GL_TEXTURE_2D, 0);

//	globalFramebuffers.geometricRenderFBO->AddColorBuffer( GL_RGB16F, 0 );
//	globalFramebuffers.geometricRenderFBO->AttachImage2D( GL_TEXTURE_2D, globalImages->deferredDiffuseFBOImage, 0 );

	qglGenTextures( 1, &globalFramebuffers.geometricRenderFBO->frameBufferTexture[1] );
	qglBindTexture( GL_TEXTURE_2D, globalFramebuffers.geometricRenderFBO->frameBufferTexture[1] );
	qglTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16F, glConfig.vidWidth, glConfig.vidHeight, 0, GL_RGBA, GL_FLOAT, NULL );
	qglTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	qglTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	qglTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	qglTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	qglFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, globalFramebuffers.geometricRenderFBO->frameBufferTexture[1], 0 );
	qglBindTexture(GL_TEXTURE_2D, 0);

//	globalFramebuffers.geometricRenderFBO->AddColorBuffer( GL_RGB16F, 1 );
//	globalFramebuffers.geometricRenderFBO->AttachImage2D( GL_TEXTURE_2D, globalImages->deferredNormalFBOImage, 1 );

	qglGenTextures( 1, &globalFramebuffers.geometricRenderFBO->frameBufferTexture[2] );
	qglBindTexture( GL_TEXTURE_2D, globalFramebuffers.geometricRenderFBO->frameBufferTexture[2] );
	qglTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16F, glConfig.vidWidth, glConfig.vidHeight, 0, GL_RGBA, GL_FLOAT, NULL );
	qglTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	qglTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	qglTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	qglTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	qglFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, globalFramebuffers.geometricRenderFBO->frameBufferTexture[2], 0 );
	qglBindTexture(GL_TEXTURE_2D, 0);
			
//	globalFramebuffers.geometricRenderFBO->AddColorBuffer( GL_RGB16F, 2 );
//	globalFramebuffers.geometricRenderFBO->AttachImage2D( GL_TEXTURE_2D, globalImages->deferredSpecularFBOImage, 2 );

	qglGenTextures( 1, &globalFramebuffers.geometricRenderFBO->frameBufferTexture[3] );
	qglBindTexture( GL_TEXTURE_2D, globalFramebuffers.geometricRenderFBO->frameBufferTexture[3] );
	qglTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16F, glConfig.vidWidth, glConfig.vidHeight, 0, GL_RGBA, GL_FLOAT, NULL );
	qglTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	qglTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	qglTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	qglTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	qglFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, globalFramebuffers.geometricRenderFBO->frameBufferTexture[3], 0 );
	qglBindTexture(GL_TEXTURE_2D, 0);

//	globalFramebuffers.geometricRenderFBO->AddColorBuffer( GL_RGB16F, 3 );
//	globalFramebuffers.geometricRenderFBO->AttachImage2D( GL_TEXTURE_2D, globalImages->deferredPositionFBOImage, 3 );

	globalFramebuffers.geometricRenderFBO->AddDepthBuffer( GL_DEPTH_COMPONENT32F );

	// enable all attachments as draw buffers
	GLenum drawbuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	qglDrawBuffers( 4, drawbuffers );
	glReadBuffer( GL_NONE );

	globalFramebuffers.geometricRenderFBO->Check();

	//
	// default to null framebuffer
	//
	BindNull();
}

void Framebuffer::Shutdown()
{
	// TODO
}

void Framebuffer::Bind()
{
#if 0
	if( r_logFile.GetBool() )
	{
		RB_LogComment( "--- Framebuffer::Bind( name = '%s' ) ---\n", fboName.c_str() );
	}
#endif
	
//	if( backEnd.glState.currentFramebuffer != this )
	{
		qglBindFramebuffer( GL_FRAMEBUFFER, frameBuffer );
		backEnd.glState.currentFramebuffer = this;
	}
}

void Framebuffer::BindNull()
{
	//if(backEnd.glState.framebuffer != NULL)
	{
		qglBindFramebuffer( GL_FRAMEBUFFER, 0 );
		qglBindRenderbuffer( GL_RENDERBUFFER, 0 );
		backEnd.glState.currentFramebuffer = NULL;
	}
}

void Framebuffer::AddColorBuffer( int format, int index )
{
	if( index < 0 || index >= glConfig.maxColorAttachments )
	{
		common->Warning( "Framebuffer::AddColorBuffer( %s ): bad index = %i", fboName.c_str(), index );
		return;
	}
	
	colorFormat = format;
	
	bool notCreatedYet = colorBuffers[index] == 0;
	if( notCreatedYet )
	{
		qglGenRenderbuffers( 1, &colorBuffers[index] );
	}
	
	qglBindRenderbuffer( GL_RENDERBUFFER, colorBuffers[index] );
	qglRenderbufferStorage( GL_RENDERBUFFER, format, width, height );
	
	if( notCreatedYet )
	{
		qglFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_RENDERBUFFER, colorBuffers[index] );
	}
}

void Framebuffer::AddDepthBuffer( int format )
{
	depthFormat = format;
	
	bool notCreatedYet = depthBuffer == 0;
	if( notCreatedYet )
	{
		qglGenRenderbuffers( 1, &depthBuffer );
	}
	
	qglBindRenderbuffer( GL_RENDERBUFFER, depthBuffer );
	qglRenderbufferStorage( GL_RENDERBUFFER, format, width, height );
	
	if( notCreatedYet )
	{
		qglFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer );
	}
}

void Framebuffer::AttachImage2D( int target, const idImage* image, int index )
{
	if( ( target != GL_TEXTURE_2D ) && ( target < GL_TEXTURE_CUBE_MAP_POSITIVE_X || target > GL_TEXTURE_CUBE_MAP_NEGATIVE_Z ) )
	{
		common->Warning( "Framebuffer::AttachImage2D( %s ): invalid target", fboName.c_str() );
		return;
	}
	
	if( index < 0 || index >= glConfig.maxColorAttachments )
	{
		common->Warning( "Framebuffer::AttachImage2D( %s ): bad index = %i", fboName.c_str(), index );
		return;
	}
	
	qglFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, target, image->texnum, 0 );
}

void Framebuffer::AttachImageDepth( const idImage* image )
{
	qglFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, image->texnum, 0 );
}

void Framebuffer::AttachImageDepthLayer( const idImage* image, int layer )
{
	//qglFramebufferTextureLayer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, image->texnum, 0, layer );
}

void Framebuffer::Check()
{
	int prev;
	glGetIntegerv( GL_FRAMEBUFFER_BINDING, &prev );
	
	qglBindFramebuffer( GL_FRAMEBUFFER, frameBuffer );
	
	int status = qglCheckFramebufferStatus( GL_FRAMEBUFFER );
	if( status == GL_FRAMEBUFFER_COMPLETE )
	{
		qglBindFramebuffer( GL_FRAMEBUFFER, prev );
		return;
	}
	
	// something went wrong
	switch( status )
	{
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			common->Error( "Framebuffer::Check( %s ): Framebuffer incomplete, incomplete attachment", fboName.c_str() );
			break;
			
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			common->Error( "Framebuffer::Check( %s ): Framebuffer incomplete, missing attachment", fboName.c_str() );
			break;
			
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			common->Error( "Framebuffer::Check( %s ): Framebuffer incomplete, missing draw buffer", fboName.c_str() );
			break;
			
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			common->Error( "Framebuffer::Check( %s ): Framebuffer incomplete, missing read buffer", fboName.c_str() );
			break;
			
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
			common->Error( "Framebuffer::Check( %s ): Framebuffer incomplete, missing layer targets", fboName.c_str() );
			break;
			
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			common->Error( "Framebuffer::Check( %s ): Framebuffer incomplete, missing multisample", fboName.c_str() );
			break;
			
		case GL_FRAMEBUFFER_UNSUPPORTED:
			common->Error( "Framebuffer::Check( %s ): Unsupported framebuffer format", fboName.c_str() );
			break;
			
		default:
			common->Error( "Framebuffer::Check( %s ): Unknown error 0x%X", fboName.c_str(), status );
			break;
	};
	
	qglBindFramebuffer( GL_FRAMEBUFFER, prev );
}
