// RenderProgram.cpp

#include "sys/platform.h"

#include "renderer/tr_local.h"

idRenderProgram *activeRenderProgram = NULL;

/*
===================
idRenderProgram::idRenderProgram
===================
*/
idRenderProgram::idRenderProgram( const char *path ) {
	memset( &handles, 0, sizeof( unsigned int ) * RENDERPROGRAM_MAXTYPES);
	programHandle = 0;
	Reload( path );
}

/*
===================
idRenderProgram::Free
===================
*/
void idRenderProgram::Free( void ) {
	for(int i = 0; i < RENDERPROGRAM_MAXTYPES; i++ ) {
		if(handles[i] > 0) {
			qglDeleteObjectARB( handles[i] );
		}

		handles[i] = 0;
	}

	if(programHandle > 0) {
		common->Printf("......Clearing Previous Instance\n" );
		qglDeleteObjectARB( programHandle );
		programHandle = 0;
		memset( &progVars[0], -1, sizeof(glRenderProgramVarHandle_t) * 40 );
	}
}

/*
===================
idRenderProgram::SetVar1fi
===================
*/
void idRenderProgram::SetVar1fi( unsigned int progVarHandle, int var ) {
	if(progVars[progVarHandle] == -1) {
		return;
	}
	qglUniform1iARB( progVars[progVarHandle], var );
}

/*
===================
idRenderProgram::SetVar2fv
===================
*/
void idRenderProgram::SetVar2fv( unsigned int progVarHandle, int count, const float *ptr ) {
	if(progVars[progVarHandle] == -1) {
		return;
	}
	qglUniform2fvARB( progVars[progVarHandle], count, ptr );
}

/*
===================
idRenderProgram::SetVar3fv
===================
*/
void idRenderProgram::SetVar3fv( unsigned int progVarHandle, int count, const float *ptr ) {
	if(progVars[progVarHandle] == -1) {
		return;
	}
	qglUniform3fvARB( progVars[progVarHandle], count, ptr );
}

/*
===================
idRenderProgram::BindTextureVar
===================
*/
void idRenderProgram::BindTextureVar( unsigned int progVarHandle ) {
	if(progVars[progVarHandle] == -1) {
		return;
	}
	qglUniform1iARB( progVars[progVarHandle], backEnd.glState.currenttmu );
}

/*
===================
idRenderProgram::Reload
===================
*/
void idRenderProgram::Reload( const char *path ) {
	idStr programBuffer;
	idStr vertexProgBuffer, fragmentProgBuffer;

	name = path;

	Free();

	programHandle = qglCreateProgramObjectARB();
	programBuffer = ReadSourceFile( path );

	PreprocessProgram(programBuffer, vertexProgBuffer, fragmentProgBuffer);

	LoadShader( vertexProgBuffer.c_str(), GL_VERTEX_SHADER_ARB );

	BindVar( PP_ATTR_TEXCOORD, 8, "attr_TexCoord" );
	BindVar( PP_ATTR_TANGENT, 9, "attr_Tangent" );
	BindVar( PP_ATTR_BINORMAL, 10, "attr_Binormal" );
	BindVar( PP_ATTR_NORMAL, 11, "attr_Normal" );

	LoadShader( fragmentProgBuffer.c_str(), GL_FRAGMENT_SHADER_ARB);
	qglLinkProgramARB(programHandle);
	
	// Try to bind all the default vars.
	BindVar( PP_LIGHT_ORIGIN, "u_lightOrigin" );
	BindVar( PP_VIEW_ORIGIN, "u_viewOrigin" );
	BindVar( PP_LIGHT_PROJECT_S, "r_lightProjectionS" );
	BindVar( PP_LIGHT_PROJECT_T, "r_lightProjectionT" );
	BindVar( PP_LIGHT_PROJECT_Q, "r_lightProjectionQ" );
	BindVar( PP_LIGHT_FALLOFF_S, "r_lightFalloff" );
	BindVar( PP_LIGHT_COLOR, "u_lightColor" );

	BindVar( PP_COLOR_MODULATE, "u_colorModulate" );
	BindVar( PP_COLOR_ADD, "u_colorAdd" );

	BindVar( PP_TEX_NORMAL, "u_normalTexture" );
	BindVar( PP_TEX_LIGHTFALLOFF, "u_lightFalloffTexture" );
	BindVar( PP_TEX_LIGHTPROJECTION, "u_lightProjectionTexture" );
	BindVar( PP_TEX_DIFFUSE, "u_diffuseTexture" );
	BindVar( PP_TEX_SPEC, "u_specularTexture" );

	BindVar(PP_DIFFUSE_MATRIX_S, "u_diffuseMatrixS" );
	BindVar(PP_DIFFUSE_MATRIX_T, "u_diffuseMatrixT" );
	
	BindVar( VV_TEX_VERTEX, "u_vertTexture" );

	BindVar( VV_TEX_MATVIEW, "u_matview" );

	BindVar( PP_TEX_CURRENT, "u_CurrentMap" );

/*
	BindVar( PP_BUMP_MATRIX_S,
	BindVar( PP_BUMP_MATRIX_T,

	BindVar( PP_SPECULAR_MATRIX_S,
	BindVar( PP_SPECULAR_MATRIX_T,
*/
}

/*
===================
idRenderProgram::SetVar4fv
===================
*/
void idRenderProgram::SetVar4fv( unsigned int progVarHandle, const float *ptr ) {
	if(progVars[progVarHandle] == -1) {
		return;
	}
	SetVar4fv( progVarHandle, 1, ptr );
}

/*
===================
idRenderProgram::SetVar4fv
===================
*/
void idRenderProgram::SetVar4fv( unsigned int progVarHandle, int count, const float *ptr ) {
	if(progVars[progVarHandle] == -1) {
		return;
	}

	qglUniform4fvARB( progVars[progVarHandle], count, ptr );
}

/*
===================
idRenderProgram::SetVar4fv
===================
*/
void idRenderProgram::SetVarMatrix4fv( unsigned int progVarHandle, int count, const float *ptr ) {
	if(progVars[progVarHandle] == -1) {
		return;
	}

	qglUniformMatrix4fvARB( progVars[progVarHandle], count, false, ptr );
}


/*
===================
idRenderProgram::SetAttrVar4fv
===================
*/
void idRenderProgram::SetAttrVar4fv( unsigned int progVarHandle, const float *ptr ) {
	if(progVars[progVarHandle] == -1) {
		return;
	}

	qglVertexAttrib4fv( progVars[progVarHandle], ptr );
}



/*
===================
idRenderProgram::LoadShader
===================
*/
void idRenderProgram::LoadShader( const char *buffer, unsigned int shadertype ) {
	char str[4096];
	idStr shaderBuffer = buffer;
	glRenderProgramHandle_t handle;
	
	// Create the GLSL shader handle.
	handle = qglCreateShaderObjectARB(shadertype);

	// Load in the shader sources.
	qglShaderSourceARB(handle, 1, &buffer, NULL);
	qglCompileShaderARB(handle);
	qglGetInfoLogARB( handle, sizeof(str), NULL, str );

	if (strlen( str ) > 0 && strstr(str, "error")) {
		idStr buffer = FormatShaderError( shaderBuffer.c_str(), str );
		common->FatalError( "LoadShader(%s): %s\n", name.c_str(), buffer.c_str() );
		return;
	}

	qglAttachObjectARB(programHandle, handle);
}

/*
===================
idRenderProgram::FormatShaderError
===================
*/
idStr idRenderProgram::FormatShaderError( const char *buffer, const char *errorBuffer ) {
	idStr finalBuffer;
	const char *buferpos = errorBuffer;
	int i = 0, d= 0;

	while( true ) {
		const char *token = strstr( buferpos, "(" );
		const char *token2 = strstr( buferpos, ")" );
		char temp[5];

		i = 0;

		if(token == NULL)
			break;

		for(i = (int)token, d=0; i < (int)token2; i++, d++) {
			temp[d] = token[d+ 1];
		}
		temp[d] = 0;
		buferpos = token2 + 1;
		finalBuffer += GetErrorLine( atoi(temp), buffer );
		finalBuffer += "\n\r";
	}

	idStr errStr = "Shader Compiler Error:\n\rError Line Data;\n\r" + finalBuffer;
	errStr += "\n\rGPU Error(s)\n\r";
	errStr += errorBuffer;

	return errStr;
}

/*
===================
idRenderProgram::GetErrorLine
===================
*/
idStr idRenderProgram::GetErrorLine( int line, const char *buffer ) {
	int currentLine = 0;
	const char *buf = (char*)buffer;
	for(int i = 0; i < strlen(buffer); i++) {
		if(currentLine >= line) {
			buf += i;
			break;
		}

		if(buffer[i] == '\n' || buffer[i] == '\r') {
			currentLine++;
		}
	}

	idStr finalBuffer = buf;

	for(int i = 0; i < finalBuffer.Length(); i++) {
		if(buf[i] == '\n' || buf[i] == '\r') {
			finalBuffer[i] = 0;
			break;
		}
	}
	
	return &finalBuffer[0];
}

/*
===================
idRenderProgram::Bind
===================
*/
void idRenderProgram::Bind( void ) {
	if(activeRenderProgram != NULL) {
		common->FatalError("RenderProgram::Bind: Program already bound");
	}
	activeRenderProgram = this;
	qglUseProgramObjectARB(programHandle);
}

/*
===================
idRenderProgram::UnBind
===================
*/
void idRenderProgram::UnBind( void ) {
	activeRenderProgram = NULL;
	qglUseProgramObjectARB(NULL);
}

/*
===================
idRenderProgram::BindVar
===================
*/
void idRenderProgram::BindVar( unsigned int progVarHandle, const char *varName ) {
	progVars[progVarHandle] = qglGetUniformLocationARB( programHandle, varName );
}

/*
===================
idRenderProgram::BindVar
===================
*/
void idRenderProgram::BindVar( unsigned int progVarHandle, int attrId, const char *varName ) {
	qglBindAttribLocationARB( programHandle, attrId, varName );
	progVars[progVarHandle] = attrId;
}

/*
===================
idRenderProgram::PreprocessProgram
===================
*/
void idRenderProgram::PreprocessProgram( idStr &buffer, idStr &vertexProgBuffer, idStr &fragmentProgBuffer ) {
	idParser parser( buffer.c_str(), buffer.Length(), name.c_str(), LEXFL_NOFATALERRORS );
	idToken token;
	idStr preprocessedStr = "#version 150\n\r";

	// Parse through the program.
	while(parser.ReadToken( &token )) {
		preprocessedStr += token;
		if(token.linesCrossed) {
			preprocessedStr += "\n";
		}
		else {
			preprocessedStr += " ";
		}
	}

	vertexProgBuffer = preprocessedStr;
	fragmentProgBuffer = preprocessedStr;

	vertexProgBuffer.Replace( "EndFragmentProgram", "*/" );
	vertexProgBuffer.Replace( "FragmentProgram", "/*" );
	vertexProgBuffer.Replace( "EndVertexProgram", " " );
	vertexProgBuffer.Replace( "VertexProgram", " " );
	
	fragmentProgBuffer.Replace( "EndVertexProgram", "*/" );
	fragmentProgBuffer.Replace( "VertexProgram", "/*" );
	fragmentProgBuffer.Replace( "EndFragmentProgram", " " );
	fragmentProgBuffer.Replace( "FragmentProgram", " " );
	
}

/*
===================
idRenderProgram::ReadSourceFile
===================
*/
idStr idRenderProgram::ReadSourceFile( const char *path ) {
	idStr srcBuffer;
	char *rawBuffer;

	if(fileSystem->ReadFile( va("renderprogs/%s", path), (void **)&rawBuffer ) <= 0) {
		common->FatalError( "idRenderProgram::ReadSourceFile: Failed to open %s\n", path );
	}

	srcBuffer = rawBuffer;

	fileSystem->FreeFile( rawBuffer );

	return srcBuffer;
}