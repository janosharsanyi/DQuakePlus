/*
Copyright (C) 1996-1997 Id Software, Inc.
Copyright (C) 2007 Peter Mackay and Chris Swindle.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include <psptypes.h>

void GL_BeginRendering (int *x, int *y, int *width, int *height);
void GL_EndRendering (void);
u32 GL_GetDrawBuffer(void);

//for render to texture
void GL_BeginTarget (int format, int width, int height, int stride);
byte* GL_EndTarget (void);


void GL_Upload8(int texture_index, const byte *data, int width, int height);
void GL_Upload16(int texture_index, const byte *data, int width, int height);
int  GL_LoadTexture(const char *identifier, int width, int height, const byte *data, qboolean stretch_to_power_of_two, int filter, int mipmap_level);
int GL_LoadTextureLM (const char *identifier, int width, int height, const byte *data, int bpp, int filter, qboolean update, int forcopy);
int GL_LoadImages (const char *identifier, int width, int height, const byte *data, qboolean stretch_to_power_of_two, int filter, int mipmap_level, int bpp);
int GL_LoadTexturePixels (byte *data, char *identifier, int width, int height, int mode);
int loadtextureimage (char* filename, int matchwidth, int matchheight, qboolean complain, int filter);

//Crow_bar
void GL_GetPixelsBGR (byte *buffer, int width, int height, int i);
void GL_GetPixelsRGB (byte *buffer, int width, int height, int i);
void GL_GetPixelsRGBA(byte *buffer, int width, int height, int i);


#define PAL_RGB  24
#define PAL_RGBA 32
#define PAL_Q2   64 //Quake II palette
#define PAL_H2   65 //Hexen II palette

int GL_LoadPalTex (const char *identifier, int width, int height, const byte *data, qboolean stretch_to_power_of_two, int filter, int mipmap_level, byte *palette, int paltype);

int GL_LoadPalletedTexture (byte *in, char *identifier, int width, int height, int mode);

void GL_UnloadTexture (const int texture_index);

extern	int glx, gly, glwidth, glheight;

// r_local.h -- private refresh defs
/*
---------------------------------
half-life Render Modes. Crow_bar
---------------------------------
*/

#define TEX_COLOR    1
#define TEX_TEXTURE  2
#define TEX_GLOW     3
#define TEX_SOLID    4
#define TEX_ADDITIVE 5
#define TEX_LMPOINT  6 //for light point

#define ISCOLOR(ent)    ((ent)->rendermode == TEX_COLOR    && ((ent)->rendercolor[0] <= 1|| \
                                                               (ent)->rendercolor[1] <= 1|| \
															   (ent)->rendercolor[2] <= 1))
															   
#define ISTEXTURE(ent)  ((ent)->rendermode == TEX_TEXTURE  && (ent)->renderamt > 0 && (ent)->renderamt <= 1)
#define ISGLOW(ent)     ((ent)->rendermode == TEX_GLOW     && (ent)->renderamt > 0 && (ent)->renderamt <= 1)
#define ISSOLID(ent)    ((ent)->rendermode == TEX_SOLID    && (ent)->renderamt > 0 && (ent)->renderamt <= 1)
#define ISADDITIVE(ent) ((ent)->rendermode == TEX_ADDITIVE && (ent)->renderamt > 0 && (ent)->renderamt <= 1)

#define ISLMPOINT(ent)  ((ent)->rendermode == TEX_LMPOINT  && ((ent)->rendercolor[0] <= 1|| \
                                                               (ent)->rendercolor[1] <= 1|| \
															   (ent)->rendercolor[2] <= 1))
/*
---------------------------------
//half-life Render Modes
---------------------------------
*/

#define ALIAS_BASE_SIZE_RATIO		(1.0 / 11.0)
					// normalizing factor so player model works out to about
					//  1 pixel per triangle
#define	MAX_LBM_HEIGHT		480

#define TILE_SIZE		128		// size of textures generated by R_GenTiledSurf

#define SKYSHIFT		7
#define	SKYSIZE			(1 << SKYSHIFT)
#define SKYMASK			(SKYSIZE - 1)

#define BACKFACE_EPSILON	0.01

//#ifdef SLIM
//#define MAX_LIGHTMAPS       128
//#else
#define MAX_LIGHTMAPS       64
//#endif


void R_TimeRefresh_f (void);
void R_ReadPointFile_f (void);
texture_t *R_TextureAnimation (texture_t *base);

typedef struct surfcache_s
{
	struct surfcache_s	*next;
	struct surfcache_s 	**owner;		// NULL is an empty chunk of memory
	int					lightadj[MAXLIGHTMAPS]; // checked for strobe flush
	int					dlight;
	int					size;		// including header
	unsigned			width;
	unsigned			height;		// DEBUG only needed for debug
	float				mipscale;
	struct texture_s	*texture;	// checked for animating textures
	byte				data[4];	// width*height elements
} surfcache_t;

typedef	enum
{
	pm_classic, pm_qmb, pm_quake3, pm_mixed
} part_mode_t;

typedef struct
{
	pixel_t		*surfdat;	// destination for generated surface
	int			rowbytes;	// destination logical width in bytes
	msurface_t	*surf;		// description for surface to generate
	fixed8_t	lightadj[MAXLIGHTMAPS];
							// adjust for lightmap levels for dynamic lighting
	texture_t	*texture;	// corrected for animating textures
	int			surfmip;	// mipmapped ratio of surface texels / world pixels
	int			surfwidth;	// in mipmapped texels
	int			surfheight;	// in mipmapped texels
} drawsurf_t;


typedef enum {
	pt_static, pt_grav, pt_slowgrav, pt_fire, pt_explode, pt_explode2, pt_blob, pt_blob2
} ptype_t;

// !!! if this is changed, it must be changed in d_ifacea.h too !!!
typedef struct particle2_s
{
// driver-usable fields
	vec3_t		org;
	float		color;
// drivers never touch the following fields
	struct particle2_s	*next;
	vec3_t		vel;
	float		ramp;
	float		die;
	ptype_t		type;
} particle2_t;

//====================================================


extern	entity_t	r_worldentity;
extern	qboolean	r_cache_thrash;		// compatability
extern	vec3_t		modelorg, r_entorigin;
extern	entity_t	*currententity;
extern	int			r_visframecount;	// ??? what difs?
extern	int			r_framecount;
extern	mplane_t	frustum[4];
extern	int		c_brush_polys, c_alias_polys;


//
// view origin
//
extern	vec3_t	vup;
extern	vec3_t	vpn;
extern	vec3_t	vright;
extern	vec3_t	r_origin;

//
// screen size info
//
extern	refdef_t	r_refdef;
extern	mleaf_t		*r_viewleaf, *r_oldviewleaf;
extern	texture_t	*r_notexture_mip;
extern	int		d_lightstylevalue[256];	// 8.8 fraction of base light value

extern  int	    skyimage[6]; // Where sky images are stored
extern  int 	lightmap_index[MAX_LIGHTMAPS]; // Where lightmaps are stored

extern  int     reloaded_pallete;

extern	qboolean	envmap;
extern	int	currenttexture;
extern	int	cnttextures[2];
extern	int	particletexture;
extern	int	playertextures;
//extern	int	playertextures[MAX_SCOREBOARD];

extern	int	skytexturenum;		// index in cl.loadmodel, not gl texture object

extern	cvar_t  scr_conheight;

extern	cvar_t	r_partalpha;

extern	cvar_t  r_maxrange;

extern	cvar_t	r_restexf;
extern	cvar_t	r_texcompr;

extern	cvar_t	r_skyfog;
extern	cvar_t	r_skyvflip;
extern	cvar_t	r_skydis;

extern  cvar_t	r_caustics;
extern	cvar_t	r_detail;
extern  cvar_t	r_detail_mipmaps;
extern  cvar_t	r_detail_mipmaps_func;
extern  cvar_t	r_detail_mipmaps_bias;

extern  cvar_t  r_farclip;
extern  cvar_t  r_loadq3models;

extern  cvar_t  r_i_model_animation;
extern  cvar_t  r_i_model_transform;
extern  cvar_t  r_ipolations;
extern  cvar_t  r_asynch;

extern  cvar_t  cl_loadmapcfg;
extern  cvar_t  r_fastsky;
extern  cvar_t  r_skycolor;
extern	cvar_t	r_waterripple;
extern	cvar_t	r_norefresh;
extern	cvar_t	r_drawentities;
extern	cvar_t	r_drawworld;
extern	cvar_t	r_drawviewmodel;
extern	cvar_t	r_speeds;
extern	cvar_t	r_waterwarp;
extern	cvar_t	r_fullbright;
extern	cvar_t	r_lightmap;
extern	cvar_t	r_shadows;
extern	cvar_t	r_mirroralpha;
extern	cvar_t	r_wateralpha;
extern	cvar_t	r_dynamic;
extern	cvar_t	r_novis;
extern	cvar_t	r_tex_scale_down;
extern	cvar_t	r_particles_simple;
extern  cvar_t	r_vsync;
extern  cvar_t	r_mipmaps;
extern  cvar_t	r_mipmaps_func;
extern  cvar_t	r_mipmaps_bias;
extern	cvar_t	gl_keeptjunctions;
extern  cvar_t	r_waterwarp;

extern  cvar_t  r_showbboxes;
extern  cvar_t  r_showbboxes_full;

extern  cvar_t  r_polyblend;

extern  cvar_t  r_showtris;
extern  cvar_t  r_showtris_full;

extern  cvar_t  r_laserpoint;
extern  cvar_t  r_particle_count;
extern  cvar_t	r_part_explosions;
extern  cvar_t	r_part_trails;
extern  cvar_t	r_part_sparks;
extern  cvar_t  r_part_spikes;
extern  cvar_t	r_part_gunshots;
extern  cvar_t	r_part_blood;
extern  cvar_t	r_part_telesplash;
extern  cvar_t	r_part_blobs;
extern  cvar_t	r_part_lavasplash;
extern	cvar_t	r_part_flames;
extern	cvar_t	r_part_lightning;
extern	cvar_t	r_part_flies;
extern	cvar_t	r_particle_count;
extern	cvar_t	r_bounceparticles;
extern	cvar_t  r_explosiontype;
extern  cvar_t	r_part_muzzleflash;
extern  cvar_t	r_flametype;
extern  cvar_t	r_bounceparticles;
extern  cvar_t	r_decal_blood;
extern  cvar_t	r_decal_bullets;
extern  cvar_t	r_decal_sparks;	
extern  cvar_t	r_decal_explosions;
extern  cvar_t  r_coronas;


extern	int			mirrortexturenum;	// quake texturenum, not gltexturenum
extern	qboolean	mirror;
extern	mplane_t	*mirror_plane;

extern	ScePspFMatrix4	r_world_matrix;

void R_TranslatePlayerSkin (int playernum);
void GL_Bind   (int texture_index);
void GL_BindLM (int texture_index);
void GL_Copy   (int texture_index, int sx, int sy, int dx, int dy, int w, int h);

// Added by PM
int R_LightPoint (vec3_t p);
void R_DrawBrushModel (entity_t *e);
void R_AnimateLight (void);
void R_DrawWorld (void);
void R_RenderDlights (void);
void R_DrawParticles (void);
void R_DrawWaterSurfaces (void);
void R_RenderBrushPoly (msurface_t *fa);
void R_InitParticles (void);
void R_ClearParticles (void);
void GL_BuildLightmaps (void);
void GL_MakeAliasModelDisplayLists (model_t *m, aliashdr_t *hdr);
void GL_Set2D (void);
void GL_SubdivideSurface (msurface_t *fa);
void GL_Surface (msurface_t *fa);
void EmitWaterPolys (msurface_t *fa);
void EmitSkyPolys (msurface_t *fa);
void EmitRefPolys (msurface_t *fa);
void EmitBothSkyLayers (msurface_t *fa);
void EmitUnderWaterPolys (void);
void EmitDetailPolys (void);
void R_DrawSkyChain (msurface_t *s);
qboolean R_CullBox (vec3_t emins, vec3_t emaxs);
qboolean R_CullSphere (vec3_t centre, float radius);
void R_MarkLights (dlight_t *light, int bit, mnode_t *node);
void R_RotateForEntity (entity_t *e, int shadow);
void R_BlendedRotateForEntity (entity_t *e, int shadow);
void R_RotateForViewEntity (entity_t *ent); //clone (R_RotateForEntity)
void R_RotateForTagEntity (tagentity_t *tagent, md3tag_t *tag, float *m); //for q3 models
void R_StoreEfrags (efrag_t **ppefrag);
void D_StartParticles (void);
// void D_DrawParticle (particle_t *pparticle);
void D_DrawParticle (particle2_t *pparticle, vec3_t up, vec3_t right, float scale);
void D_EndParticles (void);

void Fog_Init (void);
void Fog_NewMap (void);

void Sky_LoadSkyBox (char *name);
void Sky_NewMap (void);
void Sky_Init (void);
void R_ClearSkyBox (void);
void R_DrawSkyBox (void);

//-----------------------------------------------------
void QMB_InitParticles (void);
void QMB_ClearParticles (void);
void QMB_DrawParticles (void);
void QMB_Q3TorchFlame (vec3_t org, float size);
void QMB_RunParticleEffect (vec3_t org, vec3_t dir, int color, int count);
void QMB_RocketTrail (vec3_t start, vec3_t end, trail_type_t type);
void QMB_BlobExplosion (vec3_t org);
void QMB_ParticleExplosion (vec3_t org);
void QMB_LavaSplash (vec3_t org);
void QMB_TeleportSplash (vec3_t org);
void QMB_InfernoFlame (vec3_t org);
void QMB_StaticBubble (entity_t *ent);
void QMB_ColorMappedExplosion (vec3_t org, int colorStart, int colorLength);
void QMB_TorchFlame (vec3_t org);
void QMB_FlameGt (vec3_t org, float size, float time);
void QMB_BigTorchFlame (vec3_t org);
void QMB_ShamblerCharge (vec3_t org);
void QMB_LightningBeam (vec3_t start, vec3_t end);
//void QMB_GenSparks (vec3_t org, byte col[3], float count, float size, float life);
void QMB_EntityParticles (entity_t *ent);
void QMB_MuzzleFlash (vec3_t org);
void QMB_MuzzleFlashLG (vec3_t org);
void QMB_Q3Gunshot (vec3_t org, int skinnum, float alpha);
void QMB_Q3Teleport (vec3_t org, float alpha);
void QMB_Q3TorchFlame (vec3_t org, float size);

extern	qboolean	qmb_initialized;

void R_SpawnDecal (vec3_t center, vec3_t normal, vec3_t tangent, int tex, int size, int isbsp);
void R_SpawnDecalStatic(vec3_t org, int tex, int size);
void R_SpawnDecalBSP (vec3_t org, char *texname, int size);

void CheckParticles (void);

void UnloadWads (void); //By Crow_bar
#if 0
void ShowErrorDialog(const unsigned int error);
void ShowMessageDialog(const char *message, int enableYesno);
#endif
//====================================================

void Fog_ParseServerMessage (void);

typedef struct {
	float s, t;
	unsigned int color;
	float x, y, z;
} part_vertex;

typedef struct {
	part_vertex first, second;
} psp_particle;


psp_particle* D_CreateBuffer (int size);
void 	  	  D_DeleteBuffer (psp_particle* vertices);
int 	      D_DrawParticleBuffered (psp_particle* vertices, particle2_t *pparticle, vec3_t up, vec3_t right, float scale);
