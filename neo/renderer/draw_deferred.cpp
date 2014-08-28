// draw_deferred.cpp
//

#include "sys/platform.h"
#include "renderer/VertexCache.h"

#include "renderer/tr_local.h"
#include "renderer/Framebuffer.h"

void GL_SelectTextureNoClient( int unit );

idCVar r_showDeferredDiffuse( "r_showDeferredDiffuse", "0", CVAR_RENDERER | CVAR_INTEGER, "show diffuse gbuffer texture", 0, 1, idCmdSystem::ArgCompletion_Integer<0,1>  );
idCVar r_showDeferredNormal( "r_showDeferredNormal", "0", CVAR_RENDERER | CVAR_INTEGER, "show normal gbuffer texture", 0, 1, idCmdSystem::ArgCompletion_Integer<0,1>  );
idCVar r_showDeferredSpecular( "r_showDeferredSpecular", "0", CVAR_RENDERER | CVAR_INTEGER, "show specular gbuffer texture", 0, 1, idCmdSystem::ArgCompletion_Integer<0,1>  );
idCVar r_showDeferredPosition( "r_showDeferredPosition", "0", CVAR_RENDERER | CVAR_INTEGER, "show position gbuffer texture", 0, 1, idCmdSystem::ArgCompletion_Integer<0,1>  );

/*
=========================
RB_Deferred_DrawDeferredInteraction
=========================
*/
void RB_Deferred_DrawDeferredInteraction( const viewLight_t *vLight, idVec4 vLightColor ) {
	// set deferred render fbo and clear it
	qglBindFramebuffer( GL_FRAMEBUFFER, globalFramebuffers.deferredRenderFBO->GetFramebuffer() );
	qglViewport( 0,0, glConfig.vidWidth, glConfig.vidHeight );
	qglClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	qglClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	// set 2D virtual screen size
	qglMatrixMode( GL_MODELVIEW );
	qglPushMatrix();
	qglLoadIdentity();
	qglMatrixMode( GL_PROJECTION );
	qglPushMatrix();
	qglLoadIdentity();
	qglOrtho( 0, backEnd.viewDef->viewport.x2 - backEnd.viewDef->viewport.x1 + 1, 0, backEnd.viewDef->viewport.y2 - backEnd.viewDef->viewport.y1 + 1, -99999, 99999 );

	// bind the interaction program and set variables
	progs[PROG_DEFERRED_INTERACTION].programHandle->Bind();

	progs[PROG_DEFERRED_INTERACTION].programHandle->SetVar4fv( PP_LIGHT_PROJECT_S, 1, vLight->lightProject[0].ToFloatPtr());
	progs[PROG_DEFERRED_INTERACTION].programHandle->SetVar4fv( PP_LIGHT_PROJECT_T, 1, vLight->lightProject[1].ToFloatPtr() );
	progs[PROG_DEFERRED_INTERACTION].programHandle->SetVar4fv( PP_LIGHT_PROJECT_Q, 1, vLight->lightProject[2].ToFloatPtr() );
	progs[PROG_DEFERRED_INTERACTION].programHandle->SetVar4fv( PP_LIGHT_FALLOFF_S, 1, vLight->lightProject[3].ToFloatPtr() );
	progs[PROG_DEFERRED_INTERACTION].programHandle->SetVar3fv( PP_LIGHT_COLOR, 1, vLightColor.ToFloatPtr() );
	progs[PROG_DEFERRED_INTERACTION].programHandle->SetVar3fv( PP_VIEW_ORIGIN, 1, backEnd.viewDef->renderView.vieworg.ToFloatPtr() );
	progs[PROG_DEFERRED_INTERACTION].programHandle->SetVar3fv( PP_LIGHT_ORIGIN, 1, vLight->globalLightOrigin.ToFloatPtr() );

	static const float zero[4] = { 0, 0, 0, 0 };
	static const float one[4] = { 1, 1, 1, 1 };
	static const float negOne[4] = { -1, -1, -1, -1 };

	progs[PROG_DEFERRED_INTERACTION].programHandle->SetVar4fv( PP_COLOR_MODULATE, zero );
	progs[PROG_DEFERRED_INTERACTION].programHandle->SetVar4fv( PP_COLOR_ADD, one );

	// texture 1 is the normal map
	GL_SelectTextureNoClient( 0 );
	qglBindTexture( GL_TEXTURE_2D, globalFramebuffers.geometricRenderFBO->frameBufferTexture[1] );
	progs[PROG_DEFERRED_INTERACTION].programHandle->BindTextureVar( PP_TEX_NORMAL );

	// texture 2 is the diffuse map
	GL_SelectTextureNoClient( 1 );
	qglBindTexture( GL_TEXTURE_2D, globalFramebuffers.geometricRenderFBO->frameBufferTexture[0] );
	progs[PROG_DEFERRED_INTERACTION].programHandle->BindTextureVar( PP_TEX_DIFFUSE );

	// texture 3 is the specular map
	GL_SelectTextureNoClient( 2 );
	qglBindTexture( GL_TEXTURE_2D, globalFramebuffers.geometricRenderFBO->frameBufferTexture[2] );
	progs[PROG_DEFERRED_INTERACTION].programHandle->BindTextureVar( PP_TEX_SPEC );

	// texture 4 is the position map
	GL_SelectTextureNoClient( 3 );
	qglBindTexture( GL_TEXTURE_2D, globalFramebuffers.geometricRenderFBO->frameBufferTexture[3] );
	progs[PROG_DEFERRED_INTERACTION].programHandle->BindTextureVar( VV_TEX_VERTEX );
	
	// set OpenGL state for additive lighting
	GL_State( GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE | GLS_DEPTHTEST_DISABLE );
	GL_Cull( CT_TWO_SIDED );

	// do the light quad drawing
	int	 w = backEnd.viewDef->viewport.x2 - backEnd.viewDef->viewport.x1 + 1;
	int	 h = backEnd.viewDef->viewport.y2 - backEnd.viewDef->viewport.y1 + 1;

	qglBegin(GL_QUADS);
	qglVertex2f(0, 0);
	qglVertex2f(w, 0);
	qglVertex2f(w, h);
	qglVertex2f(0, h);
	qglEnd();

	// unbind and cleanup
	progs[PROG_DEFERRED_INTERACTION].programHandle->UnBind();

	GL_SelectTextureNoClient( 4 );
	globalImages->BindNull();

	GL_SelectTextureNoClient( 3 );
	globalImages->BindNull();

	GL_SelectTextureNoClient( 2 );
	globalImages->BindNull();

	GL_SelectTextureNoClient( 1 );
	globalImages->BindNull();

	backEnd.glState.currenttmu = -1;
	GL_SelectTexture( 0 );

	qglMatrixMode(GL_PROJECTION);
	qglPopMatrix();
	qglMatrixMode( GL_MODELVIEW );
	qglPopMatrix();

	// disable framebuffer
	qglBindFramebuffer( GL_FRAMEBUFFER, 0 );
}


/*
==================
RB_DrawDeferredInteractions
==================
*/
void RB_DrawDeferredInteractions( void ) {
	viewLight_t		*vLight;

	// set up render scene and render the light quad for each light interaction
	for ( vLight = backEnd.viewDef->viewLights ; vLight ; vLight = vLight->next ) {
		const shaderStage_t	*lightStage = vLight->lightShader->GetStage( 0 );
		const float			*lightRegs = vLight->shaderRegisters;

		// ignore stages that fail the condition
		if ( !lightRegs[ lightStage->conditionRegister ] ) {
			continue;
		}

		// grab light color
		idVec4 vLightColor;
		for( int i = 0; i < 4; i++ ) {
			vLightColor[i] = lightRegs[ lightStage->color.registers[i] ];
			if( i != 3 ) {
				vLightColor[i] *= backEnd.lightScale;
			}
		}
		
		// draw lighting
		RB_Deferred_DrawDeferredInteraction( vLight, vLightColor );
	}

	// reset scissor
	qglScissor( backEnd.viewDef->viewport.x1 + backEnd.currentScissor.x1,
					backEnd.viewDef->viewport.y1 + backEnd.currentScissor.y1,
					backEnd.currentScissor.x2 + 1 - backEnd.currentScissor.x1,
					backEnd.currentScissor.y2 + 1 - backEnd.currentScissor.y1 );

	GL_CheckErrors();
}


/*
==================
RB_Deferred_FillGBufferSurf
==================
*/
static void RB_Deferred_FillGBufferSurf( const drawSurf_t *surf ) {
	const idMaterial *shader;
	idImage *diffuse = NULL, *bump = NULL, *spec = NULL;

	shader = surf->material;

	// If the shader doesn't recieve lighting, we will use forward rendering instead.
	if(!shader->ReceivesLighting()) {
		return;
	}

	// translucent surfaces don't put anything in the depth buffer and don't
	// test against it, which makes them fail the mirror clip plane operation
	if ( shader->Coverage() == MC_TRANSLUCENT ) {
		return;
	}

	// set polygon offset if necessary
	if ( shader->TestMaterialFlag(MF_POLYGONOFFSET) ) {
		qglEnable( GL_POLYGON_OFFSET_FILL );
		qglPolygonOffset( r_offsetFactor.GetFloat(), r_offsetUnits.GetFloat() * shader->GetPolygonOffset() );
	}

	// set the vertex pointers
	idDrawVert	*ac = (idDrawVert *)vertexCache.Position( surf->geo->ambientCache );
	qglColorPointer( 4, GL_UNSIGNED_BYTE, sizeof( idDrawVert ), ac->color );
	qglVertexAttribPointerARB( 11, 3, GL_FLOAT, false, sizeof( idDrawVert ), ac->normal.ToFloatPtr() );
	qglVertexAttribPointerARB( 10, 3, GL_FLOAT, false, sizeof( idDrawVert ), ac->tangents[1].ToFloatPtr() );
	qglVertexAttribPointerARB( 9, 3, GL_FLOAT, false, sizeof( idDrawVert ), ac->tangents[0].ToFloatPtr() );
	qglVertexAttribPointerARB( 8, 2, GL_FLOAT, false, sizeof( idDrawVert ), ac->st.ToFloatPtr() );
	qglVertexPointer( 3, GL_FLOAT, sizeof( idDrawVert ), ac->xyz.ToFloatPtr() );

	for ( int surfaceStageNum = 0 ; surfaceStageNum < shader->GetNumStages() ; surfaceStageNum++ ) {
			const shaderStage_t	*surfaceStage = shader->GetStage( surfaceStageNum );

			switch( surfaceStage->lighting ) {
				case SL_BUMP: {
					// ignore stage that fails the condition
					if ( !surf->shaderRegisters[ surfaceStage->conditionRegister ] ) {
						break;
					}
					// draw any previous interaction
					bump = surfaceStage->texture.image;
					break;
				}
				case SL_DIFFUSE: {
					// ignore stage that fails the condition
					if ( !surf->shaderRegisters[ surfaceStage->conditionRegister ] ) {
						break;
					}
					diffuse = surfaceStage->texture.image;
					break;
				}
				case SL_SPECULAR: {
					// ignore stage that fails the condition
					if ( !surf->shaderRegisters[ surfaceStage->conditionRegister ] ) {
						break;
					}
					spec = surfaceStage->texture.image;
				break;
			}
		}
	}

	progs[PROG_GEOMETRIC_FILL].programHandle->SetVarMatrix4fv( VV_TEX_MATVIEW, 1, &surf->space->modelMatrix[0] );
		
	// texture 1 will be the per-surface bump map
	GL_SelectTextureNoClient( 0 );
	bump->Bind();
	progs[PROG_GEOMETRIC_FILL].programHandle->BindTextureVar( PP_TEX_NORMAL );

	// texture 2 is the per-surface diffuse map
	GL_SelectTextureNoClient( 1 );
	diffuse->Bind();
	progs[PROG_GEOMETRIC_FILL].programHandle->BindTextureVar( PP_TEX_DIFFUSE );

	// texture 3 is the per-surface specular map
	GL_SelectTextureNoClient( 2 );
	if(spec == NULL)
	{
		globalImages->whiteImage->Bind();
	}
	else
	{
		spec->Bind();
	}
	progs[PROG_GEOMETRIC_FILL].programHandle->BindTextureVar( PP_TEX_SPEC );

	// draw it
	RB_DrawElementsWithCounters( surf->geo );

	// reset polygon offset
	if ( shader->TestMaterialFlag(MF_POLYGONOFFSET) ) {
		qglDisable( GL_POLYGON_OFFSET_FILL );
	}

	// reset blending
	if ( shader->GetSort() == SS_SUBVIEW ) {
		GL_State( GLS_DEPTHFUNC_LESS );
	}	
}

/*
=========================
RB_Deferred_GBufferFill
=========================
*/
void RB_Deferred_GBufferFill( drawSurf_t **drawSurfs, int numDrawSurfs ) {
	// if we are just doing 2D rendering, no need to fill the depth buffer
	if ( !backEnd.viewDef->viewEntitys ) {
		return;
	}

	// bind the gbuffer and clear it
	qglBindFramebuffer( GL_FRAMEBUFFER, globalFramebuffers.geometricRenderFBO->GetFramebuffer() );
	qglViewport( 0,0, glConfig.vidWidth, glConfig.vidHeight );
	qglClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	qglClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// set drawbuffers
	GLenum drawbuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	qglDrawBuffers( 4, drawbuffers );

	// enable the vertex arrays
	qglEnableVertexAttribArrayARB( 8 );
	qglEnableVertexAttribArrayARB( 9 );
	qglEnableVertexAttribArrayARB( 10 );
	qglEnableVertexAttribArrayARB( 11 );
	
	// bind the pre-interaction program.
	progs[PROG_GEOMETRIC_FILL].programHandle->Bind();

	// draw into the gbuffer
	GL_State( GLS_DEPTHFUNC_LESS );
	RB_RenderDrawSurfListWithFunction( drawSurfs, numDrawSurfs, RB_Deferred_FillGBufferSurf );

	// unbind the pre-interaction program.
	progs[PROG_GEOMETRIC_FILL].programHandle->UnBind();

	GL_SelectTextureNoClient( 2 );
	globalImages->BindNull();

	GL_SelectTextureNoClient( 1 );
	globalImages->BindNull();

	backEnd.glState.currenttmu = -1;
	GL_SelectTexture( 0 );

	qglDisableVertexAttribArrayARB( 8 );
	qglDisableVertexAttribArrayARB( 9 );
	qglDisableVertexAttribArrayARB( 10 );
	qglDisableVertexAttribArrayARB( 11 );

	// disable offscreen gbuffer rendering
	qglBindFramebuffer( GL_FRAMEBUFFER, 0 );
}


/*
=========================
RB_Deferred_DrawToFrameBuffer
=========================
*/
void RB_Deferred_DrawToFrameBuffer( void ) {
	// disable framebuffer
	Framebuffer::BindNull();

	// enable shader
	progs[PROG_POSTPROCESS].programHandle->Bind();
//	GL_State( GLS_DEPTHTEST_DISABLE );
	GL_Cull( CT_TWO_SIDED );
	
	// bind current screen texture
	GL_SelectTextureNoClient( 0 );

	if( r_showDeferredDiffuse.GetInteger() ) {
		qglBindTexture( GL_TEXTURE_2D, globalFramebuffers.geometricRenderFBO->frameBufferTexture[0] );
	} else if( r_showDeferredNormal.GetInteger() ) {
		qglBindTexture( GL_TEXTURE_2D, globalFramebuffers.geometricRenderFBO->frameBufferTexture[1] );
	} else if( r_showDeferredSpecular.GetInteger() ) {
		qglBindTexture( GL_TEXTURE_2D, globalFramebuffers.geometricRenderFBO->frameBufferTexture[2] );
	} else if( r_showDeferredPosition.GetInteger() ) {
		qglBindTexture( GL_TEXTURE_2D, globalFramebuffers.geometricRenderFBO->frameBufferTexture[3] );
	} else {
		qglBindTexture( GL_TEXTURE_2D, globalFramebuffers.deferredRenderFBO->frameBufferTexture[0] );
	}

	progs[PROG_POSTPROCESS].programHandle->BindTextureVar( PP_TEX_CURRENT );

	// set 2D virtual screen size
	qglPushMatrix();
	qglLoadIdentity();
	qglMatrixMode(GL_PROJECTION);
	qglPushMatrix();
	qglLoadIdentity();
	qglOrtho(0, backEnd.viewDef->viewport.x2 - backEnd.viewDef->viewport.x1 + 1, 0, backEnd.viewDef->viewport.y2 - backEnd.viewDef->viewport.y1 + 1, -99999, 99999);

	qglBegin(GL_QUADS);
	qglVertex2f(0, 0);
	qglVertex2f(backEnd.viewDef->viewport.x2 - backEnd.viewDef->viewport.x1 + 1, 0);
	qglVertex2f(backEnd.viewDef->viewport.x2 - backEnd.viewDef->viewport.x1 + 1,
				backEnd.viewDef->viewport.y2 - backEnd.viewDef->viewport.y1 + 1);
	qglVertex2f(0, backEnd.viewDef->viewport.y2 - backEnd.viewDef->viewport.y1 + 1);
	qglEnd();

	qglPopMatrix();
	qglMatrixMode(GL_MODELVIEW);
	qglPopMatrix();

	// unbind the program
	progs[PROG_POSTPROCESS].programHandle->UnBind();

	backEnd.glState.currenttmu = -1;
	GL_SelectTexture( 0 );
	globalImages->BindNull();
}
