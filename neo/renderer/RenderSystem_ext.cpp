// RenderSystem_ext.cpp
#include "sys/platform.h"
#include "idlib/LangDict.h"

#include "tr_local.h"

// Vista OpenGL wrapper check
#ifdef _WIN32
#include "../sys/win32/win_local.h"
#endif

void ( APIENTRY * qglMultiTexCoord2fARB )( GLenum texture, GLfloat s, GLfloat t );
void ( APIENTRY * qglMultiTexCoord2fvARB )( GLenum texture, GLfloat *st );
void ( APIENTRY * qglActiveTextureARB )( GLenum texture );
void ( APIENTRY * qglClientActiveTextureARB )( GLenum texture );

void (APIENTRY *qglTexImage3D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *);

void (APIENTRY * qglColorTableEXT)( int, int, int, int, int, const void * );

// EXT_stencil_two_side
PFNGLACTIVESTENCILFACEEXTPROC			qglActiveStencilFaceEXT;

// ARB_texture_compression
PFNGLCOMPRESSEDTEXIMAGE2DARBPROC		qglCompressedTexImage2DARB;
PFNGLGETCOMPRESSEDTEXIMAGEARBPROC		qglGetCompressedTexImageARB;

// ARB_vertex_buffer_object
PFNGLBINDBUFFERARBPROC					qglBindBufferARB;
PFNGLDELETEBUFFERSARBPROC				qglDeleteBuffersARB;
PFNGLGENBUFFERSARBPROC					qglGenBuffersARB;
PFNGLISBUFFERARBPROC					qglIsBufferARB;
PFNGLBUFFERDATAARBPROC					qglBufferDataARB;
PFNGLBUFFERSUBDATAARBPROC				qglBufferSubDataARB;
PFNGLGETBUFFERSUBDATAARBPROC			qglGetBufferSubDataARB;
PFNGLMAPBUFFERARBPROC					qglMapBufferARB;
PFNGLUNMAPBUFFERARBPROC					qglUnmapBufferARB;
PFNGLGETBUFFERPARAMETERIVARBPROC		qglGetBufferParameterivARB;
PFNGLGETBUFFERPOINTERVARBPROC			qglGetBufferPointervARB;

// ARB_vertex_program / ARB_fragment_program
PFNGLBINDATTRIBLOCATIONARBPROC			qglBindAttribLocationARB;
PFNGLENABLEVERTEXATTRIBARRAYARBPROC		qglEnableVertexAttribArrayARB;
PFNGLDISABLEVERTEXATTRIBARRAYARBPROC	qglDisableVertexAttribArrayARB;
PFNGLVERTEXATTRIBPOINTERARBPROC			qglVertexAttribPointerARB;

// GL_EXT_depth_bounds_test
PFNGLDEPTHBOUNDSEXTPROC                 qglDepthBoundsEXT;

PFNGLCREATESHADEROBJECTARBPROC		qglCreateShaderObjectARB;
PFNGLDELETEOBJECTARBPROC				qglDeleteObjectARB;
PFNGLSHADERSOURCEARBPROC				qglShaderSourceARB;
PFNGLCOMPILESHADERARBPROC			qglCompileShaderARB;
PFNGLGETOBJECTPARAMETERIVARBPROC		qglGetObjectParameterivARB;
PFNGLCREATEPROGRAMOBJECTARBPROC		qglCreateProgramObjectARB;
PFNGLATTACHOBJECTARBPROC				qglAttachObjectARB;
PFNGLDETACHOBJECTARBPROC				qglDetachObjectARB;
PFNGLLINKPROGRAMARBPROC				qglLinkProgramARB;
PFNGLUSEPROGRAMOBJECTARBPROC			qglUseProgramObjectARB;
PFNGLGETUNIFORMLOCATIONARBPROC		qglGetUniformLocationARB;
PFNGLGETATTRIBLOCATIONARBPROC		qglGetAttributeLocationARB;
PFNGLUNIFORM1FARBPROC				qglUniform1fARB;
PFNGLUNIFORM1IARBPROC				qglUniform1iARB;
PFNGLUNIFORM1FVARBPROC				qglUniform1fvARB;
PFNGLUNIFORM2FVARBPROC				qglUniform2fvARB;
PFNGLUNIFORM3FVARBPROC				qglUniform3fvARB;
PFNGLUNIFORM4FVARBPROC				qglUniform4fvARB;
PFNGLGETINFOLOGARBPROC				qglGetInfoLogARB;
PFNGLVERTEXATTRIB4FVPROC			qglVertexAttrib4fv;
PFNGLUNIFORMMATRIX4FVARBPROC		qglUniformMatrix4fvARB;

PFNGLGENFRAMEBUFFERSPROC qglGenFramebuffers;
PFNGLBINDFRAMEBUFFERPROC qglBindFramebuffer;
PFNGLFRAMEBUFFERTEXTURE2DPROC qglFramebufferTexture2D;
PFNGLCHECKFRAMEBUFFERSTATUSPROC qglCheckFramebufferStatus;
PFNGLGENRENDERBUFFERSPROC qglGenRenderbuffers;
PFNGLBINDRENDERBUFFERPROC qglBindRenderbuffer;
PFNGLRENDERBUFFERSTORAGEPROC qglRenderbufferStorage;
PFNGLFRAMEBUFFERRENDERBUFFERPROC qglFramebufferRenderbuffer;
PFNGLDELETERENDERBUFFERSPROC qglDeleteRenderbuffers;
PFNGLDELETEFRAMEBUFFERSPROC qglDeleteFramebuffers;
PFNGLDRAWBUFFERSPROC qglDrawBuffers;

PFNGLGETUNIFORMBLOCKINDEXPROC qglGetUniformBlockIndex;
PFNGLGETACTIVEUNIFORMBLOCKIVPROC qglGetActiveUniformBlockiv;
PFNGLBINDBUFFERBASEPROC qglBindBufferBase ;
PFNGLGETUNIFORMINDICESPROC qglGetUniformIndices;
PFNGLGETACTIVEUNIFORMSIVPROC qglGetActiveUniformsiv;
PFNGLUNIFORMBLOCKBINDINGPROC qglUniformBlockBinding;

/*
=================
R_CheckExtension
=================
*/
bool R_CheckExtension( const char *name, bool required ) {
	if ( !strstr( glConfig.extensions_string, name ) ) {
		if( required ) {
			common->FatalError( "%s was not found with the current OpenGL graphics configuration\n", name );
			return false;
		}
		common->Printf( "X..%s not found\n", name );
		return false;
	}

	common->Printf( "...using %s\n", name );
	return true;
}

/*
==================
R_CheckPortableExtensions
==================
*/
void R_CheckPortableExtensions( void ) {
	glConfig.glVersion = atof( glConfig.version_string );

	// GL_ARB_multitexture
	glConfig.multitextureAvailable = R_CheckExtension( "GL_ARB_multitexture" );
	if ( glConfig.multitextureAvailable ) {
		qglMultiTexCoord2fARB = (void(APIENTRY *)(GLenum, GLfloat, GLfloat))GLimp_ExtensionPointer( "glMultiTexCoord2fARB" );
		qglMultiTexCoord2fvARB = (void(APIENTRY *)(GLenum, GLfloat *))GLimp_ExtensionPointer( "glMultiTexCoord2fvARB" );
		qglActiveTextureARB = (void(APIENTRY *)(GLenum))GLimp_ExtensionPointer( "glActiveTextureARB" );
		qglClientActiveTextureARB = (void(APIENTRY *)(GLenum))GLimp_ExtensionPointer( "glClientActiveTextureARB" );
		qglGetIntegerv( GL_MAX_TEXTURE_UNITS_ARB, (GLint *)&glConfig.maxTextureUnits );
		if ( glConfig.maxTextureUnits > MAX_MULTITEXTURE_UNITS ) {
			glConfig.maxTextureUnits = MAX_MULTITEXTURE_UNITS;
		}
		if ( glConfig.maxTextureUnits < 2 ) {
			glConfig.multitextureAvailable = false;	// shouldn't ever happen
		}
		qglGetIntegerv( GL_MAX_TEXTURE_COORDS_ARB, (GLint *)&glConfig.maxTextureCoords );
		qglGetIntegerv( GL_MAX_TEXTURE_IMAGE_UNITS_ARB, (GLint *)&glConfig.maxTextureImageUnits );
	}

	// GL_ARB_texture_env_combine
	glConfig.textureEnvCombineAvailable = R_CheckExtension( "GL_ARB_texture_env_combine" );

	// GL_ARB_texture_cube_map
	glConfig.cubeMapAvailable = R_CheckExtension( "GL_ARB_texture_cube_map" );

	// GL_ARB_texture_env_dot3
	glConfig.envDot3Available = R_CheckExtension( "GL_ARB_texture_env_dot3" );

	// GL_ARB_texture_env_add
	glConfig.textureEnvAddAvailable = R_CheckExtension( "GL_ARB_texture_env_add" );

	// GL_ARB_texture_non_power_of_two
	glConfig.textureNonPowerOfTwoAvailable = R_CheckExtension( "GL_ARB_texture_non_power_of_two" );

	// GL_ARB_texture_compression + GL_S3_s3tc
	// DRI drivers may have GL_ARB_texture_compression but no GL_EXT_texture_compression_s3tc
	if ( R_CheckExtension( "GL_ARB_texture_compression" ) && R_CheckExtension( "GL_EXT_texture_compression_s3tc" ) ) {
		glConfig.textureCompressionAvailable = true;
		qglCompressedTexImage2DARB = (PFNGLCOMPRESSEDTEXIMAGE2DARBPROC)GLimp_ExtensionPointer( "glCompressedTexImage2DARB" );
		qglGetCompressedTexImageARB = (PFNGLGETCOMPRESSEDTEXIMAGEARBPROC)GLimp_ExtensionPointer( "glGetCompressedTexImageARB" );
	} else {
		glConfig.textureCompressionAvailable = false;
	}

	// GL_EXT_texture_filter_anisotropic
	glConfig.anisotropicAvailable = R_CheckExtension( "GL_EXT_texture_filter_anisotropic" );
	if ( glConfig.anisotropicAvailable ) {
		qglGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &glConfig.maxTextureAnisotropy );
		common->Printf( "   maxTextureAnisotropy: %f\n", glConfig.maxTextureAnisotropy );
	} else {
		glConfig.maxTextureAnisotropy = 1;
	}

	// GL_EXT_texture_lod_bias
	// The actual extension is broken as specificed, storing the state in the texture unit instead
	// of the texture object.  The behavior in GL 1.4 is the behavior we use.
	if ( glConfig.glVersion >= 1.4 || R_CheckExtension( "GL_EXT_texture_lod" ) ) {
		common->Printf( "...using %s\n", "GL_1.4_texture_lod_bias" );
		glConfig.textureLODBiasAvailable = true;
	} else {
		common->Printf( "X..%s not found\n", "GL_1.4_texture_lod_bias" );
		glConfig.textureLODBiasAvailable = false;
	}

	// GL_EXT_shared_texture_palette
	glConfig.sharedTexturePaletteAvailable = R_CheckExtension( "GL_EXT_shared_texture_palette" );
	if ( glConfig.sharedTexturePaletteAvailable ) {
		qglColorTableEXT = ( void ( APIENTRY * ) ( int, int, int, int, int, const void * ) ) GLimp_ExtensionPointer( "glColorTableEXT" );
	}

	// GL_EXT_texture3D (not currently used for anything)
	glConfig.texture3DAvailable = R_CheckExtension( "GL_EXT_texture3D" );
	if ( glConfig.texture3DAvailable ) {
		qglTexImage3D =
			(void (APIENTRY *)(GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *) )
			GLimp_ExtensionPointer( "glTexImage3D" );
	}

	// EXT_stencil_wrap
	// This isn't very important, but some pathological case might cause a clamp error and give a shadow bug.
	// Nvidia also believes that future hardware may be able to run faster with this enabled to avoid the
	// serialization of clamping.
	if ( R_CheckExtension( "GL_EXT_stencil_wrap" ) ) {
		tr.stencilIncr = GL_INCR_WRAP_EXT;
		tr.stencilDecr = GL_DECR_WRAP_EXT;
	} else {
		tr.stencilIncr = GL_INCR;
		tr.stencilDecr = GL_DECR;
	}

	// GL_EXT_stencil_two_side
	glConfig.twoSidedStencilAvailable = R_CheckExtension( "GL_EXT_stencil_two_side" );
	if ( glConfig.twoSidedStencilAvailable )
		qglActiveStencilFaceEXT = (PFNGLACTIVESTENCILFACEEXTPROC)GLimp_ExtensionPointer( "glActiveStencilFaceEXT" );

	if(R_CheckExtension( "GL_ARB_uniform_buffer_object", true)) {
		 qglGetUniformBlockIndex = (PFNGLGETUNIFORMBLOCKINDEXPROC)GLimp_ExtensionPointer( "glGetUniformBlockIndex" );
		 qglGetActiveUniformBlockiv = (PFNGLGETACTIVEUNIFORMBLOCKIVPROC)GLimp_ExtensionPointer( "glGetActiveUniformBlockiv" );
		 qglBindBufferBase = (PFNGLBINDBUFFERBASEPROC)GLimp_ExtensionPointer( "glBindBufferBase" );
		 qglGetUniformIndices = (PFNGLGETUNIFORMINDICESPROC)GLimp_ExtensionPointer( "glGetUniformIndices" );
		 qglGetActiveUniformsiv = (PFNGLGETACTIVEUNIFORMSIVPROC)GLimp_ExtensionPointer( "glGetActiveUniformsiv" );
		 qglUniformBlockBinding = (PFNGLUNIFORMBLOCKBINDINGPROC)GLimp_ExtensionPointer( "glUniformBlockBinding" );
	}

	// GL_ARB_framebuffer_object
	if(R_CheckExtension( "GL_ARB_framebuffer_object", true)) {
		glGetIntegerv( GL_MAX_RENDERBUFFER_SIZE, &glConfig.maxRenderbufferSize );
		glGetIntegerv( GL_MAX_COLOR_ATTACHMENTS, &glConfig.maxColorAttachments );

		qglDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)GLimp_ExtensionPointer( "glDeleteRenderbuffers" );
		qglDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)GLimp_ExtensionPointer( "glDeleteFramebuffers" );
		qglDrawBuffers = (PFNGLDRAWBUFFERSPROC)GLimp_ExtensionPointer( "glDrawBuffers" );
		qglGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)GLimp_ExtensionPointer("glGenFramebuffers");
		qglBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)GLimp_ExtensionPointer("glBindFramebuffer");
		qglFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)GLimp_ExtensionPointer("glFramebufferTexture2D");
		qglCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)GLimp_ExtensionPointer("glCheckFramebufferStatus");
		qglGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)GLimp_ExtensionPointer("glGenRenderbuffers");
		qglBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)GLimp_ExtensionPointer("glBindRenderbuffer");
		qglRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)GLimp_ExtensionPointer("glRenderbufferStorage");
		qglFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)GLimp_ExtensionPointer("glFramebufferRenderbuffer");
	}

	// ARB_vertex_buffer_object
	glConfig.ARBVertexBufferObjectAvailable = R_CheckExtension( "GL_ARB_vertex_buffer_object" );
	if(glConfig.ARBVertexBufferObjectAvailable) {
		qglBindBufferARB = (PFNGLBINDBUFFERARBPROC)GLimp_ExtensionPointer( "glBindBufferARB");
		qglDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC)GLimp_ExtensionPointer( "glDeleteBuffersARB");
		qglGenBuffersARB = (PFNGLGENBUFFERSARBPROC)GLimp_ExtensionPointer( "glGenBuffersARB");
		qglIsBufferARB = (PFNGLISBUFFERARBPROC)GLimp_ExtensionPointer( "glIsBufferARB");
		qglBufferDataARB = (PFNGLBUFFERDATAARBPROC)GLimp_ExtensionPointer( "glBufferDataARB");
		qglBufferSubDataARB = (PFNGLBUFFERSUBDATAARBPROC)GLimp_ExtensionPointer( "glBufferSubDataARB");
		qglGetBufferSubDataARB = (PFNGLGETBUFFERSUBDATAARBPROC)GLimp_ExtensionPointer( "glGetBufferSubDataARB");
		qglMapBufferARB = (PFNGLMAPBUFFERARBPROC)GLimp_ExtensionPointer( "glMapBufferARB");
		qglUnmapBufferARB = (PFNGLUNMAPBUFFERARBPROC)GLimp_ExtensionPointer( "glUnmapBufferARB");
		qglGetBufferParameterivARB = (PFNGLGETBUFFERPARAMETERIVARBPROC)GLimp_ExtensionPointer( "glGetBufferParameterivARB");
		qglGetBufferPointervARB = (PFNGLGETBUFFERPOINTERVARBPROC)GLimp_ExtensionPointer( "glGetBufferPointervARB");
	}

	// GL_ARB_vertex_shader and GL_ARB_fragment_shader
	if(R_CheckExtension( "GL_ARB_vertex_shader", true ) && R_CheckExtension( "GL_ARB_fragment_shader", true )) {
		qglCreateShaderObjectARB = (PFNGLCREATESHADEROBJECTARBPROC)GLimp_ExtensionPointer( "glCreateShaderObjectARB");
		qglDeleteObjectARB = (PFNGLDELETEOBJECTARBPROC)GLimp_ExtensionPointer( "glDeleteObjectARB");
		qglShaderSourceARB = (PFNGLSHADERSOURCEARBPROC)GLimp_ExtensionPointer( "glShaderSourceARB");
		qglCompileShaderARB = (PFNGLCOMPILESHADERARBPROC)GLimp_ExtensionPointer( "glCompileShaderARB");
		qglGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)GLimp_ExtensionPointer( "glCompileShaderARB");
		qglCreateProgramObjectARB = (PFNGLCREATEPROGRAMOBJECTARBPROC)GLimp_ExtensionPointer( "glCreateProgramObjectARB");
		qglAttachObjectARB = (PFNGLATTACHOBJECTARBPROC)GLimp_ExtensionPointer( "glAttachObjectARB");
		qglDetachObjectARB = (PFNGLDETACHOBJECTARBPROC)GLimp_ExtensionPointer( "glDetachObjectARB");
		qglLinkProgramARB = (PFNGLLINKPROGRAMARBPROC)GLimp_ExtensionPointer( "glLinkProgramARB");
		qglUseProgramObjectARB = (PFNGLUSEPROGRAMOBJECTARBPROC)GLimp_ExtensionPointer( "glUseProgramObjectARB");
		qglGetUniformLocationARB = (PFNGLGETUNIFORMLOCATIONARBPROC)GLimp_ExtensionPointer( "glGetUniformLocationARB");
		//qglGetAttributeLocationARB  = (PFNGLGETATTRIBLOCATIONARBPROC)GLimp_ExtensionPointer( "glGetAttributeLocationARB");
		qglUniform1fARB = (PFNGLUNIFORM1FARBPROC)GLimp_ExtensionPointer( "glUniform1fARB");
		qglUniform1iARB = (PFNGLUNIFORM1IARBPROC)GLimp_ExtensionPointer( "glUniform1iARB");
		qglUniform1fvARB = (PFNGLUNIFORM1FVARBPROC)GLimp_ExtensionPointer( "glUniform1fvARB");
		qglUniform2fvARB = (PFNGLUNIFORM2FVARBPROC)GLimp_ExtensionPointer( "glUniform2fvARB");
		qglUniform3fvARB = (PFNGLUNIFORM3FVARBPROC)GLimp_ExtensionPointer( "glUniform3fvARB");
		qglUniform4fvARB = (PFNGLUNIFORM4FVARBPROC)GLimp_ExtensionPointer( "glUniform4fvARB");
		qglGetInfoLogARB = (PFNGLGETINFOLOGARBPROC)GLimp_ExtensionPointer( "glGetInfoLogARB");
		qglVertexAttrib4fv = (PFNGLVERTEXATTRIB4FVPROC)GLimp_ExtensionPointer( "glVertexAttrib4fv");
		qglUniformMatrix4fvARB = (PFNGLUNIFORMMATRIX4FVARBPROC)GLimp_ExtensionPointer( "glUniformMatrix4fvARB");
	}

	// ARB_vertex_program
	glConfig.ARBVertexProgramAvailable = R_CheckExtension( "GL_ARB_vertex_program" );
	if (glConfig.ARBVertexProgramAvailable) {
		qglVertexAttribPointerARB = (PFNGLVERTEXATTRIBPOINTERARBPROC)GLimp_ExtensionPointer( "glVertexAttribPointerARB" );
		qglEnableVertexAttribArrayARB = (PFNGLENABLEVERTEXATTRIBARRAYARBPROC)GLimp_ExtensionPointer( "glEnableVertexAttribArrayARB" );
		qglDisableVertexAttribArrayARB = (PFNGLDISABLEVERTEXATTRIBARRAYARBPROC)GLimp_ExtensionPointer( "glDisableVertexAttribArrayARB" );
		qglBindAttribLocationARB = (PFNGLBINDATTRIBLOCATIONARBPROC)GLimp_ExtensionPointer( "glBindAttribLocationARB" );
	}

	// GL_EXT_depth_bounds_test
	glConfig.depthBoundsTestAvailable = R_CheckExtension( "EXT_depth_bounds_test" );
	if ( glConfig.depthBoundsTestAvailable ) {
		qglDepthBoundsEXT = (PFNGLDEPTHBOUNDSEXTPROC)GLimp_ExtensionPointer( "glDepthBoundsEXT" );
	}

	// check for minimum set
	if ( !glConfig.multitextureAvailable || !glConfig.textureEnvCombineAvailable || !glConfig.cubeMapAvailable
		|| !glConfig.envDot3Available ) {
			common->Error( common->GetLanguageDict()->GetString( "#str_06780" ) );
	}
}
