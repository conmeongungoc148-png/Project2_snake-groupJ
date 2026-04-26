/**********************************************************************************************
*
*   rlgl v4.5 - A multi-OpenGL abstraction layer with an immediate-mode style API
*
*   DESCRIPTION:
*       An abstraction layer for multiple OpenGL versions (1.1, 2.1, 3.3 Core, 4.3 Core, ES 2.0)
*       that provides a pseudo-OpenGL 1.1 immediate-mode style API (rlVertex, rlTranslate, rlRotate...)
*
*   ADDITIONAL NOTES:
*       When choosing an OpenGL backend different than OpenGL 1.1, some internal buffer are
*       initialized on rlglInit() to accumulate vertex data.
*
*       When an internal state change is required all the stored vertex data is renderer in batch,
*       additionally, rlDrawRenderBatchActive() could be called to force flushing of the batch.
*
*       Some resources are also loaded for convenience, here the complete list:
*          - Default batch (RLGL.defaultBatch): RenderBatch system to accumulate vertex data
*          - Default texture (RLGL.defaultTextureId): 1x1 white pixel R8G8B8A8
*          - Default shader (RLGL.State.defaultShaderId, RLGL.State.defaultShaderLocs)
*
*       Internal buffer (and resources) must be manually unloaded calling rlglClose().
*
*   CONFIGURATION:
*       #define GRAPHICS_API_OPENGL_11
*       #define GRAPHICS_API_OPENGL_21
*       #define GRAPHICS_API_OPENGL_33
*       #define GRAPHICS_API_OPENGL_43
*       #define GRAPHICS_API_OPENGL_ES2
*       #define GRAPHICS_API_OPENGL_ES3
*           Use selected OpenGL graphics backend, should be supported by platform
*           Those preprocessor defines are only used on rlgl module, if OpenGL version is
*           required by any other module, use rlGetVersion() to check it
*
*       #define RLGL_IMPLEMENTATION
*           Generates the implementation of the library into the included file.
*           If not defined, the library is in header only mode and can be included in other headers
*           or source files without problems. But only ONE file should hold the implementation.
*
*       #define RLGL_RENDER_TEXTURES_HINT
*           Enable framebuffer objects (fbo) support (enabled by default)
*           Some GPUs could not support them despite the OpenGL version
*
*       #define RLGL_SHOW_GL_DETAILS_INFO
*           Show OpenGL extensions and capabilities detailed logs on init
*
*       #define RLGL_ENABLE_OPENGL_DEBUG_CONTEXT
*           Enable debug context (only available on OpenGL 4.3)
*
*       rlgl capabilities could be customized just defining some internal
*       values before library inclusion (default values listed):
*
*       #define RL_DEFAULT_BATCH_BUFFER_ELEMENTS   8192    
*       #define RL_DEFAULT_BATCH_BUFFERS              1    
*       #define RL_DEFAULT_BATCH_DRAWCALLS          256    
*       #define RL_DEFAULT_BATCH_MAX_TEXTURE_UNITS    4    
*
*       #define RL_MAX_MATRIX_STACK_SIZE             32    
*       #define RL_MAX_SHADER_LOCATIONS              32    
*       #define RL_CULL_DISTANCE_NEAR              0.01    
*       #define RL_CULL_DISTANCE_FAR             1000.0    
*
*       When loading a shader, the following vertex attributes and uniform
*       location names are tried to be set automatically:
*
*       #define RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION     "vertexPosition"    
*       #define RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD     "vertexTexCoord"    
*       #define RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL       "vertexNormal"      
*       #define RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR        "vertexColor"       
*       #define RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT      "vertexTangent"     
*       #define RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2    "vertexTexCoord2"   
*       #define RL_DEFAULT_SHADER_UNIFORM_NAME_MVP         "mvp"               
*       #define RL_DEFAULT_SHADER_UNIFORM_NAME_VIEW        "matView"           
*       #define RL_DEFAULT_SHADER_UNIFORM_NAME_PROJECTION  "matProjection"     
*       #define RL_DEFAULT_SHADER_UNIFORM_NAME_MODEL       "matModel"          
*       #define RL_DEFAULT_SHADER_UNIFORM_NAME_NORMAL      "matNormal"         
*       #define RL_DEFAULT_SHADER_UNIFORM_NAME_COLOR       "colDiffuse"        
*       #define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE0  "texture0"          
*       #define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE1  "texture1"          
*       #define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE2  "texture2"          
*
*   DEPENDENCIES:
*      - OpenGL libraries (depending on platform and OpenGL version selected)
*      - GLAD OpenGL extensions loading library (only for OpenGL 3.3 Core, 4.3 Core)
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2014-2023 Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#ifndef RLGL_H
#define RLGL_H

#define RLGL_VERSION  "4.5"



#if defined(_WIN32)
    #if defined(BUILD_LIBTYPE_SHARED)
        #define RLAPI __declspec(dllexport)     
    #elif defined(USE_LIBTYPE_SHARED)
        #define RLAPI __declspec(dllimport)     
    #endif
#endif


#ifndef RLAPI
    #define RLAPI       
#endif


#ifndef TRACELOG
    #define TRACELOG(level, ...) (void)0
    #define TRACELOGD(...) (void)0
#endif


#ifndef RL_MALLOC
    #define RL_MALLOC(sz)     malloc(sz)
#endif
#ifndef RL_CALLOC
    #define RL_CALLOC(n,sz)   calloc(n,sz)
#endif
#ifndef RL_REALLOC
    #define RL_REALLOC(n,sz)  realloc(n,sz)
#endif
#ifndef RL_FREE
    #define RL_FREE(p)        free(p)
#endif


#if !defined(GRAPHICS_API_OPENGL_11) && \
    !defined(GRAPHICS_API_OPENGL_21) && \
    !defined(GRAPHICS_API_OPENGL_33) && \
    !defined(GRAPHICS_API_OPENGL_43) && \
    !defined(GRAPHICS_API_OPENGL_ES2) && \
    !defined(GRAPHICS_API_OPENGL_ES3)
        #define GRAPHICS_API_OPENGL_33
#endif


#if defined(GRAPHICS_API_OPENGL_11)
    #if defined(GRAPHICS_API_OPENGL_21)
        #undef GRAPHICS_API_OPENGL_21
    #endif
    #if defined(GRAPHICS_API_OPENGL_33)
        #undef GRAPHICS_API_OPENGL_33
    #endif
    #if defined(GRAPHICS_API_OPENGL_43)
        #undef GRAPHICS_API_OPENGL_43
    #endif
    #if defined(GRAPHICS_API_OPENGL_ES2)
        #undef GRAPHICS_API_OPENGL_ES2
    #endif
#endif



#if defined(GRAPHICS_API_OPENGL_21)
    #define GRAPHICS_API_OPENGL_33
#endif


#if defined(GRAPHICS_API_OPENGL_43)
    #define GRAPHICS_API_OPENGL_33
#endif


#if defined(GRAPHICS_API_OPENGL_ES3)
    #define GRAPHICS_API_OPENGL_ES2
#endif



#define RLGL_RENDER_TEXTURES_HINT






#ifndef RL_DEFAULT_BATCH_BUFFER_ELEMENTS
    #if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
        
        
        #define RL_DEFAULT_BATCH_BUFFER_ELEMENTS  8192
    #endif
    #if defined(GRAPHICS_API_OPENGL_ES2)
        
        
        
        #define RL_DEFAULT_BATCH_BUFFER_ELEMENTS  2048
    #endif
#endif
#ifndef RL_DEFAULT_BATCH_BUFFERS
    #define RL_DEFAULT_BATCH_BUFFERS                 1      
#endif
#ifndef RL_DEFAULT_BATCH_DRAWCALLS
    #define RL_DEFAULT_BATCH_DRAWCALLS             256      
#endif
#ifndef RL_DEFAULT_BATCH_MAX_TEXTURE_UNITS
    #define RL_DEFAULT_BATCH_MAX_TEXTURE_UNITS       4      
#endif


#ifndef RL_MAX_MATRIX_STACK_SIZE
    #define RL_MAX_MATRIX_STACK_SIZE                32      
#endif


#ifndef RL_MAX_SHADER_LOCATIONS
    #define RL_MAX_SHADER_LOCATIONS                 32      
#endif


#ifndef RL_CULL_DISTANCE_NEAR
    #define RL_CULL_DISTANCE_NEAR                 0.01      
#endif
#ifndef RL_CULL_DISTANCE_FAR
    #define RL_CULL_DISTANCE_FAR                1000.0      
#endif


#define RL_TEXTURE_WRAP_S                       0x2802      
#define RL_TEXTURE_WRAP_T                       0x2803      
#define RL_TEXTURE_MAG_FILTER                   0x2800      
#define RL_TEXTURE_MIN_FILTER                   0x2801      

#define RL_TEXTURE_FILTER_NEAREST               0x2600      
#define RL_TEXTURE_FILTER_LINEAR                0x2601      
#define RL_TEXTURE_FILTER_MIP_NEAREST           0x2700      
#define RL_TEXTURE_FILTER_NEAREST_MIP_LINEAR    0x2702      
#define RL_TEXTURE_FILTER_LINEAR_MIP_NEAREST    0x2701      
#define RL_TEXTURE_FILTER_MIP_LINEAR            0x2703      
#define RL_TEXTURE_FILTER_ANISOTROPIC           0x3000      
#define RL_TEXTURE_MIPMAP_BIAS_RATIO            0x4000      

#define RL_TEXTURE_WRAP_REPEAT                  0x2901      
#define RL_TEXTURE_WRAP_CLAMP                   0x812F      
#define RL_TEXTURE_WRAP_MIRROR_REPEAT           0x8370      
#define RL_TEXTURE_WRAP_MIRROR_CLAMP            0x8742      


#define RL_MODELVIEW                            0x1700      
#define RL_PROJECTION                           0x1701      
#define RL_TEXTURE                              0x1702      


#define RL_LINES                                0x0001      
#define RL_TRIANGLES                            0x0004      
#define RL_QUADS                                0x0007      


#define RL_UNSIGNED_BYTE                        0x1401      
#define RL_FLOAT                                0x1406      


#define RL_STREAM_DRAW                          0x88E0      
#define RL_STREAM_READ                          0x88E1      
#define RL_STREAM_COPY                          0x88E2      
#define RL_STATIC_DRAW                          0x88E4      
#define RL_STATIC_READ                          0x88E5      
#define RL_STATIC_COPY                          0x88E6      
#define RL_DYNAMIC_DRAW                         0x88E8      
#define RL_DYNAMIC_READ                         0x88E9      
#define RL_DYNAMIC_COPY                         0x88EA      


#define RL_FRAGMENT_SHADER                      0x8B30      
#define RL_VERTEX_SHADER                        0x8B31      
#define RL_COMPUTE_SHADER                       0x91B9      


#define RL_ZERO                                 0           
#define RL_ONE                                  1           
#define RL_SRC_COLOR                            0x0300      
#define RL_ONE_MINUS_SRC_COLOR                  0x0301      
#define RL_SRC_ALPHA                            0x0302      
#define RL_ONE_MINUS_SRC_ALPHA                  0x0303      
#define RL_DST_ALPHA                            0x0304      
#define RL_ONE_MINUS_DST_ALPHA                  0x0305      
#define RL_DST_COLOR                            0x0306      
#define RL_ONE_MINUS_DST_COLOR                  0x0307      
#define RL_SRC_ALPHA_SATURATE                   0x0308      
#define RL_CONSTANT_COLOR                       0x8001      
#define RL_ONE_MINUS_CONSTANT_COLOR             0x8002      
#define RL_CONSTANT_ALPHA                       0x8003      
#define RL_ONE_MINUS_CONSTANT_ALPHA             0x8004      


#define RL_FUNC_ADD                             0x8006      
#define RL_MIN                                  0x8007      
#define RL_MAX                                  0x8008      
#define RL_FUNC_SUBTRACT                        0x800A      
#define RL_FUNC_REVERSE_SUBTRACT                0x800B      
#define RL_BLEND_EQUATION                       0x8009      
#define RL_BLEND_EQUATION_RGB                   0x8009      
#define RL_BLEND_EQUATION_ALPHA                 0x883D      
#define RL_BLEND_DST_RGB                        0x80C8      
#define RL_BLEND_SRC_RGB                        0x80C9      
#define RL_BLEND_DST_ALPHA                      0x80CA      
#define RL_BLEND_SRC_ALPHA                      0x80CB      
#define RL_BLEND_COLOR                          0x8005      





#if (defined(__STDC__) && __STDC_VERSION__ >= 199901L) || (defined(_MSC_VER) && _MSC_VER >= 1800)
    #include <stdbool.h>
#elif !defined(__cplusplus) && !defined(bool) && !defined(RL_BOOL_TYPE)
    
typedef enum bool { false = 0, true = !false } bool;
#endif

#if !defined(RL_MATRIX_TYPE)

typedef struct Matrix {
    float m0, m4, m8, m12;      
    float m1, m5, m9, m13;      
    float m2, m6, m10, m14;     
    float m3, m7, m11, m15;     
} Matrix;
#define RL_MATRIX_TYPE
#endif


typedef struct rlVertexBuffer {
    int elementCount;           

    float *vertices;            
    float *texcoords;           
    unsigned char *colors;      
#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
    unsigned int *indices;      
#endif
#if defined(GRAPHICS_API_OPENGL_ES2)
    unsigned short *indices;    
#endif
    unsigned int vaoId;         
    unsigned int vboId[4];      
} rlVertexBuffer;





typedef struct rlDrawCall {
    int mode;                   
    int vertexCount;            
    int vertexAlignment;        
    
    
    unsigned int textureId;     

    
    
} rlDrawCall;


typedef struct rlRenderBatch {
    int bufferCount;            
    int currentBuffer;          
    rlVertexBuffer *vertexBuffer; 

    rlDrawCall *draws;          
    int drawCounter;            
    float currentDepth;         
} rlRenderBatch;


typedef enum {
    RL_OPENGL_11 = 1,           
    RL_OPENGL_21,               
    RL_OPENGL_33,               
    RL_OPENGL_43,               
    RL_OPENGL_ES_20,            
    RL_OPENGL_ES_30             
} rlGlVersion;



typedef enum {
    RL_LOG_ALL = 0,             
    RL_LOG_TRACE,               
    RL_LOG_DEBUG,               
    RL_LOG_INFO,                
    RL_LOG_WARNING,             
    RL_LOG_ERROR,               
    RL_LOG_FATAL,               
    RL_LOG_NONE                 
} rlTraceLogLevel;



typedef enum {
    RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE = 1,     
    RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA,        
    RL_PIXELFORMAT_UNCOMPRESSED_R5G6B5,            
    RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8,            
    RL_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1,          
    RL_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4,          
    RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,          
    RL_PIXELFORMAT_UNCOMPRESSED_R32,               
    RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32,         
    RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32,      
    RL_PIXELFORMAT_UNCOMPRESSED_R16,               
    RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16,         
    RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16,      
    RL_PIXELFORMAT_COMPRESSED_DXT1_RGB,            
    RL_PIXELFORMAT_COMPRESSED_DXT1_RGBA,           
    RL_PIXELFORMAT_COMPRESSED_DXT3_RGBA,           
    RL_PIXELFORMAT_COMPRESSED_DXT5_RGBA,           
    RL_PIXELFORMAT_COMPRESSED_ETC1_RGB,            
    RL_PIXELFORMAT_COMPRESSED_ETC2_RGB,            
    RL_PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA,       
    RL_PIXELFORMAT_COMPRESSED_PVRT_RGB,            
    RL_PIXELFORMAT_COMPRESSED_PVRT_RGBA,           
    RL_PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA,       
    RL_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA        
} rlPixelFormat;




typedef enum {
    RL_TEXTURE_FILTER_POINT = 0,        
    RL_TEXTURE_FILTER_BILINEAR,         
    RL_TEXTURE_FILTER_TRILINEAR,        
    RL_TEXTURE_FILTER_ANISOTROPIC_4X,   
    RL_TEXTURE_FILTER_ANISOTROPIC_8X,   
    RL_TEXTURE_FILTER_ANISOTROPIC_16X,  
} rlTextureFilter;


typedef enum {
    RL_BLEND_ALPHA = 0,                 
    RL_BLEND_ADDITIVE,                  
    RL_BLEND_MULTIPLIED,                
    RL_BLEND_ADD_COLORS,                
    RL_BLEND_SUBTRACT_COLORS,           
    RL_BLEND_ALPHA_PREMULTIPLY,         
    RL_BLEND_CUSTOM,                    
    RL_BLEND_CUSTOM_SEPARATE            
} rlBlendMode;


typedef enum {
    RL_SHADER_LOC_VERTEX_POSITION = 0,  
    RL_SHADER_LOC_VERTEX_TEXCOORD01,    
    RL_SHADER_LOC_VERTEX_TEXCOORD02,    
    RL_SHADER_LOC_VERTEX_NORMAL,        
    RL_SHADER_LOC_VERTEX_TANGENT,       
    RL_SHADER_LOC_VERTEX_COLOR,         
    RL_SHADER_LOC_MATRIX_MVP,           
    RL_SHADER_LOC_MATRIX_VIEW,          
    RL_SHADER_LOC_MATRIX_PROJECTION,    
    RL_SHADER_LOC_MATRIX_MODEL,         
    RL_SHADER_LOC_MATRIX_NORMAL,        
    RL_SHADER_LOC_VECTOR_VIEW,          
    RL_SHADER_LOC_COLOR_DIFFUSE,        
    RL_SHADER_LOC_COLOR_SPECULAR,       
    RL_SHADER_LOC_COLOR_AMBIENT,        
    RL_SHADER_LOC_MAP_ALBEDO,           
    RL_SHADER_LOC_MAP_METALNESS,        
    RL_SHADER_LOC_MAP_NORMAL,           
    RL_SHADER_LOC_MAP_ROUGHNESS,        
    RL_SHADER_LOC_MAP_OCCLUSION,        
    RL_SHADER_LOC_MAP_EMISSION,         
    RL_SHADER_LOC_MAP_HEIGHT,           
    RL_SHADER_LOC_MAP_CUBEMAP,          
    RL_SHADER_LOC_MAP_IRRADIANCE,       
    RL_SHADER_LOC_MAP_PREFILTER,        
    RL_SHADER_LOC_MAP_BRDF              
} rlShaderLocationIndex;

#define RL_SHADER_LOC_MAP_DIFFUSE       RL_SHADER_LOC_MAP_ALBEDO
#define RL_SHADER_LOC_MAP_SPECULAR      RL_SHADER_LOC_MAP_METALNESS


typedef enum {
    RL_SHADER_UNIFORM_FLOAT = 0,        
    RL_SHADER_UNIFORM_VEC2,             
    RL_SHADER_UNIFORM_VEC3,             
    RL_SHADER_UNIFORM_VEC4,             
    RL_SHADER_UNIFORM_INT,              
    RL_SHADER_UNIFORM_IVEC2,            
    RL_SHADER_UNIFORM_IVEC3,            
    RL_SHADER_UNIFORM_IVEC4,            
    RL_SHADER_UNIFORM_SAMPLER2D         
} rlShaderUniformDataType;


typedef enum {
    RL_SHADER_ATTRIB_FLOAT = 0,         
    RL_SHADER_ATTRIB_VEC2,              
    RL_SHADER_ATTRIB_VEC3,              
    RL_SHADER_ATTRIB_VEC4               
} rlShaderAttributeDataType;



typedef enum {
    RL_ATTACHMENT_COLOR_CHANNEL0 = 0,       
    RL_ATTACHMENT_COLOR_CHANNEL1 = 1,       
    RL_ATTACHMENT_COLOR_CHANNEL2 = 2,       
    RL_ATTACHMENT_COLOR_CHANNEL3 = 3,       
    RL_ATTACHMENT_COLOR_CHANNEL4 = 4,       
    RL_ATTACHMENT_COLOR_CHANNEL5 = 5,       
    RL_ATTACHMENT_COLOR_CHANNEL6 = 6,       
    RL_ATTACHMENT_COLOR_CHANNEL7 = 7,       
    RL_ATTACHMENT_DEPTH = 100,              
    RL_ATTACHMENT_STENCIL = 200,            
} rlFramebufferAttachType;


typedef enum {
    RL_ATTACHMENT_CUBEMAP_POSITIVE_X = 0,   
    RL_ATTACHMENT_CUBEMAP_NEGATIVE_X = 1,   
    RL_ATTACHMENT_CUBEMAP_POSITIVE_Y = 2,   
    RL_ATTACHMENT_CUBEMAP_NEGATIVE_Y = 3,   
    RL_ATTACHMENT_CUBEMAP_POSITIVE_Z = 4,   
    RL_ATTACHMENT_CUBEMAP_NEGATIVE_Z = 5,   
    RL_ATTACHMENT_TEXTURE2D = 100,          
    RL_ATTACHMENT_RENDERBUFFER = 200,       
} rlFramebufferAttachTextureType;


typedef enum {
    RL_CULL_FACE_FRONT = 0,
    RL_CULL_FACE_BACK
} rlCullMode;





#if defined(__cplusplus)
extern "C" {            
#endif

RLAPI void rlMatrixMode(int mode);                    
RLAPI void rlPushMatrix(void);                        
RLAPI void rlPopMatrix(void);                         
RLAPI void rlLoadIdentity(void);                      
RLAPI void rlTranslatef(float x, float y, float z);   
RLAPI void rlRotatef(float angle, float x, float y, float z);  
RLAPI void rlScalef(float x, float y, float z);       
RLAPI void rlMultMatrixf(const float *matf);                
RLAPI void rlFrustum(double left, double right, double bottom, double top, double znear, double zfar);
RLAPI void rlOrtho(double left, double right, double bottom, double top, double znear, double zfar);
RLAPI void rlViewport(int x, int y, int width, int height); 




RLAPI void rlBegin(int mode);                         
RLAPI void rlEnd(void);                               
RLAPI void rlVertex2i(int x, int y);                  
RLAPI void rlVertex2f(float x, float y);              
RLAPI void rlVertex3f(float x, float y, float z);     
RLAPI void rlTexCoord2f(float x, float y);            
RLAPI void rlNormal3f(float x, float y, float z);     
RLAPI void rlColor4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a);  
RLAPI void rlColor3f(float x, float y, float z);          
RLAPI void rlColor4f(float x, float y, float z, float w); 








RLAPI bool rlEnableVertexArray(unsigned int vaoId);     
RLAPI void rlDisableVertexArray(void);                  
RLAPI void rlEnableVertexBuffer(unsigned int id);       
RLAPI void rlDisableVertexBuffer(void);                 
RLAPI void rlEnableVertexBufferElement(unsigned int id);
RLAPI void rlDisableVertexBufferElement(void);          
RLAPI void rlEnableVertexAttribute(unsigned int index); 
RLAPI void rlDisableVertexAttribute(unsigned int index);
#if defined(GRAPHICS_API_OPENGL_11)
RLAPI void rlEnableStatePointer(int vertexAttribType, void *buffer);    
RLAPI void rlDisableStatePointer(int vertexAttribType);                 
#endif


RLAPI void rlActiveTextureSlot(int slot);               
RLAPI void rlEnableTexture(unsigned int id);            
RLAPI void rlDisableTexture(void);                      
RLAPI void rlEnableTextureCubemap(unsigned int id);     
RLAPI void rlDisableTextureCubemap(void);               
RLAPI void rlTextureParameters(unsigned int id, int param, int value); 
RLAPI void rlCubemapParameters(unsigned int id, int param, int value); 


RLAPI void rlEnableShader(unsigned int id);             
RLAPI void rlDisableShader(void);                       


RLAPI void rlEnableFramebuffer(unsigned int id);        
RLAPI void rlDisableFramebuffer(void);                  
RLAPI void rlActiveDrawBuffers(int count);              
RLAPI void rlBlitFramebuffer(int srcX, int srcY, int srcWidth, int srcHeight, int dstX, int dstY, int dstWidth, int dstHeight, int bufferMask); 


RLAPI void rlEnableColorBlend(void);                     
RLAPI void rlDisableColorBlend(void);                   
RLAPI void rlEnableDepthTest(void);                     
RLAPI void rlDisableDepthTest(void);                    
RLAPI void rlEnableDepthMask(void);                     
RLAPI void rlDisableDepthMask(void);                    
RLAPI void rlEnableBackfaceCulling(void);               
RLAPI void rlDisableBackfaceCulling(void);              
RLAPI void rlSetCullFace(int mode);                     
RLAPI void rlEnableScissorTest(void);                   
RLAPI void rlDisableScissorTest(void);                  
RLAPI void rlScissor(int x, int y, int width, int height); 
RLAPI void rlEnableWireMode(void);                      
RLAPI void rlEnablePointMode(void);                     
RLAPI void rlDisableWireMode(void);                     
RLAPI void rlSetLineWidth(float width);                 
RLAPI float rlGetLineWidth(void);                       
RLAPI void rlEnableSmoothLines(void);                   
RLAPI void rlDisableSmoothLines(void);                  
RLAPI void rlEnableStereoRender(void);                  
RLAPI void rlDisableStereoRender(void);                 
RLAPI bool rlIsStereoRenderEnabled(void);               

RLAPI void rlClearColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a); 
RLAPI void rlClearScreenBuffers(void);                  
RLAPI void rlCheckErrors(void);                         
RLAPI void rlSetBlendMode(int mode);                    
RLAPI void rlSetBlendFactors(int glSrcFactor, int glDstFactor, int glEquation); 
RLAPI void rlSetBlendFactorsSeparate(int glSrcRGB, int glDstRGB, int glSrcAlpha, int glDstAlpha, int glEqRGB, int glEqAlpha); 





RLAPI void rlglInit(int width, int height);             
RLAPI void rlglClose(void);                             
RLAPI void rlLoadExtensions(void *loader);              
RLAPI int rlGetVersion(void);                           
RLAPI void rlSetFramebufferWidth(int width);            
RLAPI int rlGetFramebufferWidth(void);                  
RLAPI void rlSetFramebufferHeight(int height);          
RLAPI int rlGetFramebufferHeight(void);                 

RLAPI unsigned int rlGetTextureIdDefault(void);         
RLAPI unsigned int rlGetShaderIdDefault(void);          
RLAPI int *rlGetShaderLocsDefault(void);                




RLAPI rlRenderBatch rlLoadRenderBatch(int numBuffers, int bufferElements);  
RLAPI void rlUnloadRenderBatch(rlRenderBatch batch);                        
RLAPI void rlDrawRenderBatch(rlRenderBatch *batch);                         
RLAPI void rlSetRenderBatchActive(rlRenderBatch *batch);                    
RLAPI void rlDrawRenderBatchActive(void);                                   
RLAPI bool rlCheckRenderBatchLimit(int vCount);                             

RLAPI void rlSetTexture(unsigned int id);               




RLAPI unsigned int rlLoadVertexArray(void);                               
RLAPI unsigned int rlLoadVertexBuffer(const void *buffer, int size, bool dynamic);            
RLAPI unsigned int rlLoadVertexBufferElement(const void *buffer, int size, bool dynamic);     
RLAPI void rlUpdateVertexBuffer(unsigned int bufferId, const void *data, int dataSize, int offset);     
RLAPI void rlUpdateVertexBufferElements(unsigned int id, const void *data, int dataSize, int offset);   
RLAPI void rlUnloadVertexArray(unsigned int vaoId);
RLAPI void rlUnloadVertexBuffer(unsigned int vboId);
RLAPI void rlSetVertexAttribute(unsigned int index, int compSize, int type, bool normalized, int stride, const void *pointer);
RLAPI void rlSetVertexAttributeDivisor(unsigned int index, int divisor);
RLAPI void rlSetVertexAttributeDefault(int locIndex, const void *value, int attribType, int count); 
RLAPI void rlDrawVertexArray(int offset, int count);
RLAPI void rlDrawVertexArrayElements(int offset, int count, const void *buffer);
RLAPI void rlDrawVertexArrayInstanced(int offset, int count, int instances);
RLAPI void rlDrawVertexArrayElementsInstanced(int offset, int count, const void *buffer, int instances);


RLAPI unsigned int rlLoadTexture(const void *data, int width, int height, int format, int mipmapCount); 
RLAPI unsigned int rlLoadTextureDepth(int width, int height, bool useRenderBuffer);               
RLAPI unsigned int rlLoadTextureCubemap(const void *data, int size, int format);                        
RLAPI void rlUpdateTexture(unsigned int id, int offsetX, int offsetY, int width, int height, int format, const void *data);  
RLAPI void rlGetGlTextureFormats(int format, unsigned int *glInternalFormat, unsigned int *glFormat, unsigned int *glType);  
RLAPI const char *rlGetPixelFormatName(unsigned int format);              
RLAPI void rlUnloadTexture(unsigned int id);                              
RLAPI void rlGenTextureMipmaps(unsigned int id, int width, int height, int format, int *mipmaps); 
RLAPI void *rlReadTexturePixels(unsigned int id, int width, int height, int format);              
RLAPI unsigned char *rlReadScreenPixels(int width, int height);           


RLAPI unsigned int rlLoadFramebuffer(int width, int height);              
RLAPI void rlFramebufferAttach(unsigned int fboId, unsigned int texId, int attachType, int texType, int mipLevel);  
RLAPI bool rlFramebufferComplete(unsigned int id);                        
RLAPI void rlUnloadFramebuffer(unsigned int id);                          


RLAPI unsigned int rlLoadShaderCode(const char *vsCode, const char *fsCode);    
RLAPI unsigned int rlCompileShader(const char *shaderCode, int type);           
RLAPI unsigned int rlLoadShaderProgram(unsigned int vShaderId, unsigned int fShaderId); 
RLAPI void rlUnloadShaderProgram(unsigned int id);                              
RLAPI int rlGetLocationUniform(unsigned int shaderId, const char *uniformName); 
RLAPI int rlGetLocationAttrib(unsigned int shaderId, const char *attribName);   
RLAPI void rlSetUniform(int locIndex, const void *value, int uniformType, int count);   
RLAPI void rlSetUniformMatrix(int locIndex, Matrix mat);                        
RLAPI void rlSetUniformSampler(int locIndex, unsigned int textureId);           
RLAPI void rlSetShader(unsigned int id, int *locs);                             


RLAPI unsigned int rlLoadComputeShaderProgram(unsigned int shaderId);           
RLAPI void rlComputeShaderDispatch(unsigned int groupX, unsigned int groupY, unsigned int groupZ);  


RLAPI unsigned int rlLoadShaderBuffer(unsigned int size, const void *data, int usageHint); 
RLAPI void rlUnloadShaderBuffer(unsigned int ssboId);                           
RLAPI void rlUpdateShaderBuffer(unsigned int id, const void *data, unsigned int dataSize, unsigned int offset); 
RLAPI void rlBindShaderBuffer(unsigned int id, unsigned int index);             
RLAPI void rlReadShaderBuffer(unsigned int id, void *dest, unsigned int count, unsigned int offset); 
RLAPI void rlCopyShaderBuffer(unsigned int destId, unsigned int srcId, unsigned int destOffset, unsigned int srcOffset, unsigned int count); 
RLAPI unsigned int rlGetShaderBufferSize(unsigned int id);                      


RLAPI void rlBindImageTexture(unsigned int id, unsigned int index, int format, bool readonly);  


RLAPI Matrix rlGetMatrixModelview(void);                                  
RLAPI Matrix rlGetMatrixProjection(void);                                 
RLAPI Matrix rlGetMatrixTransform(void);                                  
RLAPI Matrix rlGetMatrixProjectionStereo(int eye);                        
RLAPI Matrix rlGetMatrixViewOffsetStereo(int eye);                        
RLAPI void rlSetMatrixProjection(Matrix proj);                            
RLAPI void rlSetMatrixModelview(Matrix view);                             
RLAPI void rlSetMatrixProjectionStereo(Matrix right, Matrix left);        
RLAPI void rlSetMatrixViewOffsetStereo(Matrix right, Matrix left);        


RLAPI void rlLoadDrawCube(void);     
RLAPI void rlLoadDrawQuad(void);     

#if defined(__cplusplus)
}
#endif

#endif 

/***********************************************************************************
*
*   RLGL IMPLEMENTATION
*
************************************************************************************/

#if defined(RLGL_IMPLEMENTATION)

#if defined(GRAPHICS_API_OPENGL_11)
    #if defined(__APPLE__)
        #include <OpenGL/gl.h>          
        #include <OpenGL/glext.h>       
    #else
        
        #if !defined(APIENTRY)
            #if defined(_WIN32)
                #define APIENTRY __stdcall
            #else
                #define APIENTRY
            #endif
        #endif
        
        #if !defined(WINGDIAPI) && defined(_WIN32)
            #define WINGDIAPI __declspec(dllimport)
        #endif

        #include <GL/gl.h>              
    #endif
#endif

#if defined(GRAPHICS_API_OPENGL_33)
    #define GLAD_MALLOC RL_MALLOC
    #define GLAD_FREE RL_FREE

    #define GLAD_GL_IMPLEMENTATION
    #include "external/glad.h"          
#endif

#if defined(GRAPHICS_API_OPENGL_ES3)
    #include <GLES3/gl3.h>              
    #define GL_GLEXT_PROTOTYPES
    #include <GLES2/gl2ext.h>           
#elif defined(GRAPHICS_API_OPENGL_ES2)
    
    
    #if defined(PLATFORM_DESKTOP) || defined(PLATFORM_DESKTOP_SDL)
        #define GLAD_GLES2_IMPLEMENTATION
        #include "external/glad_gles2.h"
    #else
        #define GL_GLEXT_PROTOTYPES
        
        #include <GLES2/gl2.h>          
        #include <GLES2/gl2ext.h>       
    #endif

    
    
    #if defined(PLATFORM_DRM)
    typedef void (GL_APIENTRYP PFNGLDRAWARRAYSINSTANCEDEXTPROC) (GLenum mode, GLint start, GLsizei count, GLsizei primcount);
    typedef void (GL_APIENTRYP PFNGLDRAWELEMENTSINSTANCEDEXTPROC) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount);
    typedef void (GL_APIENTRYP PFNGLVERTEXATTRIBDIVISOREXTPROC) (GLuint index, GLuint divisor);
    #endif
#endif

#include <stdlib.h>                     
#include <string.h>                     
#include <math.h>                       




#ifndef PI
    #define PI 3.14159265358979323846f
#endif
#ifndef DEG2RAD
    #define DEG2RAD (PI/180.0f)
#endif
#ifndef RAD2DEG
    #define RAD2DEG (180.0f/PI)
#endif

#ifndef GL_SHADING_LANGUAGE_VERSION
    #define GL_SHADING_LANGUAGE_VERSION         0x8B8C
#endif

#ifndef GL_COMPRESSED_RGB_S3TC_DXT1_EXT
    #define GL_COMPRESSED_RGB_S3TC_DXT1_EXT     0x83F0
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
    #define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT    0x83F1
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
    #define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT    0x83F2
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
    #define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT    0x83F3
#endif
#ifndef GL_ETC1_RGB8_OES
    #define GL_ETC1_RGB8_OES                    0x8D64
#endif
#ifndef GL_COMPRESSED_RGB8_ETC2
    #define GL_COMPRESSED_RGB8_ETC2             0x9274
#endif
#ifndef GL_COMPRESSED_RGBA8_ETC2_EAC
    #define GL_COMPRESSED_RGBA8_ETC2_EAC        0x9278
#endif
#ifndef GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG
    #define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG  0x8C00
#endif
#ifndef GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG
    #define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG 0x8C02
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_4x4_KHR
    #define GL_COMPRESSED_RGBA_ASTC_4x4_KHR     0x93b0
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_8x8_KHR
    #define GL_COMPRESSED_RGBA_ASTC_8x8_KHR     0x93b7
#endif

#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
    #define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT   0x84FF
#endif
#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
    #define GL_TEXTURE_MAX_ANISOTROPY_EXT       0x84FE
#endif

#if defined(GRAPHICS_API_OPENGL_11)
    #define GL_UNSIGNED_SHORT_5_6_5             0x8363
    #define GL_UNSIGNED_SHORT_5_5_5_1           0x8034
    #define GL_UNSIGNED_SHORT_4_4_4_4           0x8033
#endif

#if defined(GRAPHICS_API_OPENGL_21)
    #define GL_LUMINANCE                        0x1909
    #define GL_LUMINANCE_ALPHA                  0x190A
#endif

#if defined(GRAPHICS_API_OPENGL_ES2)
    #define glClearDepth                 glClearDepthf
    #if !defined(GRAPHICS_API_OPENGL_ES3)
        #define GL_READ_FRAMEBUFFER         GL_FRAMEBUFFER
        #define GL_DRAW_FRAMEBUFFER         GL_FRAMEBUFFER
    #endif
#endif


#ifndef RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION
    #define RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION     "vertexPosition"    
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD
    #define RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD     "vertexTexCoord"    
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL
    #define RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL       "vertexNormal"      
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR
    #define RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR        "vertexColor"       
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT
    #define RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT      "vertexTangent"     
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2
    #define RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2    "vertexTexCoord2"   
#endif

#ifndef RL_DEFAULT_SHADER_UNIFORM_NAME_MVP
    #define RL_DEFAULT_SHADER_UNIFORM_NAME_MVP         "mvp"               
#endif
#ifndef RL_DEFAULT_SHADER_UNIFORM_NAME_VIEW
    #define RL_DEFAULT_SHADER_UNIFORM_NAME_VIEW        "matView"           
#endif
#ifndef RL_DEFAULT_SHADER_UNIFORM_NAME_PROJECTION
    #define RL_DEFAULT_SHADER_UNIFORM_NAME_PROJECTION  "matProjection"     
#endif
#ifndef RL_DEFAULT_SHADER_UNIFORM_NAME_MODEL
    #define RL_DEFAULT_SHADER_UNIFORM_NAME_MODEL       "matModel"          
#endif
#ifndef RL_DEFAULT_SHADER_UNIFORM_NAME_NORMAL
    #define RL_DEFAULT_SHADER_UNIFORM_NAME_NORMAL      "matNormal"         
#endif
#ifndef RL_DEFAULT_SHADER_UNIFORM_NAME_COLOR
    #define RL_DEFAULT_SHADER_UNIFORM_NAME_COLOR       "colDiffuse"        
#endif
#ifndef RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE0
    #define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE0  "texture0"          
#endif
#ifndef RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE1
    #define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE1  "texture1"          
#endif
#ifndef RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE2
    #define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE2  "texture2"          
#endif




#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
typedef struct rlglData {
    rlRenderBatch *currentBatch;            
    rlRenderBatch defaultBatch;             

    struct {
        int vertexCounter;                  
        float texcoordx, texcoordy;         
        float normalx, normaly, normalz;    
        unsigned char colorr, colorg, colorb, colora;   

        int currentMatrixMode;              
        Matrix *currentMatrix;              
        Matrix modelview;                   
        Matrix projection;                  
        Matrix transform;                   
        bool transformRequired;             
        Matrix stack[RL_MAX_MATRIX_STACK_SIZE];
        int stackCounter;                   

        unsigned int defaultTextureId;      
        unsigned int activeTextureId[RL_DEFAULT_BATCH_MAX_TEXTURE_UNITS];    
        unsigned int defaultVShaderId;      
        unsigned int defaultFShaderId;      
        unsigned int defaultShaderId;       
        int *defaultShaderLocs;             
        unsigned int currentShaderId;       
        int *currentShaderLocs;             

        bool stereoRender;                  
        Matrix projectionStereo[2];         
        Matrix viewOffsetStereo[2];         

        
        int currentBlendMode;               
        int glBlendSrcFactor;               
        int glBlendDstFactor;               
        int glBlendEquation;                
        int glBlendSrcFactorRGB;            
        int glBlendDestFactorRGB;           
        int glBlendSrcFactorAlpha;          
        int glBlendDestFactorAlpha;         
        int glBlendEquationRGB;             
        int glBlendEquationAlpha;           
        bool glCustomBlendModeModified;     

        int framebufferWidth;               
        int framebufferHeight;              

    } State;            
    struct {
        bool vao;                           
        bool instancing;                    
        bool texNPOT;                       
        bool texDepth;                      
        bool texDepthWebGL;                 
        bool texFloat32;                    
        bool texFloat16;                    
        bool texCompDXT;                    
        bool texCompETC1;                   
        bool texCompETC2;                   
        bool texCompPVRT;                   
        bool texCompASTC;                   
        bool texMirrorClamp;                
        bool texAnisoFilter;                
        bool computeShader;                 
        bool ssbo;                          

        float maxAnisotropyLevel;           
        int maxDepthBits;                   

    } ExtSupported;     
} rlglData;

typedef void *(*rlglLoadProc)(const char *name);   

#endif  




#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
static rlglData RLGL = { 0 };
#endif  

#if defined(GRAPHICS_API_OPENGL_ES2) && !defined(GRAPHICS_API_OPENGL_ES3)

static PFNGLGENVERTEXARRAYSOESPROC glGenVertexArrays = NULL;
static PFNGLBINDVERTEXARRAYOESPROC glBindVertexArray = NULL;
static PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArrays = NULL;


static PFNGLDRAWARRAYSINSTANCEDEXTPROC glDrawArraysInstanced = NULL;
static PFNGLDRAWELEMENTSINSTANCEDEXTPROC glDrawElementsInstanced = NULL;
static PFNGLVERTEXATTRIBDIVISOREXTPROC glVertexAttribDivisor = NULL;
#endif




#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
static void rlLoadShaderDefault(void);      
static void rlUnloadShaderDefault(void);    
#if defined(RLGL_SHOW_GL_DETAILS_INFO)
static const char *rlGetCompressedFormatName(int format); 
#endif  
#endif  

static int rlGetPixelDataSize(int width, int height, int format);   


static Matrix rlMatrixIdentity(void);                       
static Matrix rlMatrixMultiply(Matrix left, Matrix right);  





#if defined(GRAPHICS_API_OPENGL_11)


void rlMatrixMode(int mode)
{
    switch (mode)
    {
        case RL_PROJECTION: glMatrixMode(GL_PROJECTION); break;
        case RL_MODELVIEW: glMatrixMode(GL_MODELVIEW); break;
        case RL_TEXTURE: glMatrixMode(GL_TEXTURE); break;
        default: break;
    }
}

void rlFrustum(double left, double right, double bottom, double top, double znear, double zfar)
{
    glFrustum(left, right, bottom, top, znear, zfar);
}

void rlOrtho(double left, double right, double bottom, double top, double znear, double zfar)
{
    glOrtho(left, right, bottom, top, znear, zfar);
}

void rlPushMatrix(void) { glPushMatrix(); }
void rlPopMatrix(void) { glPopMatrix(); }
void rlLoadIdentity(void) { glLoadIdentity(); }
void rlTranslatef(float x, float y, float z) { glTranslatef(x, y, z); }
void rlRotatef(float angle, float x, float y, float z) { glRotatef(angle, x, y, z); }
void rlScalef(float x, float y, float z) { glScalef(x, y, z); }
void rlMultMatrixf(const float *matf) { glMultMatrixf(matf); }
#endif
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)

void rlMatrixMode(int mode)
{
    if (mode == RL_PROJECTION) RLGL.State.currentMatrix = &RLGL.State.projection;
    else if (mode == RL_MODELVIEW) RLGL.State.currentMatrix = &RLGL.State.modelview;
    

    RLGL.State.currentMatrixMode = mode;
}


void rlPushMatrix(void)
{
    if (RLGL.State.stackCounter >= RL_MAX_MATRIX_STACK_SIZE) TRACELOG(RL_LOG_ERROR, "RLGL: Matrix stack overflow (RL_MAX_MATRIX_STACK_SIZE)");

    if (RLGL.State.currentMatrixMode == RL_MODELVIEW)
    {
        RLGL.State.transformRequired = true;
        RLGL.State.currentMatrix = &RLGL.State.transform;
    }

    RLGL.State.stack[RLGL.State.stackCounter] = *RLGL.State.currentMatrix;
    RLGL.State.stackCounter++;
}


void rlPopMatrix(void)
{
    if (RLGL.State.stackCounter > 0)
    {
        Matrix mat = RLGL.State.stack[RLGL.State.stackCounter - 1];
        *RLGL.State.currentMatrix = mat;
        RLGL.State.stackCounter--;
    }

    if ((RLGL.State.stackCounter == 0) && (RLGL.State.currentMatrixMode == RL_MODELVIEW))
    {
        RLGL.State.currentMatrix = &RLGL.State.modelview;
        RLGL.State.transformRequired = false;
    }
}


void rlLoadIdentity(void)
{
    *RLGL.State.currentMatrix = rlMatrixIdentity();
}


void rlTranslatef(float x, float y, float z)
{
    Matrix matTranslation = {
        1.0f, 0.0f, 0.0f, x,
        0.0f, 1.0f, 0.0f, y,
        0.0f, 0.0f, 1.0f, z,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    
    *RLGL.State.currentMatrix = rlMatrixMultiply(matTranslation, *RLGL.State.currentMatrix);
}



void rlRotatef(float angle, float x, float y, float z)
{
    Matrix matRotation = rlMatrixIdentity();

    
    float lengthSquared = x*x + y*y + z*z;
    if ((lengthSquared != 1.0f) && (lengthSquared != 0.0f))
    {
        float inverseLength = 1.0f/sqrtf(lengthSquared);
        x *= inverseLength;
        y *= inverseLength;
        z *= inverseLength;
    }

    
    float sinres = sinf(DEG2RAD*angle);
    float cosres = cosf(DEG2RAD*angle);
    float t = 1.0f - cosres;

    matRotation.m0 = x*x*t + cosres;
    matRotation.m1 = y*x*t + z*sinres;
    matRotation.m2 = z*x*t - y*sinres;
    matRotation.m3 = 0.0f;

    matRotation.m4 = x*y*t - z*sinres;
    matRotation.m5 = y*y*t + cosres;
    matRotation.m6 = z*y*t + x*sinres;
    matRotation.m7 = 0.0f;

    matRotation.m8 = x*z*t + y*sinres;
    matRotation.m9 = y*z*t - x*sinres;
    matRotation.m10 = z*z*t + cosres;
    matRotation.m11 = 0.0f;

    matRotation.m12 = 0.0f;
    matRotation.m13 = 0.0f;
    matRotation.m14 = 0.0f;
    matRotation.m15 = 1.0f;

    
    *RLGL.State.currentMatrix = rlMatrixMultiply(matRotation, *RLGL.State.currentMatrix);
}


void rlScalef(float x, float y, float z)
{
    Matrix matScale = {
        x, 0.0f, 0.0f, 0.0f,
        0.0f, y, 0.0f, 0.0f,
        0.0f, 0.0f, z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    
    *RLGL.State.currentMatrix = rlMatrixMultiply(matScale, *RLGL.State.currentMatrix);
}


void rlMultMatrixf(const float *matf)
{
    
    Matrix mat = { matf[0], matf[4], matf[8], matf[12],
                   matf[1], matf[5], matf[9], matf[13],
                   matf[2], matf[6], matf[10], matf[14],
                   matf[3], matf[7], matf[11], matf[15] };

    *RLGL.State.currentMatrix = rlMatrixMultiply(*RLGL.State.currentMatrix, mat);
}


void rlFrustum(double left, double right, double bottom, double top, double znear, double zfar)
{
    Matrix matFrustum = { 0 };

    float rl = (float)(right - left);
    float tb = (float)(top - bottom);
    float fn = (float)(zfar - znear);

    matFrustum.m0 = ((float) znear*2.0f)/rl;
    matFrustum.m1 = 0.0f;
    matFrustum.m2 = 0.0f;
    matFrustum.m3 = 0.0f;

    matFrustum.m4 = 0.0f;
    matFrustum.m5 = ((float) znear*2.0f)/tb;
    matFrustum.m6 = 0.0f;
    matFrustum.m7 = 0.0f;

    matFrustum.m8 = ((float)right + (float)left)/rl;
    matFrustum.m9 = ((float)top + (float)bottom)/tb;
    matFrustum.m10 = -((float)zfar + (float)znear)/fn;
    matFrustum.m11 = -1.0f;

    matFrustum.m12 = 0.0f;
    matFrustum.m13 = 0.0f;
    matFrustum.m14 = -((float)zfar*(float)znear*2.0f)/fn;
    matFrustum.m15 = 0.0f;

    *RLGL.State.currentMatrix = rlMatrixMultiply(*RLGL.State.currentMatrix, matFrustum);
}


void rlOrtho(double left, double right, double bottom, double top, double znear, double zfar)
{
    
    
    Matrix matOrtho = { 0 };

    float rl = (float)(right - left);
    float tb = (float)(top - bottom);
    float fn = (float)(zfar - znear);

    matOrtho.m0 = 2.0f/rl;
    matOrtho.m1 = 0.0f;
    matOrtho.m2 = 0.0f;
    matOrtho.m3 = 0.0f;
    matOrtho.m4 = 0.0f;
    matOrtho.m5 = 2.0f/tb;
    matOrtho.m6 = 0.0f;
    matOrtho.m7 = 0.0f;
    matOrtho.m8 = 0.0f;
    matOrtho.m9 = 0.0f;
    matOrtho.m10 = -2.0f/fn;
    matOrtho.m11 = 0.0f;
    matOrtho.m12 = -((float)left + (float)right)/rl;
    matOrtho.m13 = -((float)top + (float)bottom)/tb;
    matOrtho.m14 = -((float)zfar + (float)znear)/fn;
    matOrtho.m15 = 1.0f;

    *RLGL.State.currentMatrix = rlMatrixMultiply(*RLGL.State.currentMatrix, matOrtho);
}
#endif



void rlViewport(int x, int y, int width, int height)
{
    glViewport(x, y, width, height);
}




#if defined(GRAPHICS_API_OPENGL_11)


void rlBegin(int mode)
{
    switch (mode)
    {
        case RL_LINES: glBegin(GL_LINES); break;
        case RL_TRIANGLES: glBegin(GL_TRIANGLES); break;
        case RL_QUADS: glBegin(GL_QUADS); break;
        default: break;
    }
}

void rlEnd() { glEnd(); }
void rlVertex2i(int x, int y) { glVertex2i(x, y); }
void rlVertex2f(float x, float y) { glVertex2f(x, y); }
void rlVertex3f(float x, float y, float z) { glVertex3f(x, y, z); }
void rlTexCoord2f(float x, float y) { glTexCoord2f(x, y); }
void rlNormal3f(float x, float y, float z) { glNormal3f(x, y, z); }
void rlColor4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a) { glColor4ub(r, g, b, a); }
void rlColor3f(float x, float y, float z) { glColor3f(x, y, z); }
void rlColor4f(float x, float y, float z, float w) { glColor4f(x, y, z, w); }
#endif
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)

void rlBegin(int mode)
{
    
    
    if (RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].mode != mode)
    {
        if (RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].vertexCount > 0)
        {
            
            
            
            
            
            if (RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].mode == RL_LINES) RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].vertexAlignment = ((RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].vertexCount < 4)? RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].vertexCount : RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].vertexCount%4);
            else if (RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].mode == RL_TRIANGLES) RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].vertexAlignment = ((RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].vertexCount < 4)? 1 : (4 - (RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].vertexCount%4)));
            else RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].vertexAlignment = 0;

            if (!rlCheckRenderBatchLimit(RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].vertexAlignment))
            {
                RLGL.State.vertexCounter += RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].vertexAlignment;
                RLGL.currentBatch->drawCounter++;
            }
        }

        if (RLGL.currentBatch->drawCounter >= RL_DEFAULT_BATCH_DRAWCALLS) rlDrawRenderBatch(RLGL.currentBatch);

        RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].mode = mode;
        RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].vertexCount = 0;
        RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].textureId = RLGL.State.defaultTextureId;
    }
}


void rlEnd(void)
{
    
    
    
    RLGL.currentBatch->currentDepth += (1.0f/20000.0f);
}



void rlVertex3f(float x, float y, float z)
{
    float tx = x;
    float ty = y;
    float tz = z;

    
    if (RLGL.State.transformRequired)
    {
        tx = RLGL.State.transform.m0*x + RLGL.State.transform.m4*y + RLGL.State.transform.m8*z + RLGL.State.transform.m12;
        ty = RLGL.State.transform.m1*x + RLGL.State.transform.m5*y + RLGL.State.transform.m9*z + RLGL.State.transform.m13;
        tz = RLGL.State.transform.m2*x + RLGL.State.transform.m6*y + RLGL.State.transform.m10*z + RLGL.State.transform.m14;
    }

    
    
    
    if (RLGL.State.vertexCounter > (RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].elementCount*4 - 4))
    {
        if ((RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].mode == RL_LINES) &&
            (RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].vertexCount%2 == 0))
        {
            
            
            
            rlCheckRenderBatchLimit(2 + 1);
        }
        else if ((RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].mode == RL_TRIANGLES) &&
            (RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].vertexCount%3 == 0))
        {
            rlCheckRenderBatchLimit(3 + 1);
        }
        else if ((RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].mode == RL_QUADS) &&
            (RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].vertexCount%4 == 0))
        {
            rlCheckRenderBatchLimit(4 + 1);
        }
    }

    
    RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vertices[3*RLGL.State.vertexCounter] = tx;
    RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vertices[3*RLGL.State.vertexCounter + 1] = ty;
    RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vertices[3*RLGL.State.vertexCounter + 2] = tz;

    
    RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].texcoords[2*RLGL.State.vertexCounter] = RLGL.State.texcoordx;
    RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].texcoords[2*RLGL.State.vertexCounter + 1] = RLGL.State.texcoordy;

    

    
    RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].colors[4*RLGL.State.vertexCounter] = RLGL.State.colorr;
    RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].colors[4*RLGL.State.vertexCounter + 1] = RLGL.State.colorg;
    RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].colors[4*RLGL.State.vertexCounter + 2] = RLGL.State.colorb;
    RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].colors[4*RLGL.State.vertexCounter + 3] = RLGL.State.colora;

    RLGL.State.vertexCounter++;
    RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].vertexCount++;
}


void rlVertex2f(float x, float y)
{
    rlVertex3f(x, y, RLGL.currentBatch->currentDepth);
}


void rlVertex2i(int x, int y)
{
    rlVertex3f((float)x, (float)y, RLGL.currentBatch->currentDepth);
}



void rlTexCoord2f(float x, float y)
{
    RLGL.State.texcoordx = x;
    RLGL.State.texcoordy = y;
}



void rlNormal3f(float x, float y, float z)
{
    RLGL.State.normalx = x;
    RLGL.State.normaly = y;
    RLGL.State.normalz = z;
}


void rlColor4ub(unsigned char x, unsigned char y, unsigned char z, unsigned char w)
{
    RLGL.State.colorr = x;
    RLGL.State.colorg = y;
    RLGL.State.colorb = z;
    RLGL.State.colora = w;
}


void rlColor4f(float r, float g, float b, float a)
{
    rlColor4ub((unsigned char)(r*255), (unsigned char)(g*255), (unsigned char)(b*255), (unsigned char)(a*255));
}


void rlColor3f(float x, float y, float z)
{
    rlColor4ub((unsigned char)(x*255), (unsigned char)(y*255), (unsigned char)(z*255), 255);
}

#endif






void rlSetTexture(unsigned int id)
{
    if (id == 0)
    {
#if defined(GRAPHICS_API_OPENGL_11)
        rlDisableTexture();
#else
        
        if (RLGL.State.vertexCounter >=
            RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].elementCount*4)
        {
            rlDrawRenderBatch(RLGL.currentBatch);
        }
#endif
    }
    else
    {
#if defined(GRAPHICS_API_OPENGL_11)
        rlEnableTexture(id);
#else
        if (RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].textureId != id)
        {
            if (RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].vertexCount > 0)
            {
                
                
                
                
                
                if (RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].mode == RL_LINES) RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].vertexAlignment = ((RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].vertexCount < 4)? RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].vertexCount : RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].vertexCount%4);
                else if (RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].mode == RL_TRIANGLES) RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].vertexAlignment = ((RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].vertexCount < 4)? 1 : (4 - (RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].vertexCount%4)));
                else RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].vertexAlignment = 0;

                if (!rlCheckRenderBatchLimit(RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].vertexAlignment))
                {
                    RLGL.State.vertexCounter += RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].vertexAlignment;

                    RLGL.currentBatch->drawCounter++;
                }
            }

            if (RLGL.currentBatch->drawCounter >= RL_DEFAULT_BATCH_DRAWCALLS) rlDrawRenderBatch(RLGL.currentBatch);

            RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].textureId = id;
            RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].vertexCount = 0;
        }
#endif
    }
}


void rlActiveTextureSlot(int slot)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glActiveTexture(GL_TEXTURE0 + slot);
#endif
}


void rlEnableTexture(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_11)
    glEnable(GL_TEXTURE_2D);
#endif
    glBindTexture(GL_TEXTURE_2D, id);
}


void rlDisableTexture(void)
{
#if defined(GRAPHICS_API_OPENGL_11)
    glDisable(GL_TEXTURE_2D);
#endif
    glBindTexture(GL_TEXTURE_2D, 0);
}


void rlEnableTextureCubemap(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
#endif
}


void rlDisableTextureCubemap(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
#endif
}


void rlTextureParameters(unsigned int id, int param, int value)
{
    glBindTexture(GL_TEXTURE_2D, id);

#if !defined(GRAPHICS_API_OPENGL_11)
    
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
#endif

    switch (param)
    {
        case RL_TEXTURE_WRAP_S:
        case RL_TEXTURE_WRAP_T:
        {
            if (value == RL_TEXTURE_WRAP_MIRROR_CLAMP)
            {
#if !defined(GRAPHICS_API_OPENGL_11)
                if (RLGL.ExtSupported.texMirrorClamp) glTexParameteri(GL_TEXTURE_2D, param, value);
                else TRACELOG(RL_LOG_WARNING, "GL: Clamp mirror wrap mode not supported (GL_MIRROR_CLAMP_EXT)");
#endif
            }
            else glTexParameteri(GL_TEXTURE_2D, param, value);

        } break;
        case RL_TEXTURE_MAG_FILTER:
        case RL_TEXTURE_MIN_FILTER: glTexParameteri(GL_TEXTURE_2D, param, value); break;
        case RL_TEXTURE_FILTER_ANISOTROPIC:
        {
#if !defined(GRAPHICS_API_OPENGL_11)
            if (value <= RLGL.ExtSupported.maxAnisotropyLevel) glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)value);
            else if (RLGL.ExtSupported.maxAnisotropyLevel > 0.0f)
            {
                TRACELOG(RL_LOG_WARNING, "GL: Maximum anisotropic filter level supported is %iX", id, (int)RLGL.ExtSupported.maxAnisotropyLevel);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)value);
            }
            else TRACELOG(RL_LOG_WARNING, "GL: Anisotropic filtering not supported");
#endif
        } break;
#if defined(GRAPHICS_API_OPENGL_33)
        case RL_TEXTURE_MIPMAP_BIAS_RATIO: glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, value/100.0f);
#endif
        default: break;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}


void rlCubemapParameters(unsigned int id, int param, int value)
{
#if !defined(GRAPHICS_API_OPENGL_11)
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

    
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);

    switch (param)
    {
        case RL_TEXTURE_WRAP_S:
        case RL_TEXTURE_WRAP_T:
        {
            if (value == RL_TEXTURE_WRAP_MIRROR_CLAMP)
            {
                if (RLGL.ExtSupported.texMirrorClamp) glTexParameteri(GL_TEXTURE_CUBE_MAP, param, value);
                else TRACELOG(RL_LOG_WARNING, "GL: Clamp mirror wrap mode not supported (GL_MIRROR_CLAMP_EXT)");
            }
            else glTexParameteri(GL_TEXTURE_CUBE_MAP, param, value);

        } break;
        case RL_TEXTURE_MAG_FILTER:
        case RL_TEXTURE_MIN_FILTER: glTexParameteri(GL_TEXTURE_CUBE_MAP, param, value); break;
        case RL_TEXTURE_FILTER_ANISOTROPIC:
        {
            if (value <= RLGL.ExtSupported.maxAnisotropyLevel) glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)value);
            else if (RLGL.ExtSupported.maxAnisotropyLevel > 0.0f)
            {
                TRACELOG(RL_LOG_WARNING, "GL: Maximum anisotropic filter level supported is %iX", id, (int)RLGL.ExtSupported.maxAnisotropyLevel);
                glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)value);
            }
            else TRACELOG(RL_LOG_WARNING, "GL: Anisotropic filtering not supported");
        } break;
#if defined(GRAPHICS_API_OPENGL_33)
        case RL_TEXTURE_MIPMAP_BIAS_RATIO: glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_LOD_BIAS, value/100.0f);
#endif
        default: break;
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
#endif
}


void rlEnableShader(unsigned int id)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2))
    glUseProgram(id);
#endif
}


void rlDisableShader(void)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2))
    glUseProgram(0);
#endif
}


void rlEnableFramebuffer(unsigned int id)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)) && defined(RLGL_RENDER_TEXTURES_HINT)
    glBindFramebuffer(GL_FRAMEBUFFER, id);
#endif
}


void rlDisableFramebuffer(void)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)) && defined(RLGL_RENDER_TEXTURES_HINT)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
}


void rlBlitFramebuffer(int srcX, int srcY, int srcWidth, int srcHeight, int dstX, int dstY, int dstWidth, int dstHeight, int bufferMask)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES3)) && defined(RLGL_RENDER_TEXTURES_HINT)
    glBlitFramebuffer(srcX, srcY, srcWidth, srcHeight, dstX, dstY, dstWidth, dstHeight, bufferMask, GL_NEAREST);
#endif
}



void rlActiveDrawBuffers(int count)
{
#if ((defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES3)) && defined(RLGL_RENDER_TEXTURES_HINT))
    
    
    
    

    if (count > 0)
    {
        if (count > 8) TRACELOG(LOG_WARNING, "GL: Max color buffers limited to 8");
        else
        {
            unsigned int buffers[8] = {
#if defined(GRAPHICS_API_OPENGL_ES3)
                GL_COLOR_ATTACHMENT0_EXT,
                GL_COLOR_ATTACHMENT1_EXT,
                GL_COLOR_ATTACHMENT2_EXT,
                GL_COLOR_ATTACHMENT3_EXT,
                GL_COLOR_ATTACHMENT4_EXT,
                GL_COLOR_ATTACHMENT5_EXT,
                GL_COLOR_ATTACHMENT6_EXT,
                GL_COLOR_ATTACHMENT7_EXT,
#else
                GL_COLOR_ATTACHMENT0,
                GL_COLOR_ATTACHMENT1,
                GL_COLOR_ATTACHMENT2,
                GL_COLOR_ATTACHMENT3,
                GL_COLOR_ATTACHMENT4,
                GL_COLOR_ATTACHMENT5,
                GL_COLOR_ATTACHMENT6,
                GL_COLOR_ATTACHMENT7,
#endif
            };

#if defined(GRAPHICS_API_OPENGL_ES3)
            glDrawBuffersEXT(count, buffers);
#else
            glDrawBuffers(count, buffers);
#endif
        }
    }
    else TRACELOG(LOG_WARNING, "GL: One color buffer active by default");
#endif
}






void rlEnableColorBlend(void) { glEnable(GL_BLEND); }


void rlDisableColorBlend(void) { glDisable(GL_BLEND); }


void rlEnableDepthTest(void) { glEnable(GL_DEPTH_TEST); }


void rlDisableDepthTest(void) { glDisable(GL_DEPTH_TEST); }


void rlEnableDepthMask(void) { glDepthMask(GL_TRUE); }


void rlDisableDepthMask(void) { glDepthMask(GL_FALSE); }


void rlEnableBackfaceCulling(void) { glEnable(GL_CULL_FACE); }


void rlDisableBackfaceCulling(void) { glDisable(GL_CULL_FACE); }


void rlSetCullFace(int mode)
{
    switch (mode)
    {
        case RL_CULL_FACE_BACK: glCullFace(GL_BACK); break;
        case RL_CULL_FACE_FRONT: glCullFace(GL_FRONT); break;
        default: break;
    }
}


void rlEnableScissorTest(void) { glEnable(GL_SCISSOR_TEST); }


void rlDisableScissorTest(void) { glDisable(GL_SCISSOR_TEST); }


void rlScissor(int x, int y, int width, int height) { glScissor(x, y, width, height); }


void rlEnableWireMode(void)
{
#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif
}

void rlEnablePointMode(void)
{
#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    glEnable(GL_PROGRAM_POINT_SIZE);
#endif
}

void rlDisableWireMode(void)
{
#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
}


void rlSetLineWidth(float width) { glLineWidth(width); }


float rlGetLineWidth(void)
{
    float width = 0;
    glGetFloatv(GL_LINE_WIDTH, &width);
    return width;
}


void rlEnableSmoothLines(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_11)
    glEnable(GL_LINE_SMOOTH);
#endif
}


void rlDisableSmoothLines(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_11)
    glDisable(GL_LINE_SMOOTH);
#endif
}


void rlEnableStereoRender(void)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2))
    RLGL.State.stereoRender = true;
#endif
}


void rlDisableStereoRender(void)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2))
    RLGL.State.stereoRender = false;
#endif
}


bool rlIsStereoRenderEnabled(void)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2))
    return RLGL.State.stereoRender;
#else
    return false;
#endif
}


void rlClearColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    
    float cr = (float)r/255;
    float cg = (float)g/255;
    float cb = (float)b/255;
    float ca = (float)a/255;

    glClearColor(cr, cg, cb, ca);
}


void rlClearScreenBuffers(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     
    
}


void rlCheckErrors()
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    int check = 1;
    while (check)
    {
        const GLenum err = glGetError();
        switch (err)
        {
            case GL_NO_ERROR: check = 0; break;
            case 0x0500: TRACELOG(RL_LOG_WARNING, "GL: Error detected: GL_INVALID_ENUM"); break;
            case 0x0501: TRACELOG(RL_LOG_WARNING, "GL: Error detected: GL_INVALID_VALUE"); break;
            case 0x0502: TRACELOG(RL_LOG_WARNING, "GL: Error detected: GL_INVALID_OPERATION"); break;
            case 0x0503: TRACELOG(RL_LOG_WARNING, "GL: Error detected: GL_STACK_OVERFLOW"); break;
            case 0x0504: TRACELOG(RL_LOG_WARNING, "GL: Error detected: GL_STACK_UNDERFLOW"); break;
            case 0x0505: TRACELOG(RL_LOG_WARNING, "GL: Error detected: GL_OUT_OF_MEMORY"); break;
            case 0x0506: TRACELOG(RL_LOG_WARNING, "GL: Error detected: GL_INVALID_FRAMEBUFFER_OPERATION"); break;
            default: TRACELOG(RL_LOG_WARNING, "GL: Error detected: Unknown error code: %x", err); break;
        }
    }
#endif
}


void rlSetBlendMode(int mode)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if ((RLGL.State.currentBlendMode != mode) || ((mode == RL_BLEND_CUSTOM || mode == RL_BLEND_CUSTOM_SEPARATE) && RLGL.State.glCustomBlendModeModified))
    {
        rlDrawRenderBatch(RLGL.currentBatch);

        switch (mode)
        {
            case RL_BLEND_ALPHA: glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glBlendEquation(GL_FUNC_ADD); break;
            case RL_BLEND_ADDITIVE: glBlendFunc(GL_SRC_ALPHA, GL_ONE); glBlendEquation(GL_FUNC_ADD); break;
            case RL_BLEND_MULTIPLIED: glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA); glBlendEquation(GL_FUNC_ADD); break;
            case RL_BLEND_ADD_COLORS: glBlendFunc(GL_ONE, GL_ONE); glBlendEquation(GL_FUNC_ADD); break;
            case RL_BLEND_SUBTRACT_COLORS: glBlendFunc(GL_ONE, GL_ONE); glBlendEquation(GL_FUNC_SUBTRACT); break;
            case RL_BLEND_ALPHA_PREMULTIPLY: glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); glBlendEquation(GL_FUNC_ADD); break;
            case RL_BLEND_CUSTOM:
            {
                
                glBlendFunc(RLGL.State.glBlendSrcFactor, RLGL.State.glBlendDstFactor); glBlendEquation(RLGL.State.glBlendEquation);

            } break;
            case RL_BLEND_CUSTOM_SEPARATE:
            {
                
                glBlendFuncSeparate(RLGL.State.glBlendSrcFactorRGB, RLGL.State.glBlendDestFactorRGB, RLGL.State.glBlendSrcFactorAlpha, RLGL.State.glBlendDestFactorAlpha);
                glBlendEquationSeparate(RLGL.State.glBlendEquationRGB, RLGL.State.glBlendEquationAlpha);

            } break;
            default: break;
        }

        RLGL.State.currentBlendMode = mode;
        RLGL.State.glCustomBlendModeModified = false;
    }
#endif
}


void rlSetBlendFactors(int glSrcFactor, int glDstFactor, int glEquation)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if ((RLGL.State.glBlendSrcFactor != glSrcFactor) ||
        (RLGL.State.glBlendDstFactor != glDstFactor) ||
        (RLGL.State.glBlendEquation != glEquation))
    {
        RLGL.State.glBlendSrcFactor = glSrcFactor;
        RLGL.State.glBlendDstFactor = glDstFactor;
        RLGL.State.glBlendEquation = glEquation;

        RLGL.State.glCustomBlendModeModified = true;
    }
#endif
}


void rlSetBlendFactorsSeparate(int glSrcRGB, int glDstRGB, int glSrcAlpha, int glDstAlpha, int glEqRGB, int glEqAlpha)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if ((RLGL.State.glBlendSrcFactorRGB != glSrcRGB) ||
        (RLGL.State.glBlendDestFactorRGB != glDstRGB) ||
        (RLGL.State.glBlendSrcFactorAlpha != glSrcAlpha) ||
        (RLGL.State.glBlendDestFactorAlpha != glDstAlpha) ||
        (RLGL.State.glBlendEquationRGB != glEqRGB) ||
        (RLGL.State.glBlendEquationAlpha != glEqAlpha))
    {
        RLGL.State.glBlendSrcFactorRGB = glSrcRGB;
        RLGL.State.glBlendDestFactorRGB = glDstRGB;
        RLGL.State.glBlendSrcFactorAlpha = glSrcAlpha;
        RLGL.State.glBlendDestFactorAlpha = glDstAlpha;
        RLGL.State.glBlendEquationRGB = glEqRGB;
        RLGL.State.glBlendEquationAlpha = glEqAlpha;

        RLGL.State.glCustomBlendModeModified = true;
    }
#endif
}




#if defined(RLGL_ENABLE_OPENGL_DEBUG_CONTEXT) && defined(GRAPHICS_API_OPENGL_43)
static void GLAPIENTRY rlDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
    
    
    
    
    
    
    
    
    if ((id == 131169) || (id == 131185) || (id == 131218) || (id == 131204)) return;

    const char *msgSource = NULL;
    switch (source)
    {
        case GL_DEBUG_SOURCE_API: msgSource = "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: msgSource = "WINDOW_SYSTEM"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: msgSource = "SHADER_COMPILER"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY: msgSource = "THIRD_PARTY"; break;
        case GL_DEBUG_SOURCE_APPLICATION: msgSource = "APPLICATION"; break;
        case GL_DEBUG_SOURCE_OTHER: msgSource = "OTHER"; break;
        default: break;
    }

    const char *msgType = NULL;
    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR: msgType = "ERROR"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: msgType = "DEPRECATED_BEHAVIOR"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: msgType = "UNDEFINED_BEHAVIOR"; break;
        case GL_DEBUG_TYPE_PORTABILITY: msgType = "PORTABILITY"; break;
        case GL_DEBUG_TYPE_PERFORMANCE: msgType = "PERFORMANCE"; break;
        case GL_DEBUG_TYPE_MARKER: msgType = "MARKER"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP: msgType = "PUSH_GROUP"; break;
        case GL_DEBUG_TYPE_POP_GROUP: msgType = "POP_GROUP"; break;
        case GL_DEBUG_TYPE_OTHER: msgType = "OTHER"; break;
        default: break;
    }

    const char *msgSeverity = "DEFAULT";
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_LOW: msgSeverity = "LOW"; break;
        case GL_DEBUG_SEVERITY_MEDIUM: msgSeverity = "MEDIUM"; break;
        case GL_DEBUG_SEVERITY_HIGH: msgSeverity = "HIGH"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: msgSeverity = "NOTIFICATION"; break;
        default: break;
    }

    TRACELOG(LOG_WARNING, "GL: OpenGL debug message: %s", message);
    TRACELOG(LOG_WARNING, "    > Type: %s", msgType);
    TRACELOG(LOG_WARNING, "    > Source = %s", msgSource);
    TRACELOG(LOG_WARNING, "    > Severity = %s", msgSeverity);
}
#endif






void rlglInit(int width, int height)
{
    
#if defined(RLGL_ENABLE_OPENGL_DEBUG_CONTEXT) && defined(GRAPHICS_API_OPENGL_43)
    if ((glDebugMessageCallback != NULL) && (glDebugMessageControl != NULL))
    {
        glDebugMessageCallback(rlDebugMessageCallback, 0);
        

        
        
        
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    }
#endif

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    
    unsigned char pixels[4] = { 255, 255, 255, 255 };   
    RLGL.State.defaultTextureId = rlLoadTexture(pixels, 1, 1, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);

    if (RLGL.State.defaultTextureId != 0) TRACELOG(RL_LOG_INFO, "TEXTURE: [ID %i] Default texture loaded successfully", RLGL.State.defaultTextureId);
    else TRACELOG(RL_LOG_WARNING, "TEXTURE: Failed to load default texture");

    
    
    rlLoadShaderDefault();
    RLGL.State.currentShaderId = RLGL.State.defaultShaderId;
    RLGL.State.currentShaderLocs = RLGL.State.defaultShaderLocs;

    
    RLGL.defaultBatch = rlLoadRenderBatch(RL_DEFAULT_BATCH_BUFFERS, RL_DEFAULT_BATCH_BUFFER_ELEMENTS);
    RLGL.currentBatch = &RLGL.defaultBatch;

    
    for (int i = 0; i < RL_MAX_MATRIX_STACK_SIZE; i++) RLGL.State.stack[i] = rlMatrixIdentity();

    
    RLGL.State.transform = rlMatrixIdentity();
    RLGL.State.projection = rlMatrixIdentity();
    RLGL.State.modelview = rlMatrixIdentity();
    RLGL.State.currentMatrix = &RLGL.State.modelview;
#endif  

    
    
    
    glDepthFunc(GL_LEQUAL);                                 
    glDisable(GL_DEPTH_TEST);                               

    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);      
    glEnable(GL_BLEND);                                     

    
    
    glCullFace(GL_BACK);                                    
    glFrontFace(GL_CCW);                                    
    glEnable(GL_CULL_FACE);                                 

    
#if defined(GRAPHICS_API_OPENGL_33)
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);                 
#endif

#if defined(GRAPHICS_API_OPENGL_11)
    
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);      
    glShadeModel(GL_SMOOTH);                                
#endif

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    
    RLGL.State.framebufferWidth = width;
    RLGL.State.framebufferHeight = height;

    TRACELOG(RL_LOG_INFO, "RLGL: Default OpenGL state initialized successfully");
    
#endif

    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);                   
    glClearDepth(1.0f);                                     
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     
}


void rlglClose(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    rlUnloadRenderBatch(RLGL.defaultBatch);

    rlUnloadShaderDefault();          

    glDeleteTextures(1, &RLGL.State.defaultTextureId); 
    TRACELOG(RL_LOG_INFO, "TEXTURE: [ID %i] Default texture unloaded successfully", RLGL.State.defaultTextureId);
#endif
}



void rlLoadExtensions(void *loader)
{
#if defined(GRAPHICS_API_OPENGL_33)     
    
    if (gladLoadGL((GLADloadfunc)loader) == 0) TRACELOG(RL_LOG_WARNING, "GLAD: Cannot load OpenGL extensions");
    else TRACELOG(RL_LOG_INFO, "GLAD: OpenGL extensions loaded successfully");

    
    GLint numExt = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);
    TRACELOG(RL_LOG_INFO, "GL: Supported extensions count: %i", numExt);

#if defined(RLGL_SHOW_GL_DETAILS_INFO)
    
    
    TRACELOG(RL_LOG_INFO, "GL: OpenGL extensions:");
    for (int i = 0; i < numExt; i++) TRACELOG(RL_LOG_INFO, "    %s", glGetStringi(GL_EXTENSIONS, i));
#endif

#if defined(GRAPHICS_API_OPENGL_21)
    
    
    RLGL.ExtSupported.vao = GLAD_GL_ARB_vertex_array_object;
    RLGL.ExtSupported.instancing = (GLAD_GL_EXT_draw_instanced && GLAD_GL_ARB_instanced_arrays);
    RLGL.ExtSupported.texNPOT = GLAD_GL_ARB_texture_non_power_of_two;
    RLGL.ExtSupported.texFloat32 = GLAD_GL_ARB_texture_float;
    RLGL.ExtSupported.texFloat16 = GLAD_GL_ARB_texture_float;
    RLGL.ExtSupported.texDepth = GLAD_GL_ARB_depth_texture;
    RLGL.ExtSupported.maxDepthBits = 32;
    RLGL.ExtSupported.texAnisoFilter = GLAD_GL_EXT_texture_filter_anisotropic;
    RLGL.ExtSupported.texMirrorClamp = GLAD_GL_EXT_texture_mirror_clamp;
#else
    
    
    RLGL.ExtSupported.vao = true;
    RLGL.ExtSupported.instancing = true;
    RLGL.ExtSupported.texNPOT = true;
    RLGL.ExtSupported.texFloat32 = true;
    RLGL.ExtSupported.texFloat16 = true;
    RLGL.ExtSupported.texDepth = true;
    RLGL.ExtSupported.maxDepthBits = 32;
    RLGL.ExtSupported.texAnisoFilter = true;
    RLGL.ExtSupported.texMirrorClamp = true;
#endif

    
    RLGL.ExtSupported.texCompASTC = GLAD_GL_KHR_texture_compression_astc_hdr && GLAD_GL_KHR_texture_compression_astc_ldr;
    RLGL.ExtSupported.texCompDXT = GLAD_GL_EXT_texture_compression_s3tc;  
    RLGL.ExtSupported.texCompETC2 = GLAD_GL_ARB_ES3_compatibility;        
    #if defined(GRAPHICS_API_OPENGL_43)
    RLGL.ExtSupported.computeShader = GLAD_GL_ARB_compute_shader;
    RLGL.ExtSupported.ssbo = GLAD_GL_ARB_shader_storage_buffer_object;
    #endif

#endif  

#if defined(GRAPHICS_API_OPENGL_ES3)
    
    
    RLGL.ExtSupported.vao = true;
    RLGL.ExtSupported.instancing = true;
    RLGL.ExtSupported.texNPOT = true;
    RLGL.ExtSupported.texFloat32 = true;
    RLGL.ExtSupported.texFloat16 = true;
    RLGL.ExtSupported.texDepth = true;
    RLGL.ExtSupported.texDepthWebGL = true;
    RLGL.ExtSupported.maxDepthBits = 24;
    RLGL.ExtSupported.texAnisoFilter = true;
    RLGL.ExtSupported.texMirrorClamp = true;
    
    
    
    
    
    
    
    
    

#elif defined(GRAPHICS_API_OPENGL_ES2)

    #if defined(PLATFORM_DESKTOP) || defined(PLATFORM_DESKTOP_SDL)
    
    if (gladLoadGLES2((GLADloadfunc)loader) == 0) TRACELOG(RL_LOG_WARNING, "GLAD: Cannot load OpenGL ES2.0 functions");
    else TRACELOG(RL_LOG_INFO, "GLAD: OpenGL ES 2.0 loaded successfully");
    #endif

    
    GLint numExt = 0;
    const char **extList = RL_MALLOC(512*sizeof(const char *)); 
    const char *extensions = (const char *)glGetString(GL_EXTENSIONS);  

    
    int size = strlen(extensions) + 1;      
    char *extensionsDup = (char *)RL_CALLOC(size, sizeof(char));
    strcpy(extensionsDup, extensions);
    extList[numExt] = extensionsDup;

    for (int i = 0; i < size; i++)
    {
        if (extensionsDup[i] == ' ')
        {
            extensionsDup[i] = '\0';
            numExt++;
            extList[numExt] = &extensionsDup[i + 1];
        }
    }

    TRACELOG(RL_LOG_INFO, "GL: Supported extensions count: %i", numExt);

#if defined(RLGL_SHOW_GL_DETAILS_INFO)
    TRACELOG(RL_LOG_INFO, "GL: OpenGL extensions:");
    for (int i = 0; i < numExt; i++) TRACELOG(RL_LOG_INFO, "    %s", extList[i]);
#endif

    
    for (int i = 0; i < numExt; i++)
    {
        
        
        if (strcmp(extList[i], (const char *)"GL_OES_vertex_array_object") == 0)
        {
            
            
            glGenVertexArrays = (PFNGLGENVERTEXARRAYSOESPROC)((rlglLoadProc)loader)("glGenVertexArraysOES");
            glBindVertexArray = (PFNGLBINDVERTEXARRAYOESPROC)((rlglLoadProc)loader)("glBindVertexArrayOES");
            glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSOESPROC)((rlglLoadProc)loader)("glDeleteVertexArraysOES");
            

            if ((glGenVertexArrays != NULL) && (glBindVertexArray != NULL) && (glDeleteVertexArrays != NULL)) RLGL.ExtSupported.vao = true;
        }

        
        if (strcmp(extList[i], (const char *)"GL_ANGLE_instanced_arrays") == 0)         
        {
            glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDEXTPROC)((rlglLoadProc)loader)("glDrawArraysInstancedANGLE");
            glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDEXTPROC)((rlglLoadProc)loader)("glDrawElementsInstancedANGLE");
            glVertexAttribDivisor = (PFNGLVERTEXATTRIBDIVISOREXTPROC)((rlglLoadProc)loader)("glVertexAttribDivisorANGLE");

            if ((glDrawArraysInstanced != NULL) && (glDrawElementsInstanced != NULL) && (glVertexAttribDivisor != NULL)) RLGL.ExtSupported.instancing = true;
        }
        else
        {
            if ((strcmp(extList[i], (const char *)"GL_EXT_draw_instanced") == 0) &&     
                (strcmp(extList[i], (const char *)"GL_EXT_instanced_arrays") == 0))
            {
                glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDEXTPROC)((rlglLoadProc)loader)("glDrawArraysInstancedEXT");
                glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDEXTPROC)((rlglLoadProc)loader)("glDrawElementsInstancedEXT");
                glVertexAttribDivisor = (PFNGLVERTEXATTRIBDIVISOREXTPROC)((rlglLoadProc)loader)("glVertexAttribDivisorEXT");

                if ((glDrawArraysInstanced != NULL) && (glDrawElementsInstanced != NULL) && (glVertexAttribDivisor != NULL)) RLGL.ExtSupported.instancing = true;
            }
        }

        
        
        if (strcmp(extList[i], (const char *)"GL_OES_texture_npot") == 0) RLGL.ExtSupported.texNPOT = true;

        
        if (strcmp(extList[i], (const char *)"GL_OES_texture_float") == 0) RLGL.ExtSupported.texFloat32 = true;
        if (strcmp(extList[i], (const char *)"GL_OES_texture_half_float") == 0) RLGL.ExtSupported.texFloat16 = true;

        
        if (strcmp(extList[i], (const char *)"GL_OES_depth_texture") == 0) RLGL.ExtSupported.texDepth = true;
        if (strcmp(extList[i], (const char *)"GL_WEBGL_depth_texture") == 0) RLGL.ExtSupported.texDepthWebGL = true;    
        if (RLGL.ExtSupported.texDepthWebGL) RLGL.ExtSupported.texDepth = true;

        if (strcmp(extList[i], (const char *)"GL_OES_depth24") == 0) RLGL.ExtSupported.maxDepthBits = 24;   
        if (strcmp(extList[i], (const char *)"GL_OES_depth32") == 0) RLGL.ExtSupported.maxDepthBits = 32;   

        
        if ((strcmp(extList[i], (const char *)"GL_EXT_texture_compression_s3tc") == 0) ||
            (strcmp(extList[i], (const char *)"GL_WEBGL_compressed_texture_s3tc") == 0) ||
            (strcmp(extList[i], (const char *)"GL_WEBKIT_WEBGL_compressed_texture_s3tc") == 0)) RLGL.ExtSupported.texCompDXT = true;

        
        if ((strcmp(extList[i], (const char *)"GL_OES_compressed_ETC1_RGB8_texture") == 0) ||
            (strcmp(extList[i], (const char *)"GL_WEBGL_compressed_texture_etc1") == 0)) RLGL.ExtSupported.texCompETC1 = true;

        
        if (strcmp(extList[i], (const char *)"GL_ARB_ES3_compatibility") == 0) RLGL.ExtSupported.texCompETC2 = true;

        
        if (strcmp(extList[i], (const char *)"GL_IMG_texture_compression_pvrtc") == 0) RLGL.ExtSupported.texCompPVRT = true;

        
        if (strcmp(extList[i], (const char *)"GL_KHR_texture_compression_astc_hdr") == 0) RLGL.ExtSupported.texCompASTC = true;

        
        if (strcmp(extList[i], (const char *)"GL_EXT_texture_filter_anisotropic") == 0) RLGL.ExtSupported.texAnisoFilter = true;

        
        if (strcmp(extList[i], (const char *)"GL_EXT_texture_mirror_clamp") == 0) RLGL.ExtSupported.texMirrorClamp = true;
    }

    
    RL_FREE(extList);
    RL_FREE(extensionsDup);    
#endif  

    
    
    
    TRACELOG(RL_LOG_INFO, "GL: OpenGL device information:");
    TRACELOG(RL_LOG_INFO, "    > Vendor:   %s", glGetString(GL_VENDOR));
    TRACELOG(RL_LOG_INFO, "    > Renderer: %s", glGetString(GL_RENDERER));
    TRACELOG(RL_LOG_INFO, "    > Version:  %s", glGetString(GL_VERSION));
    TRACELOG(RL_LOG_INFO, "    > GLSL:     %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    
    #ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
        #define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
    #endif
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &RLGL.ExtSupported.maxAnisotropyLevel);

#if defined(RLGL_SHOW_GL_DETAILS_INFO)
    
    TRACELOG(RL_LOG_INFO, "GL: OpenGL capabilities:");
    GLint capability = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &capability);
    TRACELOG(RL_LOG_INFO, "    GL_MAX_TEXTURE_SIZE: %i", capability);
    glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &capability);
    TRACELOG(RL_LOG_INFO, "    GL_MAX_CUBE_MAP_TEXTURE_SIZE: %i", capability);
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &capability);
    TRACELOG(RL_LOG_INFO, "    GL_MAX_TEXTURE_IMAGE_UNITS: %i", capability);
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &capability);
    TRACELOG(RL_LOG_INFO, "    GL_MAX_VERTEX_ATTRIBS: %i", capability);
    #if !defined(GRAPHICS_API_OPENGL_ES2)
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &capability);
    TRACELOG(RL_LOG_INFO, "    GL_MAX_UNIFORM_BLOCK_SIZE: %i", capability);
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &capability);
    TRACELOG(RL_LOG_INFO, "    GL_MAX_DRAW_BUFFERS: %i", capability);
    if (RLGL.ExtSupported.texAnisoFilter) TRACELOG(RL_LOG_INFO, "    GL_MAX_TEXTURE_MAX_ANISOTROPY: %.0f", RLGL.ExtSupported.maxAnisotropyLevel);
    #endif
    glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &capability);
    TRACELOG(RL_LOG_INFO, "    GL_NUM_COMPRESSED_TEXTURE_FORMATS: %i", capability);
    GLint *compFormats = (GLint *)RL_CALLOC(capability, sizeof(GLint));
    glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, compFormats);
    for (int i = 0; i < capability; i++) TRACELOG(RL_LOG_INFO, "        %s", rlGetCompressedFormatName(compFormats[i]));
    RL_FREE(compFormats);

#if defined(GRAPHICS_API_OPENGL_43)
    glGetIntegerv(GL_MAX_VERTEX_ATTRIB_BINDINGS, &capability);
    TRACELOG(RL_LOG_INFO, "    GL_MAX_VERTEX_ATTRIB_BINDINGS: %i", capability);
    glGetIntegerv(GL_MAX_UNIFORM_LOCATIONS, &capability);
    TRACELOG(RL_LOG_INFO, "    GL_MAX_UNIFORM_LOCATIONS: %i", capability);
#endif  
#else   

    
    if (RLGL.ExtSupported.vao) TRACELOG(RL_LOG_INFO, "GL: VAO extension detected, VAO functions loaded successfully");
    else TRACELOG(RL_LOG_WARNING, "GL: VAO extension not found, VAO not supported");
    if (RLGL.ExtSupported.texNPOT) TRACELOG(RL_LOG_INFO, "GL: NPOT textures extension detected, full NPOT textures supported");
    else TRACELOG(RL_LOG_WARNING, "GL: NPOT textures extension not found, limited NPOT support (no-mipmaps, no-repeat)");
    if (RLGL.ExtSupported.texCompDXT) TRACELOG(RL_LOG_INFO, "GL: DXT compressed textures supported");
    if (RLGL.ExtSupported.texCompETC1) TRACELOG(RL_LOG_INFO, "GL: ETC1 compressed textures supported");
    if (RLGL.ExtSupported.texCompETC2) TRACELOG(RL_LOG_INFO, "GL: ETC2/EAC compressed textures supported");
    if (RLGL.ExtSupported.texCompPVRT) TRACELOG(RL_LOG_INFO, "GL: PVRT compressed textures supported");
    if (RLGL.ExtSupported.texCompASTC) TRACELOG(RL_LOG_INFO, "GL: ASTC compressed textures supported");
    if (RLGL.ExtSupported.computeShader) TRACELOG(RL_LOG_INFO, "GL: Compute shaders supported");
    if (RLGL.ExtSupported.ssbo) TRACELOG(RL_LOG_INFO, "GL: Shader storage buffer objects supported");
#endif  

#endif  
}


int rlGetVersion(void)
{
    int glVersion = 0;
#if defined(GRAPHICS_API_OPENGL_11)
    glVersion = RL_OPENGL_11;
#endif
#if defined(GRAPHICS_API_OPENGL_21)
    glVersion = RL_OPENGL_21;
#elif defined(GRAPHICS_API_OPENGL_43)
    glVersion = RL_OPENGL_43;
#elif defined(GRAPHICS_API_OPENGL_33)
    glVersion = RL_OPENGL_33;
#endif
#if defined(GRAPHICS_API_OPENGL_ES3)
    glVersion = RL_OPENGL_ES_30;
#elif defined(GRAPHICS_API_OPENGL_ES2)
    glVersion = RL_OPENGL_ES_20;
#endif

    return glVersion;
}


void rlSetFramebufferWidth(int width)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    RLGL.State.framebufferWidth = width;
#endif
}


void rlSetFramebufferHeight(int height)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    RLGL.State.framebufferHeight = height;
#endif
}


int rlGetFramebufferWidth(void)
{
    int width = 0;
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    width = RLGL.State.framebufferWidth;
#endif
    return width;
}


int rlGetFramebufferHeight(void)
{
    int height = 0;
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    height = RLGL.State.framebufferHeight;
#endif
    return height;
}



unsigned int rlGetTextureIdDefault(void)
{
    unsigned int id = 0;
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    id = RLGL.State.defaultTextureId;
#endif
    return id;
}


unsigned int rlGetShaderIdDefault(void)
{
    unsigned int id = 0;
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    id = RLGL.State.defaultShaderId;
#endif
    return id;
}


int *rlGetShaderLocsDefault(void)
{
    int *locs = NULL;
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    locs = RLGL.State.defaultShaderLocs;
#endif
    return locs;
}




rlRenderBatch rlLoadRenderBatch(int numBuffers, int bufferElements)
{
    rlRenderBatch batch = { 0 };

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    
    
    batch.vertexBuffer = (rlVertexBuffer *)RL_MALLOC(numBuffers*sizeof(rlVertexBuffer));

    for (int i = 0; i < numBuffers; i++)
    {
        batch.vertexBuffer[i].elementCount = bufferElements;

        batch.vertexBuffer[i].vertices = (float *)RL_MALLOC(bufferElements*3*4*sizeof(float));        
        batch.vertexBuffer[i].texcoords = (float *)RL_MALLOC(bufferElements*2*4*sizeof(float));       
        batch.vertexBuffer[i].colors = (unsigned char *)RL_MALLOC(bufferElements*4*4*sizeof(unsigned char));   
#if defined(GRAPHICS_API_OPENGL_33)
        batch.vertexBuffer[i].indices = (unsigned int *)RL_MALLOC(bufferElements*6*sizeof(unsigned int));      
#endif
#if defined(GRAPHICS_API_OPENGL_ES2)
        batch.vertexBuffer[i].indices = (unsigned short *)RL_MALLOC(bufferElements*6*sizeof(unsigned short));  
#endif

        for (int j = 0; j < (3*4*bufferElements); j++) batch.vertexBuffer[i].vertices[j] = 0.0f;
        for (int j = 0; j < (2*4*bufferElements); j++) batch.vertexBuffer[i].texcoords[j] = 0.0f;
        for (int j = 0; j < (4*4*bufferElements); j++) batch.vertexBuffer[i].colors[j] = 0;

        int k = 0;

        
        for (int j = 0; j < (6*bufferElements); j += 6)
        {
            batch.vertexBuffer[i].indices[j] = 4*k;
            batch.vertexBuffer[i].indices[j + 1] = 4*k + 1;
            batch.vertexBuffer[i].indices[j + 2] = 4*k + 2;
            batch.vertexBuffer[i].indices[j + 3] = 4*k;
            batch.vertexBuffer[i].indices[j + 4] = 4*k + 2;
            batch.vertexBuffer[i].indices[j + 5] = 4*k + 3;

            k++;
        }

        RLGL.State.vertexCounter = 0;
    }

    TRACELOG(RL_LOG_INFO, "RLGL: Render batch vertex buffers loaded successfully in RAM (CPU)");
    

    
    
    for (int i = 0; i < numBuffers; i++)
    {
        if (RLGL.ExtSupported.vao)
        {
            
            glGenVertexArrays(1, &batch.vertexBuffer[i].vaoId);
            glBindVertexArray(batch.vertexBuffer[i].vaoId);
        }

        
        
        glGenBuffers(1, &batch.vertexBuffer[i].vboId[0]);
        glBindBuffer(GL_ARRAY_BUFFER, batch.vertexBuffer[i].vboId[0]);
        glBufferData(GL_ARRAY_BUFFER, bufferElements*3*4*sizeof(float), batch.vertexBuffer[i].vertices, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(RLGL.State.currentShaderLocs[RL_SHADER_LOC_VERTEX_POSITION]);
        glVertexAttribPointer(RLGL.State.currentShaderLocs[RL_SHADER_LOC_VERTEX_POSITION], 3, GL_FLOAT, 0, 0, 0);

        
        glGenBuffers(1, &batch.vertexBuffer[i].vboId[1]);
        glBindBuffer(GL_ARRAY_BUFFER, batch.vertexBuffer[i].vboId[1]);
        glBufferData(GL_ARRAY_BUFFER, bufferElements*2*4*sizeof(float), batch.vertexBuffer[i].texcoords, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(RLGL.State.currentShaderLocs[RL_SHADER_LOC_VERTEX_TEXCOORD01]);
        glVertexAttribPointer(RLGL.State.currentShaderLocs[RL_SHADER_LOC_VERTEX_TEXCOORD01], 2, GL_FLOAT, 0, 0, 0);

        
        glGenBuffers(1, &batch.vertexBuffer[i].vboId[2]);
        glBindBuffer(GL_ARRAY_BUFFER, batch.vertexBuffer[i].vboId[2]);
        glBufferData(GL_ARRAY_BUFFER, bufferElements*4*4*sizeof(unsigned char), batch.vertexBuffer[i].colors, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(RLGL.State.currentShaderLocs[RL_SHADER_LOC_VERTEX_COLOR]);
        glVertexAttribPointer(RLGL.State.currentShaderLocs[RL_SHADER_LOC_VERTEX_COLOR], 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);

        
        glGenBuffers(1, &batch.vertexBuffer[i].vboId[3]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch.vertexBuffer[i].vboId[3]);
#if defined(GRAPHICS_API_OPENGL_33)
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferElements*6*sizeof(int), batch.vertexBuffer[i].indices, GL_STATIC_DRAW);
#endif
#if defined(GRAPHICS_API_OPENGL_ES2)
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferElements*6*sizeof(short), batch.vertexBuffer[i].indices, GL_STATIC_DRAW);
#endif
    }

    TRACELOG(RL_LOG_INFO, "RLGL: Render batch vertex buffers loaded successfully in VRAM (GPU)");

    
    if (RLGL.ExtSupported.vao) glBindVertexArray(0);
    

    
    
    batch.draws = (rlDrawCall *)RL_MALLOC(RL_DEFAULT_BATCH_DRAWCALLS*sizeof(rlDrawCall));

    for (int i = 0; i < RL_DEFAULT_BATCH_DRAWCALLS; i++)
    {
        batch.draws[i].mode = RL_QUADS;
        batch.draws[i].vertexCount = 0;
        batch.draws[i].vertexAlignment = 0;
        
        
        batch.draws[i].textureId = RLGL.State.defaultTextureId;
        
        
    }

    batch.bufferCount = numBuffers;    
    batch.drawCounter = 1;             
    batch.currentDepth = -1.0f;         
    
#endif

    return batch;
}


void rlUnloadRenderBatch(rlRenderBatch batch)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    
    for (int i = 0; i < batch.bufferCount; i++)
    {
        
        if (RLGL.ExtSupported.vao)
        {
            glBindVertexArray(batch.vertexBuffer[i].vaoId);
            glDisableVertexAttribArray(0);
            glDisableVertexAttribArray(1);
            glDisableVertexAttribArray(2);
            glDisableVertexAttribArray(3);
            glBindVertexArray(0);
        }

        
        glDeleteBuffers(1, &batch.vertexBuffer[i].vboId[0]);
        glDeleteBuffers(1, &batch.vertexBuffer[i].vboId[1]);
        glDeleteBuffers(1, &batch.vertexBuffer[i].vboId[2]);
        glDeleteBuffers(1, &batch.vertexBuffer[i].vboId[3]);

        
        if (RLGL.ExtSupported.vao) glDeleteVertexArrays(1, &batch.vertexBuffer[i].vaoId);

        
        RL_FREE(batch.vertexBuffer[i].vertices);
        RL_FREE(batch.vertexBuffer[i].texcoords);
        RL_FREE(batch.vertexBuffer[i].colors);
        RL_FREE(batch.vertexBuffer[i].indices);
    }

    
    RL_FREE(batch.vertexBuffer);
    RL_FREE(batch.draws);
#endif
}



void rlDrawRenderBatch(rlRenderBatch *batch)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    
    
    
    
    if (RLGL.State.vertexCounter > 0)
    {
        
        if (RLGL.ExtSupported.vao) glBindVertexArray(batch->vertexBuffer[batch->currentBuffer].vaoId);

        
        glBindBuffer(GL_ARRAY_BUFFER, batch->vertexBuffer[batch->currentBuffer].vboId[0]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, RLGL.State.vertexCounter*3*sizeof(float), batch->vertexBuffer[batch->currentBuffer].vertices);
        

        
        glBindBuffer(GL_ARRAY_BUFFER, batch->vertexBuffer[batch->currentBuffer].vboId[1]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, RLGL.State.vertexCounter*2*sizeof(float), batch->vertexBuffer[batch->currentBuffer].texcoords);
        

        
        glBindBuffer(GL_ARRAY_BUFFER, batch->vertexBuffer[batch->currentBuffer].vboId[2]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, RLGL.State.vertexCounter*4*sizeof(unsigned char), batch->vertexBuffer[batch->currentBuffer].colors);
        

        
        
        
        
        

        
        
        
        
        
            
        
        

        
        if (RLGL.ExtSupported.vao) glBindVertexArray(0);
    }
    

    
    
    Matrix matProjection = RLGL.State.projection;
    Matrix matModelView = RLGL.State.modelview;

    int eyeCount = 1;
    if (RLGL.State.stereoRender) eyeCount = 2;

    for (int eye = 0; eye < eyeCount; eye++)
    {
        if (eyeCount == 2)
        {
            
            rlViewport(eye*RLGL.State.framebufferWidth/2, 0, RLGL.State.framebufferWidth/2, RLGL.State.framebufferHeight);

            
            rlSetMatrixModelview(rlMatrixMultiply(matModelView, RLGL.State.viewOffsetStereo[eye]));
            
            rlSetMatrixProjection(RLGL.State.projectionStereo[eye]);
        }

        
        if (RLGL.State.vertexCounter > 0)
        {
            
            glUseProgram(RLGL.State.currentShaderId);

            
            Matrix matMVP = rlMatrixMultiply(RLGL.State.modelview, RLGL.State.projection);
            float matMVPfloat[16] = {
                matMVP.m0, matMVP.m1, matMVP.m2, matMVP.m3,
                matMVP.m4, matMVP.m5, matMVP.m6, matMVP.m7,
                matMVP.m8, matMVP.m9, matMVP.m10, matMVP.m11,
                matMVP.m12, matMVP.m13, matMVP.m14, matMVP.m15
            };
            glUniformMatrix4fv(RLGL.State.currentShaderLocs[RL_SHADER_LOC_MATRIX_MVP], 1, false, matMVPfloat);

            if (RLGL.ExtSupported.vao) glBindVertexArray(batch->vertexBuffer[batch->currentBuffer].vaoId);
            else
            {
                
                glBindBuffer(GL_ARRAY_BUFFER, batch->vertexBuffer[batch->currentBuffer].vboId[0]);
                glVertexAttribPointer(RLGL.State.currentShaderLocs[RL_SHADER_LOC_VERTEX_POSITION], 3, GL_FLOAT, 0, 0, 0);
                glEnableVertexAttribArray(RLGL.State.currentShaderLocs[RL_SHADER_LOC_VERTEX_POSITION]);

                
                glBindBuffer(GL_ARRAY_BUFFER, batch->vertexBuffer[batch->currentBuffer].vboId[1]);
                glVertexAttribPointer(RLGL.State.currentShaderLocs[RL_SHADER_LOC_VERTEX_TEXCOORD01], 2, GL_FLOAT, 0, 0, 0);
                glEnableVertexAttribArray(RLGL.State.currentShaderLocs[RL_SHADER_LOC_VERTEX_TEXCOORD01]);

                
                glBindBuffer(GL_ARRAY_BUFFER, batch->vertexBuffer[batch->currentBuffer].vboId[2]);
                glVertexAttribPointer(RLGL.State.currentShaderLocs[RL_SHADER_LOC_VERTEX_COLOR], 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
                glEnableVertexAttribArray(RLGL.State.currentShaderLocs[RL_SHADER_LOC_VERTEX_COLOR]);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch->vertexBuffer[batch->currentBuffer].vboId[3]);
            }

            
            glUniform4f(RLGL.State.currentShaderLocs[RL_SHADER_LOC_COLOR_DIFFUSE], 1.0f, 1.0f, 1.0f, 1.0f);
            glUniform1i(RLGL.State.currentShaderLocs[RL_SHADER_LOC_MAP_DIFFUSE], 0);  

            
            
            for (int i = 0; i < RL_DEFAULT_BATCH_MAX_TEXTURE_UNITS; i++)
            {
                if (RLGL.State.activeTextureId[i] > 0)
                {
                    glActiveTexture(GL_TEXTURE0 + 1 + i);
                    glBindTexture(GL_TEXTURE_2D, RLGL.State.activeTextureId[i]);
                }
            }

            
            
            glActiveTexture(GL_TEXTURE0);

            for (int i = 0, vertexOffset = 0; i < batch->drawCounter; i++)
            {
                
                glBindTexture(GL_TEXTURE_2D, batch->draws[i].textureId);

                if ((batch->draws[i].mode == RL_LINES) || (batch->draws[i].mode == RL_TRIANGLES)) glDrawArrays(batch->draws[i].mode, vertexOffset, batch->draws[i].vertexCount);
                else
                {
#if defined(GRAPHICS_API_OPENGL_33)
                    
                    
                    
                    glDrawElements(GL_TRIANGLES, batch->draws[i].vertexCount/4*6, GL_UNSIGNED_INT, (GLvoid *)(vertexOffset/4*6*sizeof(GLuint)));
#endif
#if defined(GRAPHICS_API_OPENGL_ES2)
                    glDrawElements(GL_TRIANGLES, batch->draws[i].vertexCount/4*6, GL_UNSIGNED_SHORT, (GLvoid *)(vertexOffset/4*6*sizeof(GLushort)));
#endif
                }

                vertexOffset += (batch->draws[i].vertexCount + batch->draws[i].vertexAlignment);
            }

            if (!RLGL.ExtSupported.vao)
            {
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            }

            glBindTexture(GL_TEXTURE_2D, 0);    
        }

        if (RLGL.ExtSupported.vao) glBindVertexArray(0); 

        glUseProgram(0);    
    }

    
    if (eyeCount == 2) rlViewport(0, 0, RLGL.State.framebufferWidth, RLGL.State.framebufferHeight);
    

    
    
    
    RLGL.State.vertexCounter = 0;

    
    batch->currentDepth = -1.0f;

    
    RLGL.State.projection = matProjection;
    RLGL.State.modelview = matModelView;

    
    for (int i = 0; i < RL_DEFAULT_BATCH_DRAWCALLS; i++)
    {
        batch->draws[i].mode = RL_QUADS;
        batch->draws[i].vertexCount = 0;
        batch->draws[i].textureId = RLGL.State.defaultTextureId;
    }

    
    for (int i = 0; i < RL_DEFAULT_BATCH_MAX_TEXTURE_UNITS; i++) RLGL.State.activeTextureId[i] = 0;

    
    batch->drawCounter = 1;
    

    
    batch->currentBuffer++;
    if (batch->currentBuffer >= batch->bufferCount) batch->currentBuffer = 0;
#endif
}


void rlSetRenderBatchActive(rlRenderBatch *batch)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    rlDrawRenderBatch(RLGL.currentBatch);

    if (batch != NULL) RLGL.currentBatch = batch;
    else RLGL.currentBatch = &RLGL.defaultBatch;
#endif
}


void rlDrawRenderBatchActive(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    rlDrawRenderBatch(RLGL.currentBatch);    
#endif
}



bool rlCheckRenderBatchLimit(int vCount)
{
    bool overflow = false;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if ((RLGL.State.vertexCounter + vCount) >=
        (RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].elementCount*4))
    {
        overflow = true;

        
        int currentMode = RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].mode;
        int currentTexture = RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].textureId;

        rlDrawRenderBatch(RLGL.currentBatch);    

        
        RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].mode = currentMode;
        RLGL.currentBatch->draws[RLGL.currentBatch->drawCounter - 1].textureId = currentTexture;
    }
#endif

    return overflow;
}




unsigned int rlLoadTexture(const void *data, int width, int height, int format, int mipmapCount)
{
    unsigned int id = 0;

    glBindTexture(GL_TEXTURE_2D, 0);    

    
#if defined(GRAPHICS_API_OPENGL_11)
    if (format >= RL_PIXELFORMAT_COMPRESSED_DXT1_RGB)
    {
        TRACELOG(RL_LOG_WARNING, "GL: OpenGL 1.1 does not support GPU compressed texture formats");
        return id;
    }
#else
    if ((!RLGL.ExtSupported.texCompDXT) && ((format == RL_PIXELFORMAT_COMPRESSED_DXT1_RGB) || (format == RL_PIXELFORMAT_COMPRESSED_DXT1_RGBA) ||
        (format == RL_PIXELFORMAT_COMPRESSED_DXT3_RGBA) || (format == RL_PIXELFORMAT_COMPRESSED_DXT5_RGBA)))
    {
        TRACELOG(RL_LOG_WARNING, "GL: DXT compressed texture format not supported");
        return id;
    }
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if ((!RLGL.ExtSupported.texCompETC1) && (format == RL_PIXELFORMAT_COMPRESSED_ETC1_RGB))
    {
        TRACELOG(RL_LOG_WARNING, "GL: ETC1 compressed texture format not supported");
        return id;
    }

    if ((!RLGL.ExtSupported.texCompETC2) && ((format == RL_PIXELFORMAT_COMPRESSED_ETC2_RGB) || (format == RL_PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA)))
    {
        TRACELOG(RL_LOG_WARNING, "GL: ETC2 compressed texture format not supported");
        return id;
    }

    if ((!RLGL.ExtSupported.texCompPVRT) && ((format == RL_PIXELFORMAT_COMPRESSED_PVRT_RGB) || (format == RL_PIXELFORMAT_COMPRESSED_PVRT_RGBA)))
    {
        TRACELOG(RL_LOG_WARNING, "GL: PVRT compressed texture format not supported");
        return id;
    }

    if ((!RLGL.ExtSupported.texCompASTC) && ((format == RL_PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA) || (format == RL_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA)))
    {
        TRACELOG(RL_LOG_WARNING, "GL: ASTC compressed texture format not supported");
        return id;
    }
#endif
#endif  

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(1, &id);              

    glBindTexture(GL_TEXTURE_2D, id);

    int mipWidth = width;
    int mipHeight = height;
    int mipOffset = 0;          

    
    unsigned char *dataPtr = NULL;
    if (data != NULL) dataPtr = (unsigned char *)data;

    
    for (int i = 0; i < mipmapCount; i++)
    {
        unsigned int mipSize = rlGetPixelDataSize(mipWidth, mipHeight, format);

        unsigned int glInternalFormat, glFormat, glType;
        rlGetGlTextureFormats(format, &glInternalFormat, &glFormat, &glType);

        TRACELOGD("TEXTURE: Load mipmap level %i (%i x %i), size: %i, offset: %i", i, mipWidth, mipHeight, mipSize, mipOffset);

        if (glInternalFormat != 0)
        {
            if (format < RL_PIXELFORMAT_COMPRESSED_DXT1_RGB) glTexImage2D(GL_TEXTURE_2D, i, glInternalFormat, mipWidth, mipHeight, 0, glFormat, glType, dataPtr);
#if !defined(GRAPHICS_API_OPENGL_11)
            else glCompressedTexImage2D(GL_TEXTURE_2D, i, glInternalFormat, mipWidth, mipHeight, 0, mipSize, dataPtr);
#endif

#if defined(GRAPHICS_API_OPENGL_33)
            if (format == RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE)
            {
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
                glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
            }
            else if (format == RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA)
            {
#if defined(GRAPHICS_API_OPENGL_21)
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ALPHA };
#elif defined(GRAPHICS_API_OPENGL_33)
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_GREEN };
#endif
                glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
            }
#endif
        }

        mipWidth /= 2;
        mipHeight /= 2;
        mipOffset += mipSize;       
        if (data != NULL) dataPtr += mipSize;         

        
        if (mipWidth < 1) mipWidth = 1;
        if (mipHeight < 1) mipHeight = 1;
    }

    
    
#if defined(GRAPHICS_API_OPENGL_ES2)
    
    if (RLGL.ExtSupported.texNPOT)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);       
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);       
    }
    else
    {
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);       
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);       
    }
#else
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);       
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);       
#endif

    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  

#if defined(GRAPHICS_API_OPENGL_33)
    if (mipmapCount > 1)
    {
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }
#endif

    

    

    
    glBindTexture(GL_TEXTURE_2D, 0);

    if (id > 0) TRACELOG(RL_LOG_INFO, "TEXTURE: [ID %i] Texture loaded successfully (%ix%i | %s | %i mipmaps)", id, width, height, rlGetPixelFormatName(format), mipmapCount);
    else TRACELOG(RL_LOG_WARNING, "TEXTURE: Failed to load texture");

    return id;
}



unsigned int rlLoadTextureDepth(int width, int height, bool useRenderBuffer)
{
    unsigned int id = 0;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    
    if (!RLGL.ExtSupported.texDepth) useRenderBuffer = true;

    
    
    unsigned int glInternalFormat = GL_DEPTH_COMPONENT;

#if (defined(GRAPHICS_API_OPENGL_ES2) || defined(GRAPHICS_API_OPENGL_ES3))
    
    
    if (!RLGL.ExtSupported.texDepthWebGL || useRenderBuffer)
    {
        if (RLGL.ExtSupported.maxDepthBits == 32) glInternalFormat = GL_DEPTH_COMPONENT32_OES;
        else if (RLGL.ExtSupported.maxDepthBits == 24) glInternalFormat = GL_DEPTH_COMPONENT24_OES;
        else glInternalFormat = GL_DEPTH_COMPONENT16;
    }
#endif

    if (!useRenderBuffer && RLGL.ExtSupported.texDepth)
    {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_2D, 0);

        TRACELOG(RL_LOG_INFO, "TEXTURE: Depth texture loaded successfully");
    }
    else
    {
        
        
        glGenRenderbuffers(1, &id);
        glBindRenderbuffer(GL_RENDERBUFFER, id);
        glRenderbufferStorage(GL_RENDERBUFFER, glInternalFormat, width, height);

        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        TRACELOG(RL_LOG_INFO, "TEXTURE: [ID %i] Depth renderbuffer loaded successfully (%i bits)", id, (RLGL.ExtSupported.maxDepthBits >= 24)? RLGL.ExtSupported.maxDepthBits : 16);
    }
#endif

    return id;
}




unsigned int rlLoadTextureCubemap(const void *data, int size, int format)
{
    unsigned int id = 0;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    unsigned int dataSize = rlGetPixelDataSize(size, size, format);

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

    unsigned int glInternalFormat, glFormat, glType;
    rlGetGlTextureFormats(format, &glInternalFormat, &glFormat, &glType);

    if (glInternalFormat != 0)
    {
        
        for (unsigned int i = 0; i < 6; i++)
        {
            if (data == NULL)
            {
                if (format < RL_PIXELFORMAT_COMPRESSED_DXT1_RGB)
                {
                    if ((format == RL_PIXELFORMAT_UNCOMPRESSED_R32) || (format == RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32)
                            || (format == RL_PIXELFORMAT_UNCOMPRESSED_R16) || (format == RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16))
                        TRACELOG(RL_LOG_WARNING, "TEXTURES: Cubemap requested format not supported");
                    else glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, glInternalFormat, size, size, 0, glFormat, glType, NULL);
                }
                else TRACELOG(RL_LOG_WARNING, "TEXTURES: Empty cubemap creation does not support compressed format");
            }
            else
            {
                if (format < RL_PIXELFORMAT_COMPRESSED_DXT1_RGB) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, glInternalFormat, size, size, 0, glFormat, glType, (unsigned char *)data + i*dataSize);
                else glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, glInternalFormat, size, size, 0, dataSize, (unsigned char *)data + i*dataSize);
            }

#if defined(GRAPHICS_API_OPENGL_33)
            if (format == RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE)
            {
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
                glTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
            }
            else if (format == RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA)
            {
#if defined(GRAPHICS_API_OPENGL_21)
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ALPHA };
#elif defined(GRAPHICS_API_OPENGL_33)
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_GREEN };
#endif
                glTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
            }
#endif
        }
    }

    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#if defined(GRAPHICS_API_OPENGL_33)
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);  
#endif

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
#endif

    if (id > 0) TRACELOG(RL_LOG_INFO, "TEXTURE: [ID %i] Cubemap texture loaded successfully (%ix%i)", id, size, size);
    else TRACELOG(RL_LOG_WARNING, "TEXTURE: Failed to load cubemap texture");

    return id;
}



void rlUpdateTexture(unsigned int id, int offsetX, int offsetY, int width, int height, int format, const void *data)
{
    glBindTexture(GL_TEXTURE_2D, id);

    unsigned int glInternalFormat, glFormat, glType;
    rlGetGlTextureFormats(format, &glInternalFormat, &glFormat, &glType);

    if ((glInternalFormat != 0) && (format < RL_PIXELFORMAT_COMPRESSED_DXT1_RGB))
    {
        glTexSubImage2D(GL_TEXTURE_2D, 0, offsetX, offsetY, width, height, glFormat, glType, data);
    }
    else TRACELOG(RL_LOG_WARNING, "TEXTURE: [ID %i] Failed to update for current texture format (%i)", id, format);
}


void rlGetGlTextureFormats(int format, unsigned int *glInternalFormat, unsigned int *glFormat, unsigned int *glType)
{
    *glInternalFormat = 0;
    *glFormat = 0;
    *glType = 0;

    switch (format)
    {
    #if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_21) || defined(GRAPHICS_API_OPENGL_ES2)
        
        case RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: *glInternalFormat = GL_LUMINANCE; *glFormat = GL_LUMINANCE; *glType = GL_UNSIGNED_BYTE; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA: *glInternalFormat = GL_LUMINANCE_ALPHA; *glFormat = GL_LUMINANCE_ALPHA; *glType = GL_UNSIGNED_BYTE; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R5G6B5: *glInternalFormat = GL_RGB; *glFormat = GL_RGB; *glType = GL_UNSIGNED_SHORT_5_6_5; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8: *glInternalFormat = GL_RGB; *glFormat = GL_RGB; *glType = GL_UNSIGNED_BYTE; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1: *glInternalFormat = GL_RGBA; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_SHORT_5_5_5_1; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: *glInternalFormat = GL_RGBA; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_SHORT_4_4_4_4; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: *glInternalFormat = GL_RGBA; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_BYTE; break;
        #if !defined(GRAPHICS_API_OPENGL_11)
        #if defined(GRAPHICS_API_OPENGL_ES3)
        case RL_PIXELFORMAT_UNCOMPRESSED_R32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_R32F_EXT; *glFormat = GL_RED_EXT; *glType = GL_FLOAT; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_RGB32F_EXT; *glFormat = GL_RGB; *glType = GL_FLOAT; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_RGBA32F_EXT; *glFormat = GL_RGBA; *glType = GL_FLOAT; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16: if (RLGL.ExtSupported.texFloat16) *glInternalFormat = GL_R16F_EXT; *glFormat = GL_RED_EXT; *glType = GL_HALF_FLOAT; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16: if (RLGL.ExtSupported.texFloat16) *glInternalFormat = GL_RGB16F_EXT; *glFormat = GL_RGB; *glType = GL_HALF_FLOAT; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16: if (RLGL.ExtSupported.texFloat16) *glInternalFormat = GL_RGBA16F_EXT; *glFormat = GL_RGBA; *glType = GL_HALF_FLOAT; break;
        #else
        case RL_PIXELFORMAT_UNCOMPRESSED_R32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_LUMINANCE; *glFormat = GL_LUMINANCE; *glType = GL_FLOAT; break;            
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_RGB; *glFormat = GL_RGB; *glType = GL_FLOAT; break;                  
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_RGBA; *glFormat = GL_RGBA; *glType = GL_FLOAT; break;             
        #if defined(GRAPHICS_API_OPENGL_21)
        case RL_PIXELFORMAT_UNCOMPRESSED_R16: if (RLGL.ExtSupported.texFloat16) *glInternalFormat = GL_LUMINANCE; *glFormat = GL_LUMINANCE; *glType = GL_HALF_FLOAT_ARB; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16: if (RLGL.ExtSupported.texFloat16) *glInternalFormat = GL_RGB; *glFormat = GL_RGB; *glType = GL_HALF_FLOAT_ARB; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16: if (RLGL.ExtSupported.texFloat16) *glInternalFormat = GL_RGBA; *glFormat = GL_RGBA; *glType = GL_HALF_FLOAT_ARB; break;
        #else 
        case RL_PIXELFORMAT_UNCOMPRESSED_R16: if (RLGL.ExtSupported.texFloat16) *glInternalFormat = GL_LUMINANCE; *glFormat = GL_LUMINANCE; *glType = GL_HALF_FLOAT_OES; break;   
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16: if (RLGL.ExtSupported.texFloat16) *glInternalFormat = GL_RGB; *glFormat = GL_RGB; *glType = GL_HALF_FLOAT_OES; break;         
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16: if (RLGL.ExtSupported.texFloat16) *glInternalFormat = GL_RGBA; *glFormat = GL_RGBA; *glType = GL_HALF_FLOAT_OES; break;    
        #endif
        #endif
        #endif
    #elif defined(GRAPHICS_API_OPENGL_33)
        case RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: *glInternalFormat = GL_R8; *glFormat = GL_RED; *glType = GL_UNSIGNED_BYTE; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA: *glInternalFormat = GL_RG8; *glFormat = GL_RG; *glType = GL_UNSIGNED_BYTE; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R5G6B5: *glInternalFormat = GL_RGB565; *glFormat = GL_RGB; *glType = GL_UNSIGNED_SHORT_5_6_5; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8: *glInternalFormat = GL_RGB8; *glFormat = GL_RGB; *glType = GL_UNSIGNED_BYTE; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1: *glInternalFormat = GL_RGB5_A1; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_SHORT_5_5_5_1; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: *glInternalFormat = GL_RGBA4; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_SHORT_4_4_4_4; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: *glInternalFormat = GL_RGBA8; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_BYTE; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_R32F; *glFormat = GL_RED; *glType = GL_FLOAT; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_RGB32F; *glFormat = GL_RGB; *glType = GL_FLOAT; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_RGBA32F; *glFormat = GL_RGBA; *glType = GL_FLOAT; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16: if (RLGL.ExtSupported.texFloat16) *glInternalFormat = GL_R16F; *glFormat = GL_RED; *glType = GL_HALF_FLOAT; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16: if (RLGL.ExtSupported.texFloat16) *glInternalFormat = GL_RGB16F; *glFormat = GL_RGB; *glType = GL_HALF_FLOAT; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16: if (RLGL.ExtSupported.texFloat16) *glInternalFormat = GL_RGBA16F; *glFormat = GL_RGBA; *glType = GL_HALF_FLOAT; break;
    #endif
    #if !defined(GRAPHICS_API_OPENGL_11)
        case RL_PIXELFORMAT_COMPRESSED_DXT1_RGB: if (RLGL.ExtSupported.texCompDXT) *glInternalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT; break;
        case RL_PIXELFORMAT_COMPRESSED_DXT1_RGBA: if (RLGL.ExtSupported.texCompDXT) *glInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; break;
        case RL_PIXELFORMAT_COMPRESSED_DXT3_RGBA: if (RLGL.ExtSupported.texCompDXT) *glInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; break;
        case RL_PIXELFORMAT_COMPRESSED_DXT5_RGBA: if (RLGL.ExtSupported.texCompDXT) *glInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; break;
        case RL_PIXELFORMAT_COMPRESSED_ETC1_RGB: if (RLGL.ExtSupported.texCompETC1) *glInternalFormat = GL_ETC1_RGB8_OES; break;                      
        case RL_PIXELFORMAT_COMPRESSED_ETC2_RGB: if (RLGL.ExtSupported.texCompETC2) *glInternalFormat = GL_COMPRESSED_RGB8_ETC2; break;               
        case RL_PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA: if (RLGL.ExtSupported.texCompETC2) *glInternalFormat = GL_COMPRESSED_RGBA8_ETC2_EAC; break;     
        case RL_PIXELFORMAT_COMPRESSED_PVRT_RGB: if (RLGL.ExtSupported.texCompPVRT) *glInternalFormat = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG; break;    
        case RL_PIXELFORMAT_COMPRESSED_PVRT_RGBA: if (RLGL.ExtSupported.texCompPVRT) *glInternalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG; break;  
        case RL_PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA: if (RLGL.ExtSupported.texCompASTC) *glInternalFormat = GL_COMPRESSED_RGBA_ASTC_4x4_KHR; break;  
        case RL_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA: if (RLGL.ExtSupported.texCompASTC) *glInternalFormat = GL_COMPRESSED_RGBA_ASTC_8x8_KHR; break;  
    #endif
        default: TRACELOG(RL_LOG_WARNING, "TEXTURE: Current format not supported (%i)", format); break;
    }
}


void rlUnloadTexture(unsigned int id)
{
    glDeleteTextures(1, &id);
}



void rlGenTextureMipmaps(unsigned int id, int width, int height, int format, int *mipmaps)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glBindTexture(GL_TEXTURE_2D, id);

    
    bool texIsPOT = false;

    if (((width > 0) && ((width & (width - 1)) == 0)) &&
        ((height > 0) && ((height & (height - 1)) == 0))) texIsPOT = true;

    if ((texIsPOT) || (RLGL.ExtSupported.texNPOT))
    {
        
        glGenerateMipmap(GL_TEXTURE_2D);    

        #define MIN(a,b) (((a)<(b))? (a):(b))
        #define MAX(a,b) (((a)>(b))? (a):(b))

        *mipmaps = 1 + (int)floor(log(MAX(width, height))/log(2));
        TRACELOG(RL_LOG_INFO, "TEXTURE: [ID %i] Mipmaps generated automatically, total: %i", id, *mipmaps);
    }
    else TRACELOG(RL_LOG_WARNING, "TEXTURE: [ID %i] Failed to generate mipmaps", id);

    glBindTexture(GL_TEXTURE_2D, 0);
#else
    TRACELOG(RL_LOG_WARNING, "TEXTURE: [ID %i] GPU mipmap generation not supported", id);
#endif
}



void *rlReadTexturePixels(unsigned int id, int width, int height, int format)
{
    void *pixels = NULL;

#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
    glBindTexture(GL_TEXTURE_2D, id);

    
    
    
    
    
    

    
    
    
    
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    unsigned int glInternalFormat, glFormat, glType;
    rlGetGlTextureFormats(format, &glInternalFormat, &glFormat, &glType);
    unsigned int size = rlGetPixelDataSize(width, height, format);

    if ((glInternalFormat != 0) && (format < RL_PIXELFORMAT_COMPRESSED_DXT1_RGB))
    {
        pixels = RL_MALLOC(size);
        glGetTexImage(GL_TEXTURE_2D, 0, glFormat, glType, pixels);
    }
    else TRACELOG(RL_LOG_WARNING, "TEXTURE: [ID %i] Data retrieval not suported for pixel format (%i)", id, format);

    glBindTexture(GL_TEXTURE_2D, 0);
#endif

#if defined(GRAPHICS_API_OPENGL_ES2)
    
    
    
    
    
    
    
    unsigned int fboId = rlLoadFramebuffer(width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    glBindTexture(GL_TEXTURE_2D, 0);

    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, id, 0);

    
    pixels = (unsigned char *)RL_MALLOC(rlGetPixelDataSize(width, height, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8));
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    
    rlUnloadFramebuffer(fboId);
#endif

    return pixels;
}


unsigned char *rlReadScreenPixels(int width, int height)
{
    unsigned char *screenData = (unsigned char *)RL_CALLOC(width*height*4, sizeof(unsigned char));

    
    
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, screenData);

    
    unsigned char *imgData = (unsigned char *)RL_MALLOC(width*height*4*sizeof(unsigned char));

    for (int y = height - 1; y >= 0; y--)
    {
        for (int x = 0; x < (width*4); x++)
        {
            imgData[((height - 1) - y)*width*4 + x] = screenData[(y*width*4) + x];  

            
            
            if (((x + 1)%4) == 0) imgData[((height - 1) - y)*width*4 + x] = 255;
        }
    }

    RL_FREE(screenData);

    return imgData;     
}





unsigned int rlLoadFramebuffer(int width, int height)
{
    unsigned int fboId = 0;

#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)) && defined(RLGL_RENDER_TEXTURES_HINT)
    glGenFramebuffers(1, &fboId);       
    glBindFramebuffer(GL_FRAMEBUFFER, 0);   
#endif

    return fboId;
}



void rlFramebufferAttach(unsigned int fboId, unsigned int texId, int attachType, int texType, int mipLevel)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)) && defined(RLGL_RENDER_TEXTURES_HINT)
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);

    switch (attachType)
    {
        case RL_ATTACHMENT_COLOR_CHANNEL0:
        case RL_ATTACHMENT_COLOR_CHANNEL1:
        case RL_ATTACHMENT_COLOR_CHANNEL2:
        case RL_ATTACHMENT_COLOR_CHANNEL3:
        case RL_ATTACHMENT_COLOR_CHANNEL4:
        case RL_ATTACHMENT_COLOR_CHANNEL5:
        case RL_ATTACHMENT_COLOR_CHANNEL6:
        case RL_ATTACHMENT_COLOR_CHANNEL7:
        {
            if (texType == RL_ATTACHMENT_TEXTURE2D) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachType, GL_TEXTURE_2D, texId, mipLevel);
            else if (texType == RL_ATTACHMENT_RENDERBUFFER) glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachType, GL_RENDERBUFFER, texId);
            else if (texType >= RL_ATTACHMENT_CUBEMAP_POSITIVE_X) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachType, GL_TEXTURE_CUBE_MAP_POSITIVE_X + texType, texId, mipLevel);

        } break;
        case RL_ATTACHMENT_DEPTH:
        {
            if (texType == RL_ATTACHMENT_TEXTURE2D) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texId, mipLevel);
            else if (texType == RL_ATTACHMENT_RENDERBUFFER)  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, texId);

        } break;
        case RL_ATTACHMENT_STENCIL:
        {
            if (texType == RL_ATTACHMENT_TEXTURE2D) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texId, mipLevel);
            else if (texType == RL_ATTACHMENT_RENDERBUFFER)  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, texId);

        } break;
        default: break;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
}


bool rlFramebufferComplete(unsigned int id)
{
    bool result = false;

#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)) && defined(RLGL_RENDER_TEXTURES_HINT)
    glBindFramebuffer(GL_FRAMEBUFFER, id);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        switch (status)
        {
            case GL_FRAMEBUFFER_UNSUPPORTED: TRACELOG(RL_LOG_WARNING, "FBO: [ID %i] Framebuffer is unsupported", id); break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: TRACELOG(RL_LOG_WARNING, "FBO: [ID %i] Framebuffer has incomplete attachment", id); break;
#if defined(GRAPHICS_API_OPENGL_ES2)
            case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS: TRACELOG(RL_LOG_WARNING, "FBO: [ID %i] Framebuffer has incomplete dimensions", id); break;
#endif
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: TRACELOG(RL_LOG_WARNING, "FBO: [ID %i] Framebuffer has a missing attachment", id); break;
            default: break;
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    result = (status == GL_FRAMEBUFFER_COMPLETE);
#endif

    return result;
}



void rlUnloadFramebuffer(unsigned int id)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)) && defined(RLGL_RENDER_TEXTURES_HINT)
    
    int depthType = 0, depthId = 0;
    glBindFramebuffer(GL_FRAMEBUFFER, id);   
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &depthType);

    
    
    
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &depthId);

    unsigned int depthIdU = (unsigned int)depthId;
    if (depthType == GL_RENDERBUFFER) glDeleteRenderbuffers(1, &depthIdU);
    else if (depthType == GL_TEXTURE) glDeleteTextures(1, &depthIdU);

    
    

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &id);

    TRACELOG(RL_LOG_INFO, "FBO: [ID %i] Unloaded framebuffer from VRAM (GPU)", id);
#endif
}




unsigned int rlLoadVertexBuffer(const void *buffer, int size, bool dynamic)
{
    unsigned int id = 0;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glGenBuffers(1, &id);
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferData(GL_ARRAY_BUFFER, size, buffer, dynamic? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
#endif

    return id;
}


unsigned int rlLoadVertexBufferElement(const void *buffer, int size, bool dynamic)
{
    unsigned int id = 0;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glGenBuffers(1, &id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, buffer, dynamic? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
#endif

    return id;
}


void rlEnableVertexBuffer(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glBindBuffer(GL_ARRAY_BUFFER, id);
#endif
}


void rlDisableVertexBuffer(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif
}


void rlEnableVertexBufferElement(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
#endif
}


void rlDisableVertexBufferElement(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif
}



void rlUpdateVertexBuffer(unsigned int id, const void *data, int dataSize, int offset)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferSubData(GL_ARRAY_BUFFER, offset, dataSize, data);
#endif
}



void rlUpdateVertexBufferElements(unsigned int id, const void *data, int dataSize, int offset)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, dataSize, data);
#endif
}


bool rlEnableVertexArray(unsigned int vaoId)
{
    bool result = false;
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if (RLGL.ExtSupported.vao)
    {
        glBindVertexArray(vaoId);
        result = true;
    }
#endif
    return result;
}


void rlDisableVertexArray(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if (RLGL.ExtSupported.vao) glBindVertexArray(0);
#endif
}


void rlEnableVertexAttribute(unsigned int index)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glEnableVertexAttribArray(index);
#endif
}


void rlDisableVertexAttribute(unsigned int index)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glDisableVertexAttribArray(index);
#endif
}


void rlDrawVertexArray(int offset, int count)
{
    glDrawArrays(GL_TRIANGLES, offset, count);
}


void rlDrawVertexArrayElements(int offset, int count, const void *buffer)
{
    
    unsigned short *bufferPtr = (unsigned short *)buffer;
    if (offset > 0) bufferPtr += offset;

    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, (const unsigned short *)bufferPtr);
}


void rlDrawVertexArrayInstanced(int offset, int count, int instances)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glDrawArraysInstanced(GL_TRIANGLES, 0, count, instances);
#endif
}


void rlDrawVertexArrayElementsInstanced(int offset, int count, const void *buffer, int instances)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    
    unsigned short *bufferPtr = (unsigned short *)buffer;
    if (offset > 0) bufferPtr += offset;

    glDrawElementsInstanced(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, (const unsigned short *)bufferPtr, instances);
#endif
}

#if defined(GRAPHICS_API_OPENGL_11)

void rlEnableStatePointer(int vertexAttribType, void *buffer)
{
    if (buffer != NULL) glEnableClientState(vertexAttribType);
    switch (vertexAttribType)
    {
        case GL_VERTEX_ARRAY: glVertexPointer(3, GL_FLOAT, 0, buffer); break;
        case GL_TEXTURE_COORD_ARRAY: glTexCoordPointer(2, GL_FLOAT, 0, buffer); break;
        case GL_NORMAL_ARRAY: if (buffer != NULL) glNormalPointer(GL_FLOAT, 0, buffer); break;
        case GL_COLOR_ARRAY: if (buffer != NULL) glColorPointer(4, GL_UNSIGNED_BYTE, 0, buffer); break;
        
        default: break;
    }
}


void rlDisableStatePointer(int vertexAttribType)
{
    glDisableClientState(vertexAttribType);
}
#endif


unsigned int rlLoadVertexArray(void)
{
    unsigned int vaoId = 0;
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if (RLGL.ExtSupported.vao)
    {
        glGenVertexArrays(1, &vaoId);
    }
#endif
    return vaoId;
}


void rlSetVertexAttribute(unsigned int index, int compSize, int type, bool normalized, int stride, const void *pointer)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glVertexAttribPointer(index, compSize, type, normalized, stride, pointer);
#endif
}


void rlSetVertexAttributeDivisor(unsigned int index, int divisor)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glVertexAttribDivisor(index, divisor);
#endif
}


void rlUnloadVertexArray(unsigned int vaoId)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if (RLGL.ExtSupported.vao)
    {
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &vaoId);
        TRACELOG(RL_LOG_INFO, "VAO: [ID %i] Unloaded vertex array data from VRAM (GPU)", vaoId);
    }
#endif
}


void rlUnloadVertexBuffer(unsigned int vboId)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glDeleteBuffers(1, &vboId);
    
#endif
}





unsigned int rlLoadShaderCode(const char *vsCode, const char *fsCode)
{
    unsigned int id = 0;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    unsigned int vertexShaderId = 0;
    unsigned int fragmentShaderId = 0;

    
    if (vsCode != NULL) vertexShaderId = rlCompileShader(vsCode, GL_VERTEX_SHADER);
    
    if (vertexShaderId == 0) vertexShaderId = RLGL.State.defaultVShaderId;

    
    if (fsCode != NULL) fragmentShaderId = rlCompileShader(fsCode, GL_FRAGMENT_SHADER);
    
    if (fragmentShaderId == 0) fragmentShaderId = RLGL.State.defaultFShaderId;

    
    if ((vertexShaderId == RLGL.State.defaultVShaderId) && (fragmentShaderId == RLGL.State.defaultFShaderId)) id = RLGL.State.defaultShaderId;
    else
    {
        
        id = rlLoadShaderProgram(vertexShaderId, fragmentShaderId);

        
        
        if (vertexShaderId != RLGL.State.defaultVShaderId)
        {
            
            if (id > 0) glDetachShader(id, vertexShaderId);
            glDeleteShader(vertexShaderId);
        }
        if (fragmentShaderId != RLGL.State.defaultFShaderId)
        {
            
            if (id > 0) glDetachShader(id, fragmentShaderId);
            glDeleteShader(fragmentShaderId);
        }

        
        if (id == 0)
        {
            
            TRACELOG(RL_LOG_WARNING, "SHADER: Failed to load custom shader code, using default shader");
            id = RLGL.State.defaultShaderId;
        }
        /*
        else
        {
            
            
            int uniformCount = -1;
            glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &uniformCount);

            for (int i = 0; i < uniformCount; i++)
            {
                int namelen = -1;
                int num = -1;
                char name[256] = { 0 };     
                GLenum type = GL_ZERO;

                
                glGetActiveUniform(id, i, sizeof(name) - 1, &namelen, &num, &type, name);

                name[namelen] = 0;
                TRACELOGD("SHADER: [ID %i] Active uniform (%s) set at location: %i", id, name, glGetUniformLocation(id, name));
            }
        }
        */
    }
#endif

    return id;
}


unsigned int rlCompileShader(const char *shaderCode, int type)
{
    unsigned int shader = 0;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    shader = glCreateShader(type);
    glShaderSource(shader, 1, &shaderCode, NULL);

    GLint success = 0;
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (success == GL_FALSE)
    {
        switch (type)
        {
            case GL_VERTEX_SHADER: TRACELOG(RL_LOG_WARNING, "SHADER: [ID %i] Failed to compile vertex shader code", shader); break;
            case GL_FRAGMENT_SHADER: TRACELOG(RL_LOG_WARNING, "SHADER: [ID %i] Failed to compile fragment shader code", shader); break;
            
        #if defined(GRAPHICS_API_OPENGL_43)
            case GL_COMPUTE_SHADER: TRACELOG(RL_LOG_WARNING, "SHADER: [ID %i] Failed to compile compute shader code", shader); break;
        #endif
            default: break;
        }

        int maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        if (maxLength > 0)
        {
            int length = 0;
            char *log = (char *)RL_CALLOC(maxLength, sizeof(char));
            glGetShaderInfoLog(shader, maxLength, &length, log);
            TRACELOG(RL_LOG_WARNING, "SHADER: [ID %i] Compile error: %s", shader, log);
            RL_FREE(log);
        }
    }
    else
    {
        switch (type)
        {
            case GL_VERTEX_SHADER: TRACELOG(RL_LOG_INFO, "SHADER: [ID %i] Vertex shader compiled successfully", shader); break;
            case GL_FRAGMENT_SHADER: TRACELOG(RL_LOG_INFO, "SHADER: [ID %i] Fragment shader compiled successfully", shader); break;
            
        #if defined(GRAPHICS_API_OPENGL_43)
            case GL_COMPUTE_SHADER: TRACELOG(RL_LOG_INFO, "SHADER: [ID %i] Compute shader compiled successfully", shader); break;
        #endif
            default: break;
        }
    }
#endif

    return shader;
}


unsigned int rlLoadShaderProgram(unsigned int vShaderId, unsigned int fShaderId)
{
    unsigned int program = 0;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    GLint success = 0;
    program = glCreateProgram();

    glAttachShader(program, vShaderId);
    glAttachShader(program, fShaderId);

    
    glBindAttribLocation(program, 0, RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION);
    glBindAttribLocation(program, 1, RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD);
    glBindAttribLocation(program, 2, RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL);
    glBindAttribLocation(program, 3, RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR);
    glBindAttribLocation(program, 4, RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT);
    glBindAttribLocation(program, 5, RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2);

    

    glLinkProgram(program);

    

    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (success == GL_FALSE)
    {
        TRACELOG(RL_LOG_WARNING, "SHADER: [ID %i] Failed to link shader program", program);

        int maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        if (maxLength > 0)
        {
            int length = 0;
            char *log = (char *)RL_CALLOC(maxLength, sizeof(char));
            glGetProgramInfoLog(program, maxLength, &length, log);
            TRACELOG(RL_LOG_WARNING, "SHADER: [ID %i] Link error: %s", program, log);
            RL_FREE(log);
        }

        glDeleteProgram(program);

        program = 0;
    }
    else
    {
        
        
        
        

        TRACELOG(RL_LOG_INFO, "SHADER: [ID %i] Program shader loaded successfully", program);
    }
#endif
    return program;
}


void rlUnloadShaderProgram(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glDeleteProgram(id);

    TRACELOG(RL_LOG_INFO, "SHADER: [ID %i] Unloaded shader program data from VRAM (GPU)", id);
#endif
}


int rlGetLocationUniform(unsigned int shaderId, const char *uniformName)
{
    int location = -1;
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    location = glGetUniformLocation(shaderId, uniformName);

    
    
#endif
    return location;
}


int rlGetLocationAttrib(unsigned int shaderId, const char *attribName)
{
    int location = -1;
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    location = glGetAttribLocation(shaderId, attribName);

    
    
#endif
    return location;
}


void rlSetUniform(int locIndex, const void *value, int uniformType, int count)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    switch (uniformType)
    {
        case RL_SHADER_UNIFORM_FLOAT: glUniform1fv(locIndex, count, (float *)value); break;
        case RL_SHADER_UNIFORM_VEC2: glUniform2fv(locIndex, count, (float *)value); break;
        case RL_SHADER_UNIFORM_VEC3: glUniform3fv(locIndex, count, (float *)value); break;
        case RL_SHADER_UNIFORM_VEC4: glUniform4fv(locIndex, count, (float *)value); break;
        case RL_SHADER_UNIFORM_INT: glUniform1iv(locIndex, count, (int *)value); break;
        case RL_SHADER_UNIFORM_IVEC2: glUniform2iv(locIndex, count, (int *)value); break;
        case RL_SHADER_UNIFORM_IVEC3: glUniform3iv(locIndex, count, (int *)value); break;
        case RL_SHADER_UNIFORM_IVEC4: glUniform4iv(locIndex, count, (int *)value); break;
        case RL_SHADER_UNIFORM_SAMPLER2D: glUniform1iv(locIndex, count, (int *)value); break;
        default: TRACELOG(RL_LOG_WARNING, "SHADER: Failed to set uniform value, data type not recognized");
    }
#endif
}


void rlSetVertexAttributeDefault(int locIndex, const void *value, int attribType, int count)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    switch (attribType)
    {
        case RL_SHADER_ATTRIB_FLOAT: if (count == 1) glVertexAttrib1fv(locIndex, (float *)value); break;
        case RL_SHADER_ATTRIB_VEC2: if (count == 2) glVertexAttrib2fv(locIndex, (float *)value); break;
        case RL_SHADER_ATTRIB_VEC3: if (count == 3) glVertexAttrib3fv(locIndex, (float *)value); break;
        case RL_SHADER_ATTRIB_VEC4: if (count == 4) glVertexAttrib4fv(locIndex, (float *)value); break;
        default: TRACELOG(RL_LOG_WARNING, "SHADER: Failed to set attrib default value, data type not recognized");
    }
#endif
}


void rlSetUniformMatrix(int locIndex, Matrix mat)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    float matfloat[16] = {
        mat.m0, mat.m1, mat.m2, mat.m3,
        mat.m4, mat.m5, mat.m6, mat.m7,
        mat.m8, mat.m9, mat.m10, mat.m11,
        mat.m12, mat.m13, mat.m14, mat.m15
    };
    glUniformMatrix4fv(locIndex, 1, false, matfloat);
#endif
}


void rlSetUniformSampler(int locIndex, unsigned int textureId)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    
    for (int i = 0; i < RL_DEFAULT_BATCH_MAX_TEXTURE_UNITS; i++) if (RLGL.State.activeTextureId[i] == textureId) return;

    
    
    for (int i = 0; i < RL_DEFAULT_BATCH_MAX_TEXTURE_UNITS; i++)
    {
        if (RLGL.State.activeTextureId[i] == 0)
        {
            glUniform1i(locIndex, 1 + i);              
            RLGL.State.activeTextureId[i] = textureId; 
            break;
        }
    }
#endif
}


void rlSetShader(unsigned int id, int *locs)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if (RLGL.State.currentShaderId != id)
    {
        rlDrawRenderBatch(RLGL.currentBatch);
        RLGL.State.currentShaderId = id;
        RLGL.State.currentShaderLocs = locs;
    }
#endif
}


unsigned int rlLoadComputeShaderProgram(unsigned int shaderId)
{
    unsigned int program = 0;

#if defined(GRAPHICS_API_OPENGL_43)
    GLint success = 0;
    program = glCreateProgram();
    glAttachShader(program, shaderId);
    glLinkProgram(program);

    

    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (success == GL_FALSE)
    {
        TRACELOG(RL_LOG_WARNING, "SHADER: [ID %i] Failed to link compute shader program", program);

        int maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        if (maxLength > 0)
        {
            int length = 0;
            char *log = (char *)RL_CALLOC(maxLength, sizeof(char));
            glGetProgramInfoLog(program, maxLength, &length, log);
            TRACELOG(RL_LOG_WARNING, "SHADER: [ID %i] Link error: %s", program, log);
            RL_FREE(log);
        }

        glDeleteProgram(program);

        program = 0;
    }
    else
    {
        
        
        
        

        TRACELOG(RL_LOG_INFO, "SHADER: [ID %i] Compute shader program loaded successfully", program);
    }
#endif

    return program;
}


void rlComputeShaderDispatch(unsigned int groupX, unsigned int groupY, unsigned int groupZ)
{
#if defined(GRAPHICS_API_OPENGL_43)
    glDispatchCompute(groupX, groupY, groupZ);
#endif
}


unsigned int rlLoadShaderBuffer(unsigned int size, const void *data, int usageHint)
{
    unsigned int ssbo = 0;

#if defined(GRAPHICS_API_OPENGL_43)
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, usageHint? usageHint : RL_STREAM_COPY);
    if (data == NULL) glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE, NULL);    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
#endif

    return ssbo;
}


void rlUnloadShaderBuffer(unsigned int ssboId)
{
#if defined(GRAPHICS_API_OPENGL_43)
    glDeleteBuffers(1, &ssboId);
#endif
}


void rlUpdateShaderBuffer(unsigned int id, const void *data, unsigned int dataSize, unsigned int offset)
{
#if defined(GRAPHICS_API_OPENGL_43)
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, dataSize, data);
#endif
}


unsigned int rlGetShaderBufferSize(unsigned int id)
{
    long long size = 0;

#if defined(GRAPHICS_API_OPENGL_43)
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
    glGetInteger64v(GL_SHADER_STORAGE_BUFFER_SIZE, &size);
#endif

    return (size > 0)? (unsigned int)size : 0;
}


void rlReadShaderBuffer(unsigned int id, void *dest, unsigned int count, unsigned int offset)
{
#if defined(GRAPHICS_API_OPENGL_43)
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, count, dest);
#endif
}


void rlBindShaderBuffer(unsigned int id, unsigned int index)
{
#if defined(GRAPHICS_API_OPENGL_43)
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, id);
#endif
}


void rlCopyShaderBuffer(unsigned int destId, unsigned int srcId, unsigned int destOffset, unsigned int srcOffset, unsigned int count)
{
#if defined(GRAPHICS_API_OPENGL_43)
    glBindBuffer(GL_COPY_READ_BUFFER, srcId);
    glBindBuffer(GL_COPY_WRITE_BUFFER, destId);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, srcOffset, destOffset, count);
#endif
}


void rlBindImageTexture(unsigned int id, unsigned int index, int format, bool readonly)
{
#if defined(GRAPHICS_API_OPENGL_43)
    unsigned int glInternalFormat = 0, glFormat = 0, glType = 0;

    rlGetGlTextureFormats(format, &glInternalFormat, &glFormat, &glType);
    glBindImageTexture(index, id, 0, 0, 0, readonly? GL_READ_ONLY : GL_READ_WRITE, glInternalFormat);
#endif
}




Matrix rlGetMatrixModelview(void)
{
    Matrix matrix = rlMatrixIdentity();
#if defined(GRAPHICS_API_OPENGL_11)
    float mat[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, mat);
    matrix.m0 = mat[0];
    matrix.m1 = mat[1];
    matrix.m2 = mat[2];
    matrix.m3 = mat[3];
    matrix.m4 = mat[4];
    matrix.m5 = mat[5];
    matrix.m6 = mat[6];
    matrix.m7 = mat[7];
    matrix.m8 = mat[8];
    matrix.m9 = mat[9];
    matrix.m10 = mat[10];
    matrix.m11 = mat[11];
    matrix.m12 = mat[12];
    matrix.m13 = mat[13];
    matrix.m14 = mat[14];
    matrix.m15 = mat[15];
#else
    matrix = RLGL.State.modelview;
#endif
    return matrix;
}


Matrix rlGetMatrixProjection(void)
{
#if defined(GRAPHICS_API_OPENGL_11)
    float mat[16];
    glGetFloatv(GL_PROJECTION_MATRIX,mat);
    Matrix m;
    m.m0 = mat[0];
    m.m1 = mat[1];
    m.m2 = mat[2];
    m.m3 = mat[3];
    m.m4 = mat[4];
    m.m5 = mat[5];
    m.m6 = mat[6];
    m.m7 = mat[7];
    m.m8 = mat[8];
    m.m9 = mat[9];
    m.m10 = mat[10];
    m.m11 = mat[11];
    m.m12 = mat[12];
    m.m13 = mat[13];
    m.m14 = mat[14];
    m.m15 = mat[15];
    return m;
#else
    return RLGL.State.projection;
#endif
}


Matrix rlGetMatrixTransform(void)
{
    Matrix mat = rlMatrixIdentity();
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    
    
    
    
    mat = RLGL.State.transform;
#endif
    return mat;
}


RLAPI Matrix rlGetMatrixProjectionStereo(int eye)
{
    Matrix mat = rlMatrixIdentity();
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    mat = RLGL.State.projectionStereo[eye];
#endif
    return mat;
}


RLAPI Matrix rlGetMatrixViewOffsetStereo(int eye)
{
    Matrix mat = rlMatrixIdentity();
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    mat = RLGL.State.viewOffsetStereo[eye];
#endif
    return mat;
}


void rlSetMatrixModelview(Matrix view)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    RLGL.State.modelview = view;
#endif
}


void rlSetMatrixProjection(Matrix projection)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    RLGL.State.projection = projection;
#endif
}


void rlSetMatrixProjectionStereo(Matrix right, Matrix left)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    RLGL.State.projectionStereo[0] = right;
    RLGL.State.projectionStereo[1] = left;
#endif
}


void rlSetMatrixViewOffsetStereo(Matrix right, Matrix left)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    RLGL.State.viewOffsetStereo[0] = right;
    RLGL.State.viewOffsetStereo[1] = left;
#endif
}


void rlLoadDrawQuad(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    unsigned int quadVAO = 0;
    unsigned int quadVBO = 0;

    float vertices[] = {
         
        -1.0f,  1.0f, 0.0f,   0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f,
         1.0f,  1.0f, 0.0f,   1.0f, 1.0f,
         1.0f, -1.0f, 0.0f,   1.0f, 0.0f,
    };

    
    glGenVertexArrays(1, &quadVAO);
    glBindVertexArray(quadVAO);

    
    glGenBuffers(1, &quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void *)0); 
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void *)(3*sizeof(float))); 

    
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    
    glDeleteBuffers(1, &quadVBO);
    glDeleteVertexArrays(1, &quadVAO);
#endif
}


void rlLoadDrawCube(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    unsigned int cubeVAO = 0;
    unsigned int cubeVBO = 0;

    float vertices[] = {
         
        -1.0f, -1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
         1.0f,  1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
         1.0f, -1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   1.0f, 0.0f,
         1.0f,  1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
        -1.0f,  1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f, 1.0f,
        -1.0f, -1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
         1.0f, -1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   1.0f, 0.0f,
         1.0f,  1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
         1.0f,  1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
        -1.0f,  1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   0.0f, 1.0f,
        -1.0f, -1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
        -1.0f,  1.0f,  1.0f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
        -1.0f,  1.0f, -1.0f,  -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
        -1.0f, -1.0f,  1.0f,  -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
        -1.0f,  1.0f,  1.0f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
         1.0f,  1.0f,  1.0f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
         1.0f, -1.0f, -1.0f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
         1.0f,  1.0f, -1.0f,   1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
         1.0f, -1.0f, -1.0f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
         1.0f,  1.0f,  1.0f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
         1.0f, -1.0f,  1.0f,   1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
        -1.0f, -1.0f, -1.0f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f,
         1.0f, -1.0f, -1.0f,   0.0f, -1.0f,  0.0f,   1.0f, 1.0f,
         1.0f, -1.0f,  1.0f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
         1.0f, -1.0f,  1.0f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
        -1.0f, -1.0f,  1.0f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f,
        -1.0f, -1.0f, -1.0f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f,
        -1.0f,  1.0f, -1.0f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
         1.0f,  1.0f,  1.0f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
         1.0f,  1.0f, -1.0f,   0.0f,  1.0f,  0.0f,   1.0f, 1.0f,
         1.0f,  1.0f,  1.0f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
        -1.0f,  1.0f, -1.0f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
        -1.0f,  1.0f,  1.0f,   0.0f,  1.0f,  0.0f,   0.0f, 0.0f
    };

    
    glGenVertexArrays(1, &cubeVAO);
    glBindVertexArray(cubeVAO);

    
    glGenBuffers(1, &cubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    
    glBindVertexArray(cubeVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void *)0); 
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void *)(3*sizeof(float))); 
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void *)(6*sizeof(float))); 
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    
    glDeleteBuffers(1, &cubeVBO);
    glDeleteVertexArrays(1, &cubeVAO);
#endif
}


const char *rlGetPixelFormatName(unsigned int format)
{
    switch (format)
    {
        case RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: return "GRAYSCALE"; break;         
        case RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA: return "GRAY_ALPHA"; break;       
        case RL_PIXELFORMAT_UNCOMPRESSED_R5G6B5: return "R5G6B5"; break;               
        case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8: return "R8G8B8"; break;               
        case RL_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1: return "R5G5B5A1"; break;           
        case RL_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: return "R4G4B4A4"; break;           
        case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: return "R8G8B8A8"; break;           
        case RL_PIXELFORMAT_UNCOMPRESSED_R32: return "R32"; break;                     
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32: return "R32G32B32"; break;         
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32: return "R32G32B32A32"; break;   
        case RL_PIXELFORMAT_UNCOMPRESSED_R16: return "R16"; break;                     
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16: return "R16G16B16"; break;         
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16: return "R16G16B16A16"; break;   
        case RL_PIXELFORMAT_COMPRESSED_DXT1_RGB: return "DXT1_RGB"; break;             
        case RL_PIXELFORMAT_COMPRESSED_DXT1_RGBA: return "DXT1_RGBA"; break;           
        case RL_PIXELFORMAT_COMPRESSED_DXT3_RGBA: return "DXT3_RGBA"; break;           
        case RL_PIXELFORMAT_COMPRESSED_DXT5_RGBA: return "DXT5_RGBA"; break;           
        case RL_PIXELFORMAT_COMPRESSED_ETC1_RGB: return "ETC1_RGB"; break;             
        case RL_PIXELFORMAT_COMPRESSED_ETC2_RGB: return "ETC2_RGB"; break;             
        case RL_PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA: return "ETC2_RGBA"; break;       
        case RL_PIXELFORMAT_COMPRESSED_PVRT_RGB: return "PVRT_RGB"; break;             
        case RL_PIXELFORMAT_COMPRESSED_PVRT_RGBA: return "PVRT_RGBA"; break;           
        case RL_PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA: return "ASTC_4x4_RGBA"; break;   
        case RL_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA: return "ASTC_8x8_RGBA"; break;   
        default: return "UNKNOWN"; break;
    }
}




#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)



static void rlLoadShaderDefault(void)
{
    RLGL.State.defaultShaderLocs = (int *)RL_CALLOC(RL_MAX_SHADER_LOCATIONS, sizeof(int));

    
    for (int i = 0; i < RL_MAX_SHADER_LOCATIONS; i++) RLGL.State.defaultShaderLocs[i] = -1;

    
    const char *defaultVShaderCode =
#if defined(GRAPHICS_API_OPENGL_21)
    "#version 120                       \n"
    "attribute vec3 vertexPosition;     \n"
    "attribute vec2 vertexTexCoord;     \n"
    "attribute vec4 vertexColor;        \n"
    "varying vec2 fragTexCoord;         \n"
    "varying vec4 fragColor;            \n"
#elif defined(GRAPHICS_API_OPENGL_33)
    "#version 330                       \n"
    "in vec3 vertexPosition;            \n"
    "in vec2 vertexTexCoord;            \n"
    "in vec4 vertexColor;               \n"
    "out vec2 fragTexCoord;             \n"
    "out vec4 fragColor;                \n"
#endif
#if defined(GRAPHICS_API_OPENGL_ES2)
    "#version 100                       \n"
    "precision mediump float;           \n"     
    "attribute vec3 vertexPosition;     \n"
    "attribute vec2 vertexTexCoord;     \n"
    "attribute vec4 vertexColor;        \n"
    "varying vec2 fragTexCoord;         \n"
    "varying vec4 fragColor;            \n"
#endif
    "uniform mat4 mvp;                  \n"
    "void main()                        \n"
    "{                                  \n"
    "    fragTexCoord = vertexTexCoord; \n"
    "    fragColor = vertexColor;       \n"
    "    gl_Position = mvp*vec4(vertexPosition, 1.0); \n"
    "}                                  \n";

    
    const char *defaultFShaderCode =
#if defined(GRAPHICS_API_OPENGL_21)
    "#version 120                       \n"
    "varying vec2 fragTexCoord;         \n"
    "varying vec4 fragColor;            \n"
    "uniform sampler2D texture0;        \n"
    "uniform vec4 colDiffuse;           \n"
    "void main()                        \n"
    "{                                  \n"
    "    vec4 texelColor = texture2D(texture0, fragTexCoord); \n"
    "    gl_FragColor = texelColor*colDiffuse*fragColor;      \n"
    "}                                  \n";
#elif defined(GRAPHICS_API_OPENGL_33)
    "#version 330       \n"
    "in vec2 fragTexCoord;              \n"
    "in vec4 fragColor;                 \n"
    "out vec4 finalColor;               \n"
    "uniform sampler2D texture0;        \n"
    "uniform vec4 colDiffuse;           \n"
    "void main()                        \n"
    "{                                  \n"
    "    vec4 texelColor = texture(texture0, fragTexCoord);   \n"
    "    finalColor = texelColor*colDiffuse*fragColor;        \n"
    "}                                  \n";
#endif
#if defined(GRAPHICS_API_OPENGL_ES2)
    "#version 100                       \n"
    "precision mediump float;           \n"     
    "varying vec2 fragTexCoord;         \n"
    "varying vec4 fragColor;            \n"
    "uniform sampler2D texture0;        \n"
    "uniform vec4 colDiffuse;           \n"
    "void main()                        \n"
    "{                                  \n"
    "    vec4 texelColor = texture2D(texture0, fragTexCoord); \n"
    "    gl_FragColor = texelColor*colDiffuse*fragColor;      \n"
    "}                                  \n";
#endif

    
    
    RLGL.State.defaultVShaderId = rlCompileShader(defaultVShaderCode, GL_VERTEX_SHADER);     
    RLGL.State.defaultFShaderId = rlCompileShader(defaultFShaderCode, GL_FRAGMENT_SHADER);   

    RLGL.State.defaultShaderId = rlLoadShaderProgram(RLGL.State.defaultVShaderId, RLGL.State.defaultFShaderId);

    if (RLGL.State.defaultShaderId > 0)
    {
        TRACELOG(RL_LOG_INFO, "SHADER: [ID %i] Default shader loaded successfully", RLGL.State.defaultShaderId);

        
        RLGL.State.defaultShaderLocs[RL_SHADER_LOC_VERTEX_POSITION] = glGetAttribLocation(RLGL.State.defaultShaderId, "vertexPosition");
        RLGL.State.defaultShaderLocs[RL_SHADER_LOC_VERTEX_TEXCOORD01] = glGetAttribLocation(RLGL.State.defaultShaderId, "vertexTexCoord");
        RLGL.State.defaultShaderLocs[RL_SHADER_LOC_VERTEX_COLOR] = glGetAttribLocation(RLGL.State.defaultShaderId, "vertexColor");

        
        RLGL.State.defaultShaderLocs[RL_SHADER_LOC_MATRIX_MVP]  = glGetUniformLocation(RLGL.State.defaultShaderId, "mvp");
        RLGL.State.defaultShaderLocs[RL_SHADER_LOC_COLOR_DIFFUSE] = glGetUniformLocation(RLGL.State.defaultShaderId, "colDiffuse");
        RLGL.State.defaultShaderLocs[RL_SHADER_LOC_MAP_DIFFUSE] = glGetUniformLocation(RLGL.State.defaultShaderId, "texture0");
    }
    else TRACELOG(RL_LOG_WARNING, "SHADER: [ID %i] Failed to load default shader", RLGL.State.defaultShaderId);
}



static void rlUnloadShaderDefault(void)
{
    glUseProgram(0);

    glDetachShader(RLGL.State.defaultShaderId, RLGL.State.defaultVShaderId);
    glDetachShader(RLGL.State.defaultShaderId, RLGL.State.defaultFShaderId);
    glDeleteShader(RLGL.State.defaultVShaderId);
    glDeleteShader(RLGL.State.defaultFShaderId);

    glDeleteProgram(RLGL.State.defaultShaderId);

    RL_FREE(RLGL.State.defaultShaderLocs);

    TRACELOG(RL_LOG_INFO, "SHADER: [ID %i] Default shader unloaded successfully", RLGL.State.defaultShaderId);
}

#if defined(RLGL_SHOW_GL_DETAILS_INFO)

static const char *rlGetCompressedFormatName(int format)
{
    switch (format)
    {
        
        case 0x83F0: return "GL_COMPRESSED_RGB_S3TC_DXT1_EXT"; break;
        case 0x83F1: return "GL_COMPRESSED_RGBA_S3TC_DXT1_EXT"; break;
        case 0x83F2: return "GL_COMPRESSED_RGBA_S3TC_DXT3_EXT"; break;
        case 0x83F3: return "GL_COMPRESSED_RGBA_S3TC_DXT5_EXT"; break;
        
        case 0x86B0: return "GL_COMPRESSED_RGB_FXT1_3DFX"; break;
        case 0x86B1: return "GL_COMPRESSED_RGBA_FXT1_3DFX"; break;
        
        case 0x8C00: return "GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG"; break;
        case 0x8C01: return "GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG"; break;
        case 0x8C02: return "GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG"; break;
        case 0x8C03: return "GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG"; break;
        
        case 0x8D64: return "GL_ETC1_RGB8_OES"; break;
        
        case 0x8DBB: return "GL_COMPRESSED_RED_RGTC1"; break;
        case 0x8DBC: return "GL_COMPRESSED_SIGNED_RED_RGTC1"; break;
        case 0x8DBD: return "GL_COMPRESSED_RG_RGTC2"; break;
        case 0x8DBE: return "GL_COMPRESSED_SIGNED_RG_RGTC2"; break;
        
        case 0x8E8C: return "GL_COMPRESSED_RGBA_BPTC_UNORM_ARB"; break;
        case 0x8E8D: return "GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB"; break;
        case 0x8E8E: return "GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB"; break;
        case 0x8E8F: return "GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB"; break;
        
        case 0x9274: return "GL_COMPRESSED_RGB8_ETC2"; break;
        case 0x9275: return "GL_COMPRESSED_SRGB8_ETC2"; break;
        case 0x9276: return "GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2"; break;
        case 0x9277: return "GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2"; break;
        case 0x9278: return "GL_COMPRESSED_RGBA8_ETC2_EAC"; break;
        case 0x9279: return "GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC"; break;
        case 0x9270: return "GL_COMPRESSED_R11_EAC"; break;
        case 0x9271: return "GL_COMPRESSED_SIGNED_R11_EAC"; break;
        case 0x9272: return "GL_COMPRESSED_RG11_EAC"; break;
        case 0x9273: return "GL_COMPRESSED_SIGNED_RG11_EAC"; break;
        
        case 0x93B0: return "GL_COMPRESSED_RGBA_ASTC_4x4_KHR"; break;
        case 0x93B1: return "GL_COMPRESSED_RGBA_ASTC_5x4_KHR"; break;
        case 0x93B2: return "GL_COMPRESSED_RGBA_ASTC_5x5_KHR"; break;
        case 0x93B3: return "GL_COMPRESSED_RGBA_ASTC_6x5_KHR"; break;
        case 0x93B4: return "GL_COMPRESSED_RGBA_ASTC_6x6_KHR"; break;
        case 0x93B5: return "GL_COMPRESSED_RGBA_ASTC_8x5_KHR"; break;
        case 0x93B6: return "GL_COMPRESSED_RGBA_ASTC_8x6_KHR"; break;
        case 0x93B7: return "GL_COMPRESSED_RGBA_ASTC_8x8_KHR"; break;
        case 0x93B8: return "GL_COMPRESSED_RGBA_ASTC_10x5_KHR"; break;
        case 0x93B9: return "GL_COMPRESSED_RGBA_ASTC_10x6_KHR"; break;
        case 0x93BA: return "GL_COMPRESSED_RGBA_ASTC_10x8_KHR"; break;
        case 0x93BB: return "GL_COMPRESSED_RGBA_ASTC_10x10_KHR"; break;
        case 0x93BC: return "GL_COMPRESSED_RGBA_ASTC_12x10_KHR"; break;
        case 0x93BD: return "GL_COMPRESSED_RGBA_ASTC_12x12_KHR"; break;
        case 0x93D0: return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR"; break;
        case 0x93D1: return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR"; break;
        case 0x93D2: return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR"; break;
        case 0x93D3: return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR"; break;
        case 0x93D4: return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR"; break;
        case 0x93D5: return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR"; break;
        case 0x93D6: return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR"; break;
        case 0x93D7: return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR"; break;
        case 0x93D8: return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR"; break;
        case 0x93D9: return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR"; break;
        case 0x93DA: return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR"; break;
        case 0x93DB: return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR"; break;
        case 0x93DC: return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR"; break;
        case 0x93DD: return "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR"; break;
        default: return "GL_COMPRESSED_UNKNOWN"; break;
    }
}
#endif  

#endif  



static int rlGetPixelDataSize(int width, int height, int format)
{
    int dataSize = 0;       
    int bpp = 0;            

    switch (format)
    {
        case RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: bpp = 8; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
        case RL_PIXELFORMAT_UNCOMPRESSED_R5G6B5:
        case RL_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
        case RL_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: bpp = 16; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: bpp = 32; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8: bpp = 24; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R32: bpp = 32; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32: bpp = 32*3; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32: bpp = 32*4; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16: bpp = 16; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16: bpp = 16*3; break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16: bpp = 16*4; break;
        case RL_PIXELFORMAT_COMPRESSED_DXT1_RGB:
        case RL_PIXELFORMAT_COMPRESSED_DXT1_RGBA:
        case RL_PIXELFORMAT_COMPRESSED_ETC1_RGB:
        case RL_PIXELFORMAT_COMPRESSED_ETC2_RGB:
        case RL_PIXELFORMAT_COMPRESSED_PVRT_RGB:
        case RL_PIXELFORMAT_COMPRESSED_PVRT_RGBA: bpp = 4; break;
        case RL_PIXELFORMAT_COMPRESSED_DXT3_RGBA:
        case RL_PIXELFORMAT_COMPRESSED_DXT5_RGBA:
        case RL_PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA:
        case RL_PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA: bpp = 8; break;
        case RL_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA: bpp = 2; break;
        default: break;
    }

    dataSize = width*height*bpp/8;  

    
    
    if ((width < 4) && (height < 4))
    {
        if ((format >= RL_PIXELFORMAT_COMPRESSED_DXT1_RGB) && (format < RL_PIXELFORMAT_COMPRESSED_DXT3_RGBA)) dataSize = 8;
        else if ((format >= RL_PIXELFORMAT_COMPRESSED_DXT3_RGBA) && (format < RL_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA)) dataSize = 16;
    }

    return dataSize;
}




static Matrix rlMatrixIdentity(void)
{
    Matrix result = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    return result;
}



static Matrix rlMatrixMultiply(Matrix left, Matrix right)
{
    Matrix result = { 0 };

    result.m0 = left.m0*right.m0 + left.m1*right.m4 + left.m2*right.m8 + left.m3*right.m12;
    result.m1 = left.m0*right.m1 + left.m1*right.m5 + left.m2*right.m9 + left.m3*right.m13;
    result.m2 = left.m0*right.m2 + left.m1*right.m6 + left.m2*right.m10 + left.m3*right.m14;
    result.m3 = left.m0*right.m3 + left.m1*right.m7 + left.m2*right.m11 + left.m3*right.m15;
    result.m4 = left.m4*right.m0 + left.m5*right.m4 + left.m6*right.m8 + left.m7*right.m12;
    result.m5 = left.m4*right.m1 + left.m5*right.m5 + left.m6*right.m9 + left.m7*right.m13;
    result.m6 = left.m4*right.m2 + left.m5*right.m6 + left.m6*right.m10 + left.m7*right.m14;
    result.m7 = left.m4*right.m3 + left.m5*right.m7 + left.m6*right.m11 + left.m7*right.m15;
    result.m8 = left.m8*right.m0 + left.m9*right.m4 + left.m10*right.m8 + left.m11*right.m12;
    result.m9 = left.m8*right.m1 + left.m9*right.m5 + left.m10*right.m9 + left.m11*right.m13;
    result.m10 = left.m8*right.m2 + left.m9*right.m6 + left.m10*right.m10 + left.m11*right.m14;
    result.m11 = left.m8*right.m3 + left.m9*right.m7 + left.m10*right.m11 + left.m11*right.m15;
    result.m12 = left.m12*right.m0 + left.m13*right.m4 + left.m14*right.m8 + left.m15*right.m12;
    result.m13 = left.m12*right.m1 + left.m13*right.m5 + left.m14*right.m9 + left.m15*right.m13;
    result.m14 = left.m12*right.m2 + left.m13*right.m6 + left.m14*right.m10 + left.m15*right.m14;
    result.m15 = left.m12*right.m3 + left.m13*right.m7 + left.m14*right.m11 + left.m15*right.m15;

    return result;
}

#endif  
