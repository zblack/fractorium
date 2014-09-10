#pragma once

#include "Point.h"
#include "Isaac.h"

/// <summary>
/// Base variation classes. Individual variations will be grouped into files of roughly 50
/// to avoid a single file becoming too unweildy.
/// </summary>

namespace EmberNs
{
/// <summary>
/// Xform and Variation need each other, but each can't include the other.
/// So Xform includes this file, and use a forward declaration here.
/// </summary>
template <typename T> class Xform;

/// <summary>
/// The type of variation: regular, pre or post.
/// </summary>
enum eVariationType
{
	VARTYPE_REG,
	VARTYPE_PRE,
	VARTYPE_POST,
};

/// <summary>
/// How to handle the results of the variation when it's a pre or post.
/// If the calculation involved the input points, then it should be directly assigned
/// to the output. However, if they did not involve the input points, they should be added
/// to the output.
/// </summary>
enum eVariationAssignType
{
	ASSIGNTYPE_SET,
	ASSIGNTYPE_SUM
};

/// <summary>
/// Complete list of every variation class ID.
/// </summary>
enum eVariationId
{
	VAR_ARCH           ,
	VAR_AUGER		   ,
	VAR_BARYCENTROID   ,
	VAR_BCIRCLE		   ,
	VAR_BCOLLIDE	   ,
	VAR_BENT		   ,
	VAR_BENT2		   ,
	VAR_BIPOLAR		   ,
	VAR_BISPLIT		   ,
	VAR_BLADE		   ,
	VAR_BLADE3D		   ,
	VAR_BLOB		   ,
	VAR_BLOB2		   ,
	VAR_BLOB3D		   ,
	VAR_BLOCKY		   ,
	VAR_BLUR		   ,
	VAR_BLUR_CIRCLE	   ,
	VAR_BLUR_HEART     ,
	VAR_BLUR_LINEAR	   ,
	VAR_BLUR_PIXELIZE  ,
	VAR_BLUR_SQUARE	   ,
	VAR_BLUR_ZOOM	   ,
	VAR_BLUR3D		   ,
	VAR_BMOD		   ,
	VAR_BOARDERS	   ,
	VAR_BOARDERS2	   ,
	VAR_BSWIRL		   ,
	VAR_BTRANSFORM	   ,
	VAR_BUBBLE		   ,
	VAR_BUBBLE2		   ,
	VAR_BUTTERFLY	   ,
	VAR_BWRAPS		   ,
	VAR_CARDIOID	   ,
	VAR_CELL		   ,
	VAR_CHECKS		   ,
	VAR_CIRCLEBLUR	   ,
	VAR_CIRCLECROP     ,
	VAR_CIRCLELINEAR   ,
	VAR_CIRCLERAND     ,
	VAR_CIRCLETRANS1   ,
	VAR_CIRCLIZE	   ,
	VAR_CIRCLIZE2	   ,
	VAR_CIRCUS         ,
	VAR_COLLIDEOSCOPE  ,
	VAR_CONIC		   ,
	VAR_COS			   ,
	VAR_COS_WRAP	   ,
	VAR_COSH		   ,
	VAR_COSHQ          ,
	VAR_COSINE		   ,
	VAR_COSQ           ,
	VAR_COT			   ,
	VAR_COTH		   ,
	VAR_COTHQ		   ,
	VAR_COTQ		   ,
	VAR_CPOW		   ,
	VAR_CPOW2		   ,
	VAR_CRESCENTS	   ,
	VAR_CROP		   ,
	VAR_CROPN		   ,
	VAR_CROSS		   ,
	VAR_CSC			   ,
	VAR_CSCH		   ,
	VAR_CSCHQ		   ,
	VAR_CSCQ		   ,
	VAR_CUBIC3D	       ,
	VAR_CUBIC_LATTICE3D,
	VAR_CURL		   ,
	VAR_CURL3D		   ,
	VAR_CURL_SP        ,
	VAR_CURVATURE      ,
	VAR_CURVE		   ,
	VAR_CYLINDER	   ,
	VAR_DELTA_A		   ,
	VAR_DEPTH          ,
	VAR_DIAMOND		   ,
	VAR_DISC		   ,
	VAR_DISC2		   ,
	VAR_DISC3D		   ,
	VAR_ECLIPSE		   ,
	VAR_ECOLLIDE	   ,
	VAR_EDISC		   ,
	VAR_EJULIA		   ,
	VAR_ELLIPTIC	   ,
	VAR_EMOD		   ,
	VAR_EMOTION		   ,
	VAR_ENNEPERS	   ,
	VAR_EPISPIRAL	   ,
	VAR_EPUSH		   ,
	VAR_EROTATE		   ,
	VAR_ESCALE		   ,
	VAR_ESCHER		   ,
	VAR_ESTIQ          ,
	VAR_ESWIRL		   ,
	VAR_EX			   ,
	VAR_EXP			   ,
	VAR_EXPO		   ,
	VAR_EXPONENTIAL	   ,
	VAR_EXTRUDE		   ,
	VAR_EYEFISH		   ,
	VAR_FALLOFF 	   ,
	VAR_FALLOFF2	   ,
	VAR_FALLOFF3	   ,
	VAR_FAN			   ,
	VAR_FAN2		   ,
	VAR_FARBLUR        ,
	VAR_FDISC		   ,
	VAR_FIBONACCI	   ,
	VAR_FIBONACCI2	   ,
	VAR_FISHEYE		   ,
	VAR_FLATTEN		   ,
	VAR_FLIP_CIRCLE	   ,
	VAR_FLIP_Y		   ,
	VAR_FLOWER		   ,
	VAR_FLUX		   ,
	VAR_FOCI		   ,
	VAR_FOCI3D		   ,
	VAR_FOURTH         ,
	VAR_FUNNEL		   ,
	VAR_GAUSSIAN_BLUR  ,
	VAR_GDOFFS         ,
	VAR_GLYNNIA		   ,
	VAR_GLYNNSIM1	   ,
	VAR_GLYNNSIM2	   ,
	VAR_GLYNNSIM3	   ,
	VAR_GRIDOUT		   ,
	VAR_HANDKERCHIEF   ,
	VAR_HEART		   ,
	VAR_HEAT           ,
	VAR_HEMISPHERE	   ,
	VAR_HO  		   ,
	VAR_HOLE		   ,
	VAR_HORSESHOE	   ,
	VAR_HYPERBOLIC	   ,
	VAR_HYPERTILE	   ,
	VAR_HYPERTILE1	   ,
	VAR_HYPERTILE2	   ,
	VAR_HYPERTILE3D	   ,
	VAR_HYPERTILE3D1   ,
	VAR_HYPERTILE3D2   ,
	VAR_IDISC		   ,
	VAR_INTERFERENCE2  ,
	VAR_JULIA		   ,
	VAR_JULIA3D		   ,
	VAR_JULIA3DQ	   ,
	VAR_JULIA3DZ	   ,
	VAR_JULIAC         ,
	VAR_JULIAN		   ,
	VAR_JULIAN2		   ,
	VAR_JULIAN3DX      ,
	VAR_JULIANAB       ,
	VAR_JULIAQ		   ,
	VAR_JULIASCOPE	   ,
	VAR_KALEIDOSCOPE   ,
	VAR_LAZY_TRAVIS	   ,
	VAR_LAZYSUSAN	   ,
	VAR_LINE		   ,
	VAR_LINEAR		   ,
	VAR_LINEAR_T	   ,
	VAR_LINEAR_T3D	   ,
	//VAR_LINEAR_XZ	   ,
	//VAR_LINEAR_YZ	   ,
	VAR_LINEAR3D	   ,
	VAR_LISSAJOUS	   ,
	VAR_LOG			   ,
	VAR_LOQ			   ,
	VAR_LOONIE		   ,
	VAR_LOONIE3D	   ,
	VAR_MASK		   ,
	VAR_MCARPET		   ,
	VAR_MIRROR_X       ,
	VAR_MIRROR_Y       ,
	VAR_MIRROR_Z       ,
	VAR_MOBIQ          ,
	VAR_MOBIUS		   ,
	VAR_MOBIUS_STRIP   ,
	VAR_MOBIUSN		   ,
	VAR_MODULUS		   ,
	VAR_MURL		   ,
	VAR_MURL2		   ,
	VAR_NGON		   ,
	VAR_NOISE		   ,
	VAR_NPOLAR		   ,
	VAR_OCTAGON		   ,
	VAR_ORTHO		   ,
	VAR_OSCILLOSCOPE   ,
	VAR_OVOID		   ,
	VAR_OVOID3D		   ,
	VAR_PARABOLA	   ,
	VAR_PDJ			   ,
	VAR_PERSPECTIVE	   ,
	VAR_PETAL		   ,
	VAR_PHOENIX_JULIA  ,
	VAR_PIE			   ,
	VAR_PIE3D		   ,
	VAR_POINCARE	   ,
	VAR_POINCARE3D	   ,
	VAR_POLAR		   ,
	VAR_POLAR2		   ,
	VAR_POLYNOMIAL	   ,
	VAR_POPCORN		   ,
	VAR_POPCORN2	   ,
	VAR_POPCORN23D	   ,
	VAR_POW_BLOCK	   ,
	VAR_POWER		   ,
	VAR_PSPHERE		   ,
	VAR_Q_ODE          ,
	VAR_RADIAL_BLUR	   ,
	VAR_RATIONAL3	   ,
	VAR_RAYS		   ,
	VAR_RBLUR          ,
	VAR_RECTANGLES	   ,
	VAR_RINGS		   ,
	VAR_RINGS2		   ,
	VAR_RIPPLE		   ,
	VAR_RIPPLED		   ,
	VAR_ROTATE_X       ,
	VAR_ROTATE_Y       ,
	VAR_ROTATE_Z       ,
	VAR_ROUNDSPHER	   ,
	VAR_ROUNDSPHER3D   ,
	VAR_SCRY		   ,
	VAR_SCRY3D		   ,
	VAR_SEC			   ,
	VAR_SECANT2		   ,
	VAR_SECH		   ,
	VAR_SECHQ          ,
	VAR_SECQ           ,
	VAR_SEPARATION	   ,
	VAR_SHRED_RAD	   ,
	VAR_SHRED_LIN	   ,
	VAR_SIGMOID		   ,
	VAR_SIN			   ,
	VAR_SINEBLUR	   ,
	VAR_SINH		   ,
	VAR_SINHQ		   ,
	VAR_SINQ		   ,
	VAR_SINTRANGE      ,
	VAR_SINUS_GRID	   ,
	VAR_SINUSOIDAL	   ,
	VAR_SINUSOIDAL3D   ,
	VAR_SPHERICAL	   ,
	VAR_SPHERICAL3D	   ,
	VAR_SPHERICALN	   ,
	VAR_SPHERIVOID     ,
	VAR_SPHYP3D		   ,
	VAR_SPIRAL		   ,
	VAR_SPIRAL_WING	   ,
	VAR_SPIROGRAPH	   ,
	VAR_SPLIT		   ,
	VAR_SPLIT_BRDR     ,
	VAR_SPLITS		   ,
	VAR_SQUARE		   ,
	VAR_SQUARE3D	   ,
	VAR_SQUARIZE	   ,
	VAR_SQUIRREL	   ,
	VAR_SQUISH         ,
	VAR_SSCHECKS	   ,
	VAR_STARBLUR	   ,
	VAR_STRIPES		   ,
	VAR_STWIN		   ,
	VAR_SUPER_SHAPE	   ,
	VAR_SUPER_SHAPE3D  ,
	VAR_SVF			   ,
	VAR_SWIRL		   ,
	VAR_TAN			   ,
	VAR_TANCOS         ,
	VAR_TANGENT		   ,
	VAR_TANH		   ,
	VAR_TANHQ		   ,
	VAR_TANQ		   ,
	VAR_TARGET		   ,
	VAR_TAURUS		   ,
	VAR_TRADE		   ,
	VAR_TRUCHET        ,
	VAR_TWINTRIAN	   ,
	VAR_TWO_FACE	   ,
	VAR_UNPOLAR		   ,
	VAR_VORON          ,
	VAR_WAFFLE         ,
	VAR_WAVES		   ,
	VAR_WAVES2		   ,
	VAR_WAVES23D	   ,
	VAR_WAVESN		   ,
	VAR_WDISC		   ,
	VAR_WEDGE		   ,
	VAR_WEDGE_JULIA	   ,
	VAR_WEDGE_SPH	   ,
	VAR_WHORL		   ,
	VAR_XHEART		   ,
	VAR_XTRB		   ,
	VAR_ZBLUR		   ,
	VAR_ZCONE		   ,
	VAR_ZSCALE		   ,
	VAR_ZTRANSLATE     ,

	VAR_PRE_ARCH,
	VAR_PRE_AUGER,
	VAR_PRE_BARYCENTROID,
	VAR_PRE_BCIRCLE,
	VAR_PRE_BCOLLIDE,
	VAR_PRE_BENT,
	VAR_PRE_BENT2,
	VAR_PRE_BIPOLAR,
	VAR_PRE_BISPLIT,
	VAR_PRE_BLADE,
	VAR_PRE_BLADE3D,
	VAR_PRE_BLOB,
	VAR_PRE_BLOB2,
	VAR_PRE_BLOB3D,
	VAR_PRE_BLOCKY,
	VAR_PRE_BLUR,
	VAR_PRE_BLUR_CIRCLE,
	VAR_PRE_BLUR_HEART,
	VAR_PRE_BLUR_LINEAR,
	VAR_PRE_BLUR_PIXELIZE,
	VAR_PRE_BLUR_SQUARE,
	VAR_PRE_BLUR_ZOOM,
	VAR_PRE_BLUR3D,
	VAR_PRE_BMOD,
	VAR_PRE_BOARDERS,
	VAR_PRE_BOARDERS2,
	VAR_PRE_BSWIRL,
	VAR_PRE_BTRANSFORM,
	VAR_PRE_BUBBLE,
	VAR_PRE_BUBBLE2,
	VAR_PRE_BUTTERFLY,
	VAR_PRE_BWRAPS,
	VAR_PRE_CARDIOID,
	VAR_PRE_CELL,
	VAR_PRE_CHECKS,
	VAR_PRE_CIRCLEBLUR,
	VAR_PRE_CIRCLECROP,
	VAR_PRE_CIRCLELINEAR,
	VAR_PRE_CIRCLERAND,
	VAR_PRE_CIRCLETRANS1,
	VAR_PRE_CIRCLIZE,
	VAR_PRE_CIRCLIZE2,
	VAR_PRE_CIRCUS,
	VAR_PRE_COLLIDEOSCOPE,
	VAR_PRE_CONIC,
	VAR_PRE_COS,
	VAR_PRE_COS_WRAP,
	VAR_PRE_COSH,
	VAR_PRE_COSHQ,
	VAR_PRE_COSINE,
	VAR_PRE_COSQ,
	VAR_PRE_COT,
	VAR_PRE_COTH,
	VAR_PRE_COTHQ,
	VAR_PRE_COTQ,
	VAR_PRE_CPOW,
	VAR_PRE_CPOW2,
	VAR_PRE_CRESCENTS,
	VAR_PRE_CROP,
	VAR_PRE_CROPN,
	VAR_PRE_CROSS,
	VAR_PRE_CSC,
	VAR_PRE_CSCH,
	VAR_PRE_CSCHQ,
	VAR_PRE_CSCQ,
	VAR_PRE_CUBIC3D,
	VAR_PRE_CUBIC_LATTICE3D,
	VAR_PRE_CURL,
	VAR_PRE_CURL3D,
	VAR_PRE_CURL_SP,
	VAR_PRE_CURVATURE,
	VAR_PRE_CURVE,
	VAR_PRE_CYLINDER,
	VAR_PRE_DELTA_A,
	VAR_PRE_DEPTH,
	VAR_PRE_DIAMOND,
	VAR_PRE_DISC,
	VAR_PRE_DISC2,
	VAR_PRE_DISC3D,
	VAR_PRE_ECLIPSE,
	VAR_PRE_ECOLLIDE,
	VAR_PRE_EDISC,
	VAR_PRE_EJULIA,
	VAR_PRE_ELLIPTIC,
	VAR_PRE_EMOD,
	VAR_PRE_EMOTION,
	VAR_PRE_ENNEPERS,
	VAR_PRE_EPISPIRAL,
	VAR_PRE_EPUSH,
	VAR_PRE_EROTATE,
	VAR_PRE_ESCALE,
	VAR_PRE_ESCHER,
	VAR_PRE_ESTIQ,
	VAR_PRE_ESWIRL,
	VAR_PRE_EX,
	VAR_PRE_EXP,
	VAR_PRE_EXPO,
	VAR_PRE_EXPONENTIAL,
	VAR_PRE_EXTRUDE,
	VAR_PRE_EYEFISH,
	VAR_PRE_FALLOFF,
	VAR_PRE_FALLOFF2,
	VAR_PRE_FALLOFF3,
	VAR_PRE_FAN,
	VAR_PRE_FAN2,
	VAR_PRE_FARBLUR,
	VAR_PRE_FDISC,
	VAR_PRE_FIBONACCI,
	VAR_PRE_FIBONACCI2,
	VAR_PRE_FISHEYE,
	VAR_PRE_FLATTEN,
	VAR_PRE_FLIP_CIRCLE,
	VAR_PRE_FLIP_Y,
	VAR_PRE_FLOWER,
	VAR_PRE_FLUX,
	VAR_PRE_FOCI,
	VAR_PRE_FOCI3D,
	VAR_PRE_FOURTH,
	VAR_PRE_FUNNEL,
	VAR_PRE_GAUSSIAN_BLUR,
	VAR_PRE_GDOFFS,
	VAR_PRE_GLYNNIA,
	VAR_PRE_GLYNNSIM1,
	VAR_PRE_GLYNNSIM2,
	VAR_PRE_GLYNNSIM3,
	VAR_PRE_GRIDOUT,
	VAR_PRE_HANDKERCHIEF,
	VAR_PRE_HEART,
	VAR_PRE_HEAT,
	VAR_PRE_HEMISPHERE,
	VAR_PRE_HO,
	VAR_PRE_HOLE,
	VAR_PRE_HORSESHOE,
	VAR_PRE_HYPERBOLIC,
	VAR_PRE_HYPERTILE,
	VAR_PRE_HYPERTILE1,
	VAR_PRE_HYPERTILE2,
	VAR_PRE_HYPERTILE3D,
	VAR_PRE_HYPERTILE3D1,
	VAR_PRE_HYPERTILE3D2,
	VAR_PRE_IDISC,
	VAR_PRE_INTERFERENCE2,
	VAR_PRE_JULIA,
	VAR_PRE_JULIA3D,
	VAR_PRE_JULIA3DQ,
	VAR_PRE_JULIA3DZ,
	VAR_PRE_JULIAC,
	VAR_PRE_JULIAN,
	VAR_PRE_JULIAN2,
	VAR_PRE_JULIAN3DX,
	VAR_PRE_JULIANAB,
	VAR_PRE_JULIAQ,
	VAR_PRE_JULIASCOPE,
	VAR_PRE_KALEIDOSCOPE,
	VAR_PRE_LAZY_TRAVIS,
	VAR_PRE_LAZYSUSAN,
	VAR_PRE_LINE,
	VAR_PRE_LINEAR,
	VAR_PRE_LINEAR_T,
	VAR_PRE_LINEAR_T3D,
	//VAR_PRE_LINEAR_XZ,
	//VAR_PRE_LINEAR_YZ,
	VAR_PRE_LINEAR3D,
	VAR_PRE_LISSAJOUS,
	VAR_PRE_LOG,
	VAR_PRE_LOQ,
	VAR_PRE_LOONIE,
	VAR_PRE_LOONIE3D,
	VAR_PRE_MASK,
	VAR_PRE_MCARPET,
	VAR_PRE_MIRROR_X,
	VAR_PRE_MIRROR_Y,
	VAR_PRE_MIRROR_Z,
	VAR_PRE_MOBIQ,
	VAR_PRE_MOBIUS,
	VAR_PRE_MOBIUS_STRIP,
	VAR_PRE_MOBIUSN,
	VAR_PRE_MODULUS,
	VAR_PRE_MURL,
	VAR_PRE_MURL2,
	VAR_PRE_NGON,
	VAR_PRE_NOISE,
	VAR_PRE_NPOLAR,
	VAR_PRE_OCTAGON,
	VAR_PRE_ORTHO,
	VAR_PRE_OSCILLOSCOPE,
	VAR_PRE_OVOID,
	VAR_PRE_OVOID3D,
	VAR_PRE_PARABOLA,
	VAR_PRE_PDJ,
	VAR_PRE_PERSPECTIVE,
	VAR_PRE_PETAL,
	VAR_PRE_PHOENIX_JULIA,
	VAR_PRE_PIE,
	VAR_PRE_PIE3D,
	VAR_PRE_POINCARE,
	VAR_PRE_POINCARE3D,
	VAR_PRE_POLAR,
	VAR_PRE_POLAR2,
	VAR_PRE_POLYNOMIAL,
	VAR_PRE_POPCORN,
	VAR_PRE_POPCORN2,
	VAR_PRE_POPCORN23D,
	VAR_PRE_POW_BLOCK,
	VAR_PRE_POWER,
	VAR_PRE_PSPHERE,
	VAR_PRE_Q_ODE,
	VAR_PRE_RADIAL_BLUR,
	VAR_PRE_RATIONAL3,
	VAR_PRE_RAYS,
	VAR_PRE_RBLUR,
	VAR_PRE_RECTANGLES,
	VAR_PRE_RINGS,
	VAR_PRE_RINGS2,
	VAR_PRE_RIPPLE,
	VAR_PRE_RIPPLED,
	VAR_PRE_ROTATE_X,
	VAR_PRE_ROTATE_Y,
	VAR_PRE_ROTATE_Z,
	VAR_PRE_ROUNDSPHER,
	VAR_PRE_ROUNDSPHER3D,
	VAR_PRE_SCRY,
	VAR_PRE_SCRY3D,
	VAR_PRE_SEC,
	VAR_PRE_SECANT2,
	VAR_PRE_SECH,
	VAR_PRE_SECHQ,
	VAR_PRE_SECQ,
	VAR_PRE_SEPARATION,
	VAR_PRE_SHRED_RAD,
	VAR_PRE_SHRED_LIN,
	VAR_PRE_SIGMOID,
	VAR_PRE_SIN,
	VAR_PRE_SINEBLUR,
	VAR_PRE_SINH,
	VAR_PRE_SINHQ,
	VAR_PRE_SINQ,
	VAR_PRE_SINTRANGE,
	VAR_PRE_SINUS_GRID,
	VAR_PRE_SINUSOIDAL,
	VAR_PRE_SINUSOIDAL3D,
	VAR_PRE_SPHERICAL,
	VAR_PRE_SPHERICAL3D,
	VAR_PRE_SPHERICALN,
	VAR_PRE_SPHERIVOID,
	VAR_PRE_SPHYP3D,
	VAR_PRE_SPIRAL,
	VAR_PRE_SPIRAL_WING,
	VAR_PRE_SPIROGRAPH,
	VAR_PRE_SPLIT,
	VAR_PRE_SPLIT_BRDR,
	VAR_PRE_SPLITS,
	VAR_PRE_SQUARE,
	VAR_PRE_SQUARE3D,
	VAR_PRE_SQUARIZE,
	VAR_PRE_SQUIRREL,
	VAR_PRE_SQUISH,
	VAR_PRE_SSCHECKS,
	VAR_PRE_STARBLUR,
	VAR_PRE_STRIPES,
	VAR_PRE_STWIN,
	VAR_PRE_SUPER_SHAPE,
	VAR_PRE_SUPER_SHAPE3D,
	VAR_PRE_SVF,
	VAR_PRE_SWIRL,
	VAR_PRE_TAN,
	VAR_PRE_TANCOS,
	VAR_PRE_TANGENT,
	VAR_PRE_TANH,
	VAR_PRE_TANHQ,
	VAR_PRE_TANQ,
	VAR_PRE_TARGET,
	VAR_PRE_TAURUS,
	VAR_PRE_TRADE,
	VAR_PRE_TRUCHET,
	VAR_PRE_TWINTRIAN,
	VAR_PRE_TWO_FACE,
	VAR_PRE_UNPOLAR,
	VAR_PRE_VORON,
	VAR_PRE_WAFFLE,
	VAR_PRE_WAVES,
	VAR_PRE_WAVES2,
	VAR_PRE_WAVES23D,
	VAR_PRE_WAVESN,
	VAR_PRE_WDISC,
	VAR_PRE_WEDGE,
	VAR_PRE_WEDGE_JULIA,
	VAR_PRE_WEDGE_SPH,
	VAR_PRE_WHORL,
	VAR_PRE_XHEART,
	VAR_PRE_XTRB,
	VAR_PRE_ZBLUR,
	VAR_PRE_ZCONE,
	VAR_PRE_ZSCALE,
	VAR_PRE_ZTRANSLATE,

	VAR_POST_ARCH,
	VAR_POST_AUGER,
	VAR_POST_BARYCENTROID,
	VAR_POST_BCIRCLE,
	VAR_POST_BCOLLIDE,
	VAR_POST_BENT,
	VAR_POST_BENT2,
	VAR_POST_BIPOLAR,
	VAR_POST_BISPLIT,
	VAR_POST_BLADE,
	VAR_POST_BLADE3D,
	VAR_POST_BLOB,
	VAR_POST_BLOB2,
	VAR_POST_BLOB3D,
	VAR_POST_BLOCKY,
	VAR_POST_BLUR,
	VAR_POST_BLUR_CIRCLE,
	VAR_POST_BLUR_HEART,
	VAR_POST_BLUR_LINEAR,
	VAR_POST_BLUR_PIXELIZE,
	VAR_POST_BLUR_SQUARE,
	VAR_POST_BLUR_ZOOM,
	VAR_POST_BLUR3D,
	VAR_POST_BMOD,
	VAR_POST_BOARDERS,
	VAR_POST_BOARDERS2,
	VAR_POST_BSWIRL,
	VAR_POST_BTRANSFORM,
	VAR_POST_BUBBLE,
	VAR_POST_BUBBLE2,
	VAR_POST_BUTTERFLY,
	VAR_POST_BWRAPS,
	VAR_POST_CARDIOID,
	VAR_POST_CELL,
	VAR_POST_CHECKS,
	VAR_POST_CIRCLEBLUR,
	VAR_POST_CIRCLECROP,
	VAR_POST_CIRCLELINEAR,
	VAR_POST_CIRCLERAND,
	VAR_POST_CIRCLETRANS1,
	VAR_POST_CIRCLIZE,
	VAR_POST_CIRCLIZE2,
	VAR_POST_CIRCUS,
	VAR_POST_COLLIDEOSCOPE,
	VAR_POST_CONIC,
	VAR_POST_COS,
	VAR_POST_COS_WRAP,
	VAR_POST_COSH,
	VAR_POST_COSHQ,
	VAR_POST_COSINE,
	VAR_POST_COSQ,
	VAR_POST_COT,
	VAR_POST_COTH,
	VAR_POST_COTHQ,
	VAR_POST_COTQ,
	VAR_POST_CPOW,
	VAR_POST_CPOW2,
	VAR_POST_CRESCENTS,
	VAR_POST_CROP,
	VAR_POST_CROPN,
	VAR_POST_CROSS,
	VAR_POST_CSC,
	VAR_POST_CSCH,
	VAR_POST_CSCHQ,
	VAR_POST_CSCQ,
	VAR_POST_CUBIC3D,
	VAR_POST_CUBIC_LATTICE3D,
	VAR_POST_CURL,
	VAR_POST_CURL3D,
	VAR_POST_CURL_SP,
	VAR_POST_CURVATURE,
	VAR_POST_CURVE,
	VAR_POST_CYLINDER,
	VAR_POST_DELTA_A,
	VAR_POST_DEPTH,
	VAR_POST_DIAMOND,
	VAR_POST_DISC,
	VAR_POST_DISC2,
	VAR_POST_DISC3D,
	VAR_POST_ECLIPSE,
	VAR_POST_ECOLLIDE,
	VAR_POST_EDISC,
	VAR_POST_EJULIA,
	VAR_POST_ELLIPTIC,
	VAR_POST_EMOD,
	VAR_POST_EMOTION,
	VAR_POST_ENNEPERS,
	VAR_POST_EPISPIRAL,
	VAR_POST_EPUSH,
	VAR_POST_EROTATE,
	VAR_POST_ESCALE,
	VAR_POST_ESCHER,
	VAR_POST_ESTIQ,
	VAR_POST_ESWIRL,
	VAR_POST_EX,
	VAR_POST_EXP,
	VAR_POST_EXPO,
	VAR_POST_EXPONENTIAL,
	VAR_POST_EXTRUDE,
	VAR_POST_EYEFISH,
	VAR_POST_FALLOFF,
	VAR_POST_FALLOFF2,
	VAR_POST_FALLOFF3,
	VAR_POST_FAN,
	VAR_POST_FAN2,
	VAR_POST_FARBLUR,
	VAR_POST_FDISC,
	VAR_POST_FIBONACCI,
	VAR_POST_FIBONACCI2,
	VAR_POST_FISHEYE,
	VAR_POST_FLATTEN,
	VAR_POST_FLIP_CIRCLE,
	VAR_POST_FLIP_Y,
	VAR_POST_FLOWER,
	VAR_POST_FLUX,
	VAR_POST_FOCI,
	VAR_POST_FOCI3D,
	VAR_POST_FOURTH,
	VAR_POST_FUNNEL,
	VAR_POST_GAUSSIAN_BLUR,
	VAR_POST_GDOFFS,
	VAR_POST_GLYNNIA,
	VAR_POST_GLYNNSIM1,
	VAR_POST_GLYNNSIM2,
	VAR_POST_GLYNNSIM3,
	VAR_POST_GRIDOUT,
	VAR_POST_HANDKERCHIEF,
	VAR_POST_HEART,
	VAR_POST_HEAT,
	VAR_POST_HEMISPHERE,
	VAR_POST_HO,
	VAR_POST_HOLE,
	VAR_POST_HORSESHOE,
	VAR_POST_HYPERBOLIC,
	VAR_POST_HYPERTILE,
	VAR_POST_HYPERTILE1,
	VAR_POST_HYPERTILE2,
	VAR_POST_HYPERTILE3D,
	VAR_POST_HYPERTILE3D1,
	VAR_POST_HYPERTILE3D2,
	VAR_POST_IDISC,
	VAR_POST_INTERFERENCE2,
	VAR_POST_JULIA,
	VAR_POST_JULIA3D,
	VAR_POST_JULIA3DQ,
	VAR_POST_JULIA3DZ,
	VAR_POST_JULIAC,
	VAR_POST_JULIAN,
	VAR_POST_JULIAN2,
	VAR_POST_JULIAN3DX,
	VAR_POST_JULIANAB,
	VAR_POST_JULIAQ,
	VAR_POST_JULIASCOPE,
	VAR_POST_KALEIDOSCOPE,
	VAR_POST_LAZY_TRAVIS,
	VAR_POST_LAZYSUSAN,
	VAR_POST_LINE,
	VAR_POST_LINEAR,
	VAR_POST_LINEAR_T,
	VAR_POST_LINEAR_T3D,
	//VAR_POST_LINEAR_XZ,
	//VAR_POST_LINEAR_YZ,
	VAR_POST_LINEAR3D,
	VAR_POST_LISSAJOUS,
	VAR_POST_LOG,
	VAR_POST_LOQ,
	VAR_POST_LOONIE,
	VAR_POST_LOONIE3D,
	VAR_POST_MASK,
	VAR_POST_MCARPET,
	VAR_POST_MIRROR_X,
	VAR_POST_MIRROR_Y,
	VAR_POST_MIRROR_Z,
	VAR_POST_MOBIQ,
	VAR_POST_MOBIUS,
	VAR_POST_MOBIUS_STRIP,
	VAR_POST_MOBIUSN,
	VAR_POST_MODULUS,
	VAR_POST_MURL,
	VAR_POST_MURL2,
	VAR_POST_NGON,
	VAR_POST_NOISE,
	VAR_POST_NPOLAR,
	VAR_POST_OCTAGON,
	VAR_POST_ORTHO,
	VAR_POST_OSCILLOSCOPE,
	VAR_POST_OVOID,
	VAR_POST_OVOID3D,
	VAR_POST_PARABOLA,
	VAR_POST_PDJ,
	VAR_POST_PERSPECTIVE,
	VAR_POST_PETAL,
	VAR_POST_PHOENIX_JULIA,
	VAR_POST_PIE,
	VAR_POST_PIE3D,
	VAR_POST_POINCARE,
	VAR_POST_POINCARE3D,
	VAR_POST_POLAR,
	VAR_POST_POLAR2,
	VAR_POST_POLYNOMIAL,
	VAR_POST_POPCORN,
	VAR_POST_POPCORN2,
	VAR_POST_POPCORN23D,
	VAR_POST_POW_BLOCK,
	VAR_POST_POWER,
	VAR_POST_PSPHERE,
	VAR_POST_Q_ODE,
	VAR_POST_RADIAL_BLUR,
	VAR_POST_RATIONAL3,
	VAR_POST_RAYS,
	VAR_POST_RBLUR,
	VAR_POST_RECTANGLES,
	VAR_POST_RINGS,
	VAR_POST_RINGS2,
	VAR_POST_RIPPLE,
	VAR_POST_RIPPLED,
	VAR_POST_ROTATE_X,
	VAR_POST_ROTATE_Y,
	VAR_POST_ROTATE_Z,
	VAR_POST_ROUNDSPHER,
	VAR_POST_ROUNDSPHER3D,
	VAR_POST_SCRY,
	VAR_POST_SCRY3D,
	VAR_POST_SEC,
	VAR_POST_SECANT2,
	VAR_POST_SECH,
	VAR_POST_SECHQ,
	VAR_POST_SECQ,
	VAR_POST_SEPARATION,
	VAR_POST_SHRED_RAD,
	VAR_POST_SHRED_LIN,
	VAR_POST_SIGMOID,
	VAR_POST_SIN,
	VAR_POST_SINEBLUR,
	VAR_POST_SINH,
	VAR_POST_SINHQ,
	VAR_POST_SINQ,
	VAR_POST_SINTRANGE,
	VAR_POST_SINUS_GRID,
	VAR_POST_SINUSOIDAL,
	VAR_POST_SINUSOIDAL3D,
	VAR_POST_SPHERICAL,
	VAR_POST_SPHERICAL3D,
	VAR_POST_SPHERICALN,
	VAR_POST_SPHERIVOID,
	VAR_POST_SPHYP3D,
	VAR_POST_SPIRAL,
	VAR_POST_SPIRAL_WING,
	VAR_POST_SPIROGRAPH,
	VAR_POST_SPLIT,
	VAR_POST_SPLIT_BRDR,
	VAR_POST_SPLITS,
	VAR_POST_SQUARE,
	VAR_POST_SQUARE3D,
	VAR_POST_SQUARIZE,
	VAR_POST_SQUIRREL,
	VAR_POST_SQUISH,
	VAR_POST_SSCHECKS,
	VAR_POST_STARBLUR,
	VAR_POST_STRIPES,
	VAR_POST_STWIN,
	VAR_POST_SUPER_SHAPE,
	VAR_POST_SUPER_SHAPE3D,
	VAR_POST_SVF,
	VAR_POST_SWIRL,
	VAR_POST_TAN,
	VAR_POST_TANCOS,
	VAR_POST_TANGENT,
	VAR_POST_TANH,
	VAR_POST_TANHQ,
	VAR_POST_TANQ,
	VAR_POST_TARGET,
	VAR_POST_TAURUS,
	VAR_POST_TRADE,
	VAR_POST_TRUCHET,
	VAR_POST_TWINTRIAN,
	VAR_POST_TWO_FACE,
	VAR_POST_UNPOLAR,
	VAR_POST_VORON,
	VAR_POST_WAFFLE,
	VAR_POST_WAVES,
	VAR_POST_WAVES2,
	VAR_POST_WAVES23D,
	VAR_POST_WAVESN,
	VAR_POST_WDISC,
	VAR_POST_WEDGE,
	VAR_POST_WEDGE_JULIA,
	VAR_POST_WEDGE_SPH,
	VAR_POST_WHORL,
	VAR_POST_XHEART,
	VAR_POST_XTRB,
	VAR_POST_ZBLUR,
	VAR_POST_ZCONE,
	VAR_POST_ZSCALE,
	VAR_POST_ZTRANSLATE,

	//Direct color are special and only some have pre/post counterparts.
	VAR_DC_BUBBLE,
	VAR_DC_CARPET,
	VAR_DC_CUBE,
	VAR_DC_CYLINDER,
	VAR_DC_GRIDOUT,
	VAR_DC_LINEAR,
	VAR_DC_TRIANGLE,
	VAR_DC_ZTRANSL,

	VAR_PRE_DC_CARPET,
	VAR_PRE_DC_CUBE,
	VAR_PRE_DC_GRIDOUT,
	VAR_PRE_DC_TRIANGLE,
	VAR_PRE_DC_ZTRANSL,

	VAR_POST_DC_CARPET,
	VAR_POST_DC_CUBE,
	VAR_POST_DC_GRIDOUT,
	VAR_POST_DC_TRIANGLE,
	VAR_POST_DC_ZTRANSL,

	LAST_VAR = VAR_POST_DC_ZTRANSL + 1
};

/// <summary>
/// Translated and precalculated values that get passed to each variation's virtual function.
/// Note that this must be passed in and not a member because multiple threads will be calling
/// the variation functions simultaneously. Each thread will get its own IteratorHelper object.
/// Template argument expected to be float or double.
/// </summary>
template <typename T>
class EMBER_API IteratorHelper
{
public:
	v2T m_Color;
	T m_TransX, m_TransY, m_TransZ;//Translated point gotten by applying the affine transform to the input point gotten from the output of the previous iteration (excluding final).
	T m_PrecalcSumSquares;//Precalculated value of the sum of the squares of the translated point.
	T m_PrecalcSqrtSumSquares;//Precalculated value of the square root of m_PrecalcSumSquares.
	T m_PrecalcSina;//Precalculated value of m_TransX / m_PrecalcSqrtSumSquares.
	T m_PrecalcCosa;//Precalculated value of m_TransY / m_PrecalcSqrtSumSquares.
	T m_PrecalcAtanxy;//Precalculated value of atan2(m_TransX, m_TransY).
	T m_PrecalcAtanyx;//Precalculated value of atan2(m_TransY, m_TransX).
	v4T In, Out;
};

/// <summary>
/// The base variation class from which all variations will derive.
/// Each has a unique ID, name and weight, as well as a virtual function Func() which
/// does the actual calculations.
/// Each also has boolean values that specify whether precalculations are needed.
/// These precalc flags are used by the parent Xform to determine which values to
/// precalculate in each iteration.
/// Template argument expected to be float or double.
/// </summary>
template <class T>
class EMBER_API Variation
{
public:
	/// <summary>
	/// Constructor which takes parameters.
	/// </summary>
	/// <param name="name">The unique name of the variation</param>
	/// <param name="id">The unique ID of the variation</param>
	/// <param name="weight">The weight. Default: 1.</param>
	/// <param name="needPrecalcSumSquares">Whether it uses the precalc sum squares value in its calculations. Default: false.</param>
	/// <param name="needPrecalcSqrtSumSquares">Whether it uses the sqrt precalc sum squares value in its calculations. Default: false.</param>
	/// <param name="needPrecalcAngles">Whether it uses the precalc sin and cos values in its calculations. Default: false.</param>
	/// <param name="needPrecalcAtanXY">Whether it uses the precalc atan XY value in its calculations. Default: false.</param>
	/// <param name="needPrecalcAtanYX">Whether it uses the precalc atan YX value in its calculations. Default: false.</param>
	Variation(const char* name, eVariationId id, T weight = 1.0,
			  bool needPrecalcSumSquares = false,
			  bool needPrecalcSqrtSumSquares = false,
			  bool needPrecalcAngles = false,
			  bool needPrecalcAtanXY = false,
			  bool needPrecalcAtanYX = false)
			  : m_Name(name)//Omit unnecessary default constructor call.
	{
		m_Xform = nullptr;
		m_VariationId = id;
		m_Weight = weight;
		m_NeedPrecalcSumSquares = needPrecalcSumSquares;
		m_NeedPrecalcSqrtSumSquares = needPrecalcSqrtSumSquares;
		m_NeedPrecalcAngles = needPrecalcAngles;
		m_NeedPrecalcAtanXY = needPrecalcAtanXY;
		m_NeedPrecalcAtanYX = needPrecalcAtanYX;

		//Make absolutely sure that flag logic makes sense.
		if (m_NeedPrecalcSqrtSumSquares)
			m_NeedPrecalcSumSquares = true;

		if (m_NeedPrecalcAngles)
		{
			m_NeedPrecalcSumSquares = true;
			m_NeedPrecalcSqrtSumSquares = true;
		}

		m_AssignType = ASSIGNTYPE_SET;
		SetType();
	}

	/// <summary>
	/// Default copy constructor.
	/// </summary>
	/// <param name="variation">The Variation object to copy</param>
	Variation(const Variation<T>& variation)
	{
		Variation<T>::operator=<T>(variation);
	}

	/// <summary>
	/// Copy constructor to copy a Variation object of type U.
	/// </summary>
	/// <param name="variation">The Variation object to copy</param>
	template <typename U>
	Variation(const Variation<U>& variation)
	{
		Variation<T>::operator=<U>(variation);
	}

	/// <summary>
	/// Empty virtual destructor.
	/// Note that even though this is empty, it must be present
	/// and be virtual for the derived classes to properly get destroyed.
	/// </summary>
	virtual ~Variation()
	{
	}

	/// <summary>
	/// Default assignment operator.
	/// </summary>
	/// <param name="variation">The Variation object to copy</param>
	Variation<T>& operator = (const Variation<T>& variation)
	{
		if (this != &variation)
			Variation<T>::operator=<T>(variation);

		return *this;
	}

	/// <summary>
	/// Assignment operator to assign a Variation object of type U.
	/// </summary>
	/// <param name="variation">The Variation object to copy.</param>
	/// <returns>Reference to updated self</returns>
	template <typename U>
	Variation<T>& operator = (const Variation<U>& variation)
	{
		m_Name = variation.Name();
		m_VarType = variation.VarType();
		m_AssignType = variation.AssignType();
		m_VariationId = variation.VariationId();
		m_Weight = T(variation.m_Weight);
		m_Xform = typeid(T) == typeid(U) ? (Xform<T>*)variation.ParentXform() : nullptr;
		m_NeedPrecalcSumSquares = variation.NeedPrecalcSumSquares();
		m_NeedPrecalcSqrtSumSquares = variation.NeedPrecalcSqrtSumSquares();
		m_NeedPrecalcAngles = variation.NeedPrecalcAngles();
		m_NeedPrecalcAtanXY = variation.NeedPrecalcAtanXY();
		m_NeedPrecalcAtanYX = variation.NeedPrecalcAtanYX();

		return *this;
	}

	/// <summary>
	/// Per-variation precalc used for pre and post variations.
	/// </summary>
	/// <param name="iteratorHelper">The helper to read values from in the case of pre, and store precalc values to in both cases.</param>
	/// <param name="point">The point to read values from in the case of post, ignored for pre.</param>
	void PrecalcHelper(IteratorHelper<T>& iteratorHelper, Point<T>* point)
	{
		if (m_VarType == VARTYPE_PRE)
		{
			if (m_NeedPrecalcSumSquares)
			{
				iteratorHelper.m_PrecalcSumSquares = SQR(iteratorHelper.m_TransX) + SQR(iteratorHelper.m_TransY);

				if (m_NeedPrecalcSqrtSumSquares)
				{
					iteratorHelper.m_PrecalcSqrtSumSquares = sqrt(iteratorHelper.m_PrecalcSumSquares);

					if (m_NeedPrecalcAngles)
					{
						iteratorHelper.m_PrecalcSina = iteratorHelper.m_TransX / iteratorHelper.m_PrecalcSqrtSumSquares;
						iteratorHelper.m_PrecalcCosa = iteratorHelper.m_TransY / iteratorHelper.m_PrecalcSqrtSumSquares;
					}
				}
			}

			if (m_NeedPrecalcAtanXY)
				iteratorHelper.m_PrecalcAtanxy = atan2(iteratorHelper.m_TransX, iteratorHelper.m_TransY);

			if (m_NeedPrecalcAtanYX)
				iteratorHelper.m_PrecalcAtanyx = atan2(iteratorHelper.m_TransY, iteratorHelper.m_TransX);
		}
		else if (m_VarType == VARTYPE_POST)
		{
			if (m_NeedPrecalcSumSquares)
			{
				iteratorHelper.m_PrecalcSumSquares = SQR(point->m_X) + SQR(point->m_Y);

				if (m_NeedPrecalcSqrtSumSquares)
				{
					iteratorHelper.m_PrecalcSqrtSumSquares = sqrt(iteratorHelper.m_PrecalcSumSquares);

					if (m_NeedPrecalcAngles)
					{
						iteratorHelper.m_PrecalcSina = point->m_X / iteratorHelper.m_PrecalcSqrtSumSquares;
						iteratorHelper.m_PrecalcCosa = point->m_Y / iteratorHelper.m_PrecalcSqrtSumSquares;
					}
				}
			}

			if (m_NeedPrecalcAtanXY)
				iteratorHelper.m_PrecalcAtanxy = atan2(point->m_X, point->m_Y);

			if (m_NeedPrecalcAtanYX)
				iteratorHelper.m_PrecalcAtanyx = atan2(point->m_Y, point->m_X);
		}
	}

	/// <summary>
	/// Per-variation precalc OpenCL string used for pre and post variations.
	/// </summary>
	/// <returns>The per-variation OpenCL precalc string</returns>
	string PrecalcOpenCLString()
	{
		ostringstream ss;

		if (m_VarType == VARTYPE_PRE)
		{
			if (m_NeedPrecalcSumSquares)
			{
				ss << "\tprecalcSumSquares = SQR(transX) + SQR(transY);\n";

				if (m_NeedPrecalcSqrtSumSquares)
				{
					ss << "\tprecalcSqrtSumSquares = sqrt(precalcSumSquares);\n";

					if (m_NeedPrecalcAngles)
					{
						ss << "\tprecalcSina = transX / precalcSqrtSumSquares;\n";
						ss << "\tprecalcCosa = transY / precalcSqrtSumSquares;\n";
					}
				}
			}

			if (m_NeedPrecalcAtanXY)
				ss << "\tprecalcAtanxy = atan2(transX, transY);\n";

			if (m_NeedPrecalcAtanYX)
				ss << "\tprecalcAtanyx = atan2(transY, transX);\n";
		}
		else if (m_VarType == VARTYPE_POST)
		{
			if (m_NeedPrecalcSumSquares)
			{
				ss << "\tprecalcSumSquares = SQR(outPoint->m_X) + SQR(outPoint->m_Y);\n";

				if (m_NeedPrecalcSqrtSumSquares)
				{
					ss << "\tprecalcSqrtSumSquares = sqrt(precalcSumSquares);\n";

					if (m_NeedPrecalcAngles)
					{
						ss << "\tprecalcSina = outPoint->m_X / precalcSqrtSumSquares;\n";
						ss << "\tprecalcCosa = outPoint->m_Y / precalcSqrtSumSquares;\n";
					}
				}
			}

			if (m_NeedPrecalcAtanXY)
				ss << "\tprecalcAtanxy = atan2(outPoint->m_X, outPoint->m_Y);\n";

			if (m_NeedPrecalcAtanYX)
				ss << "\tprecalcAtanyx = atan2(outPoint->m_Y, outPoint->m_X);\n";
		}

		if (NeedAnyPrecalc())
			ss << "\n";

		return ss.str();
	}

	/// <summary>
	/// Return the name and weight of the variation as a string.
	/// </summary>
	/// <returns>The name and weight of the variation</returns>
	virtual string ToString() const
	{
		ostringstream ss;

		ss << m_Name << "(" << m_Weight << ")";

		return ss.str();
	}

	/// <summary>
	/// Abstract copy function. Derived classes must implement.
	/// </summary>
	/// <returns>A copy of this object</returns>
	virtual Variation<T>* Copy() = 0;

	/// <summary>
	/// Create a new Variation<float>, store it in the pointer reference passed in and
	/// copy the this Variation's values into it.
	/// Note this is a severe hack to overcome two shortcomings in C++.
	/// One is that templated functions cannot be virtual.
	/// The second is that function overloading only works when parameters differ, not just return types.
	/// In an ideal world, all copy functionality would be consolidated into a single function that looked like:
	/// template <typename U> virtual Variation<U> Copy();
	/// Since that isn't possible, the only way to do what's needed is to create two functions to do this, one for
	/// Variation<float> and another for Variation<double>.
	/// This further offends design sensiblities since it requires this template class to know which types it's going to
	/// be instantiated for. Sadly, there is no alternative and it must be done this way. Fortunately, we know it will
	/// only ever be used with float and double.
	/// </summary>
	/// <param name="var">A reference to a pointer which will store the newly created Variation<float>*</param>
	virtual void Copy(Variation<float>*& var) const = 0;

#ifdef DO_DOUBLE
	/// <summary>
	/// See description for Copy(Variation<float>*& var).
	/// </summary>
	/// <param name="var">A reference to a pointer which will store the newly created Variation<double>*</param>
	virtual void Copy(Variation<double>*& var) const = 0;
#endif

	/// <summary>
	/// Abstract function where the actual work takes place. Derived classes must implement.
	/// </summary>
	/// <param name="helper">The IteratorHelper object which holds translated and precalculated values</param>
	/// <param name="outPoint">The point to store the result in</param>
	/// <param name="rand">The random number generator to use.</param>
	virtual void Func(IteratorHelper<T>& helper, Point<T>& outPoint, QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) = 0;

	/// <summary>
	/// Return a string which performs the equivalent calculation in Func(), but on the GPU in OpenCL.
	/// Derived classes will implement this.
	/// </summary>
	/// <returns>The OpenCL string to perform the equivalent calculation on the GPU in OpenCL</returns>
	virtual string OpenCLString() { return ""; }

	/// <summary>
	/// If the OpenCL string depends on any functions specific to this variation, return them.
	/// </summary>
	/// <returns>The OpenCL string for functions specific to this variation</returns>
	virtual string OpenCLFuncsString() { return ""; }

	/// <summary>
	/// In addition to the standard precalculation stored in the IteratorHelper object, some
	/// variations have additional precalculation work to do that can save processing time while iterating.
	/// For most this is left empty, however a few will override.
	/// </summary>
	virtual void Precalc() { }

	/// <summary>
	/// When creating random embers, the variations are placed in a random state.
	/// For most this base implementation will be used, however a few will override.
	/// </summary>
	/// <param name="rand">The rand.</param>
	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand)
	{
		m_Weight = rand.Frand11<T>();
	}

	/// <summary>
	/// Returns the string prefix to be used with params and the variation name.
	/// </summary>
	/// <returns>pre_, post_ or the empty string</returns>
	string Prefix() const
	{
		if (m_VarType == VARTYPE_PRE)
			return "pre_";
		else if (m_VarType == VARTYPE_POST)
			return "post_";
		else
			return "";
	}

	string BaseName() const
	{
		string prefix = Prefix();

		if (prefix != "" && m_Name.find(prefix) == 0)
			return m_Name.substr(prefix.size(), m_Name.size() - prefix.size());
		else
			return m_Name;
	}

	/// <summary>
	/// Accessors.
	/// </summary>
	inline bool NeedPrecalcSumSquares()     const { return m_NeedPrecalcSumSquares; }
	inline bool NeedPrecalcSqrtSumSquares() const { return m_NeedPrecalcSqrtSumSquares; }
	inline bool NeedPrecalcAngles()         const { return m_NeedPrecalcAngles; }
	inline bool NeedPrecalcAtanXY()         const { return m_NeedPrecalcAtanXY; }
	inline bool NeedPrecalcAtanYX()         const { return m_NeedPrecalcAtanYX; }
	inline bool NeedAnyPrecalc()            const { return NeedPrecalcSumSquares() || NeedPrecalcSqrtSumSquares() || NeedPrecalcAngles() || NeedPrecalcAtanXY() || NeedPrecalcAtanYX(); }
	eVariationId VariationId() const { return m_VariationId; }
	string Name() const { return m_Name; }
	eVariationType VarType() const { return m_VarType; }
	eVariationAssignType AssignType() const { return m_AssignType; }
	const Xform<T>* ParentXform() const { return m_Xform; }
	void ParentXform(Xform<T>* xform) { m_Xform = xform; }
	int IndexInXform() { return m_Xform ? m_Xform->GetVariationIndex(this) : -1; }
	int XformIndexInEmber() { return m_Xform ? m_Xform->IndexInParentEmber() : -1; }

	T m_Weight;//The weight of the variation.

protected:
	void SetType()
	{
		if (m_Name.find("pre_") == 0)
			m_VarType = VARTYPE_PRE;
		else if (m_Name.find("post_") == 0)
			m_VarType = VARTYPE_POST;
		else
			m_VarType = VARTYPE_REG;
	}

	Xform<T>* m_Xform;//The parent Xform that this variation is a child of.
	eVariationId m_VariationId;//The unique ID of this variation.
	string m_Name;//The unique name of this variation.
	eVariationType m_VarType;//The type of variation: regular, pre or post.
	eVariationAssignType m_AssignType;//Whether to assign the results for pre/post, or sum them.

private:
	bool m_NeedPrecalcSumSquares;//Whether this variation uses the precalc sum squares value in its calculations.
	bool m_NeedPrecalcSqrtSumSquares;//Whether it uses the sqrt precalc sum squares value in its calculations.
	bool m_NeedPrecalcAngles;//Whether it uses the precalc sin and cos values in its calculations.
	bool m_NeedPrecalcAtanXY;//Whether it uses the precalc atan XY value in its calculations.
	bool m_NeedPrecalcAtanYX;//Whether it uses the precalc atan YX value in its calculations.
};

/// <summary>
/// The type of parameter represented by ParamWithName<T>.
/// This allows restricting of certain parameters to sensible values.
/// </summary>
enum eParamType
{
	REAL,
	REAL_CYCLIC,
	REAL_NONZERO,
	INTEGER,
	INTEGER_NONZERO
};

template <typename T> class ParametricVariation;

/// <summary>
/// Parametric variations use parameters in addition to weight.
/// These values are stored in members of classes derived from ParametricVariation,
/// however for easy access, pointers to them are also stored in a vector
/// of ParamWithName in ParametricVariation.
/// Each of these takes the form of a name string and a pointer to a value.
/// Also, some of them can be considered precalculated values, rather than
/// formal parameters.
/// This class encapsulates a single parameter.
/// Template argument expected to be float or double.
/// </summary>
template <typename T>
class EMBER_API ParamWithName
{
	friend ParametricVariation<T>;

public:
	/// <summary>
	/// Default constructor.
	/// </summary>
	ParamWithName()
	{
		Init(nullptr, "", 0, REAL, TLOW, TMAX);
	}

	/// <summary>
	/// Constructor for a precalc param that takes arguments.
	/// </summary>
	/// <param name="isPrecalc">Whether the parameter is actually a precalculated value. Default: false.</param>
	/// <param name="param">A pointer to the parameter</param>
	/// <param name="name">The name of the parameter</param>
	ParamWithName(bool isPrecalc,
		T* param,
		string name)
	{
		Init(param, name, 0, REAL, TLOW, TMAX, true);
	}

	/// <summary>
	/// Constructor for a non-precalc param that takes arguments.
	/// </summary>
	/// <param name="param">A pointer to the parameter</param>
	/// <param name="name">The name of the parameter</param>
	/// <param name="def">The default value of the parameter</param>
	/// <param name="type">The type of the parameter</param>
	/// <param name="min">The minimum value the parameter can be</param>
	/// <param name="max">The maximum value the parameter can be</param>
	ParamWithName(T* param, string name, T def = 0, eParamType type = REAL, T min = TLOW, T max = TMAX)
	{
		Init(param, name, def, type, min, max);
	}

	/// <summary>
	/// Copy constructor.
	/// Note this constructor does not take an additional template parameter
	/// like the others do. This is because there is not way to assign the
	/// param pointer from one type to another. Luckily, such functionality is not needed
	/// with this class.
	/// </summary>
	/// <param name="paramWithName">The ParamWithName object to copy</param>
	ParamWithName(const ParamWithName<T>& paramWithName)
	{
		*this = paramWithName;
	}

	/// <summary>
	/// Assignment operator.
	/// Note this assignment operator does not take an additional template parameter
	/// like the others do. This is because there is not way to assign the
	/// param pointer from one type to another. Luckily, such functionality is not needed
	/// with this class.
	/// </summary>
	/// <param name="paramWithName">The ParamWithName object to copy.</param>
	/// <returns>Reference to updated self</returns>
	ParamWithName<T>& operator = (const ParamWithName<T>& paramWithName)
	{
		if (this != &paramWithName)
		{
			m_Param = paramWithName.m_Param;
			m_Def = paramWithName.m_Def;
			m_Min = paramWithName.m_Min;
			m_Max = paramWithName.m_Max;
			m_Type = paramWithName.m_Type;
			m_Name = paramWithName.m_Name;
			m_IsPrecalc = paramWithName.m_IsPrecalc;
		}

		return *this;
	}

	/// <summary>
	/// Constructor that takes arguments.
	/// </summary>
	/// <param name="param">A pointer to the parameter</param>
	/// <param name="name">The name of the parameter</param>
	/// <param name="def">The default value of the parameter</param>
	/// <param name="type">The type of the parameter</param>
	/// <param name="min">The minimum value the parameter can be</param>
	/// <param name="max">The maximum value the parameter can be</param>
	/// <param name="isPrecalc">Whether the parameter is actually a precalculated value. Default: false.</param>
	void Init(T* param, string name, T def = 0, eParamType type = REAL, T min = TLOW, T max = TMAX, bool isPrecalc = false)
	{
		m_Param = param;
		m_Def = def;
		m_Min = min;
		m_Max = max;
		m_Type = type;
		m_Name = name;
		m_IsPrecalc = isPrecalc;

		Set(m_Def);//Initial value.
	}

	/// <summary>
	/// Set this parameter to the val.
	/// Depending on the type that was specified in the constructor, various restrictions
	/// will be put on the value.
	/// </summary>
	/// <param name="val">The value to set the parameter to</param>
	void Set(T val)
	{
		switch (m_Type)
		{
			case REAL :
			{
				*m_Param = max(min(val, m_Max), m_Min);
				break;
			}

			case REAL_CYCLIC :
			{
				if (val > m_Max)
					*m_Param = m_Min + fmod(val - m_Min, m_Max - m_Min);
				else if (val < m_Min)
					*m_Param = m_Max - fmod(m_Max - val, m_Max - m_Min);
				else
					*m_Param = val;

				break;
			}

			case REAL_NONZERO :
			{
				T vd = max(min(val, m_Max), m_Min);

				if (IsNearZero(vd))
					*m_Param = EPS * SignNz(vd);
				else
					*m_Param = vd;

				break;
			}

			case INTEGER :
			{
				*m_Param = T((int)max(min<T>((T)Floor<T>(val + T(0.5)), m_Max), m_Min));
				break;
			}

			case INTEGER_NONZERO :
			default:
			{
				int vi = (int)max(min<T>((T)Floor<T>(val + T(0.5)), m_Max), m_Min);

				if (vi == 0)
					vi = (int)SignNz<T>(val);

				*m_Param = T(vi);
				break;
			}
		}
	}

	/// <summary>
	/// Return the values of the ParamWithName as a string.
	/// </summary>
	/// <returns>The ParamWithName values as a string</returns>
	string ToString() const
	{
		ostringstream ss;

		ss << "Param Name: " << m_Name << endl
		   << "Param Pointer: " << m_Param << endl
		   << "Param Value: " << *m_Param << endl
		   << "Param Def: " << m_Def << endl
		   << "Param Min: " << m_Min << endl
		   << "Param Max: " << m_Max << endl
		   << "Param Type: " << m_Type << endl
		   << "Is Precalc: " << m_IsPrecalc << endl;

		return ss.str();
	}

	/// <summary>
	/// Accessors.
	/// </summary>
	T* Param() const { return m_Param; }
	T ParamVal() const { return *m_Param; }
	T Def() const { return m_Def; }
	T Min() const { return m_Min; }
	T Max() const { return m_Max; }
	eParamType Type() const { return m_Type; }
	string Name() const { return m_Name; }
	bool IsPrecalc() const { return m_IsPrecalc; }

private:
	T* m_Param;//Pointer to the parameter value.
	T m_Def;//The default value of the parameter.
	T m_Min;//The minimum value the parameter can be.
	T m_Max;//The maximum value the parameter can be.
	eParamType m_Type;//The type of the parameter.
	string m_Name;//Name of the parameter.
	bool m_IsPrecalc;//Whether the parameter is actually a precalculated value.
};

/// <summary>
/// Parametric variations use parameters in addition to weight.
/// These values are stored in members of derived classes, however
/// for easy access, pointers to them are also stored in a vector
/// of ParamWithName in this class.
/// Template argument expected to be float or double.
/// </summary>
template <typename T>
class EMBER_API ParametricVariation : public Variation<T>
{
using Variation<T>::Precalc;

public:
	/// <summary>
	/// Constructor which takes arguments and just passes them to the base class.
	/// </summary>
	/// <param name="name">The unique name of the variation</param>
	/// <param name="id">The unique ID of the variation</param>
	/// <param name="weight">The weight. Default: 1.</param>
	/// <param name="needPrecalcSumSquares">Whether it uses the precalc sum squares value in its calculations. Default: false.</param>
	/// <param name="needPrecalcSqrtSumSquares">Whether it uses the sqrt precalc sum squares value in its calculations. Default: false.</param>
	/// <param name="needPrecalcAngles">Whether it uses the precalc sin and cos values in its calculations. Default: false.</param>
	/// <param name="needPrecalcAtanXY">Whether it uses the precalc atan XY value in its calculations. Default: false.</param>
	/// <param name="needPrecalcAtanYX">Whether it uses the precalc atan YX value in its calculations. Default: false.</param>
	ParametricVariation(const char* name, eVariationId id, T weight = 1.0,
				   bool needPrecalcSumSquares = false,
				   bool needPrecalcSqrtSumSquares = false,
				   bool needPrecalcAngles = false,
				   bool needPrecalcAtanXY = false,
				   bool needPrecalcAtanYX = false)
				   : Variation<T>(name, id, weight,
								   needPrecalcSumSquares,
								   needPrecalcSqrtSumSquares,
								   needPrecalcAngles,
								   needPrecalcAtanXY,
								   needPrecalcAtanYX)
	{
		m_Params.reserve(5);
	}

	/// <summary>
	/// Default copy constructor.
	/// </summary>
	/// <param name="var">The ParametricVariation object to copy</param>
	ParametricVariation(const ParametricVariation<T>& var)
		: Variation<T>(var)
	{
		//Derived classes will have to initialize the m_Params vector
		//to the addresses of its members and then assign values from var.
		m_Params.reserve(5);
	}

	/// <summary>
	/// Copy constructor to copy a ParametricVariation object of type U.
	/// </summary>
	/// <param name="var">The ParametricVariation object to copy</param>
	template <typename U>
	ParametricVariation(const ParametricVariation<U>& var)
		: Variation<T>(var)
	{
		//Derived classes will have to initialize the m_Params vector
		//to the addresses of its members and then assign values from var.
		m_Params.reserve(5);
	}

	/// <summary>
	/// Empty virtual destructor.
	/// Needed to eliminate warnings about inlining.
	/// </summary>
	virtual ~ParametricVariation()
	{
	}

	/// <summary>
	/// Determine whether the params vector contains a parameter with the specified name.
	/// </summary>
	/// <param name="name">The name to search for</param>
	/// <returns>True if found, else false.</returns>
	bool ContainsParam(const char* name)
	{
		bool b = false;

		ForEach(m_Params, [&](ParamWithName<T>& param)
		{
			if (!_stricmp(param.Name().c_str(), name))
				b = true;
		});

		return b;
	}

	/// <summary>
	/// Get a pointer to a parameter value with the specified name.
	/// </summary>
	/// <param name="name">The name to search for</param>
	/// <returns>A pointer to the parameter value if the name matched, else false.</returns>
	T* GetParam(const char* name)
	{
		for (size_t i = 0; i < m_Params.size(); i++)
			if (!_stricmp(m_Params[i].Name().c_str(), name))
				return m_Params[i].Param();

		return nullptr;
	}

	/// <summary>
	/// Get a parameter value with the specified name.
	/// </summary>
	/// <param name="name">The name to search for</param>
	/// <returns>A parameter value if the name matched, else 0.</returns>
	T GetParamVal(const char* name) const
	{
		for (size_t i = 0; i < m_Params.size(); i++)
			if (!_stricmp(m_Params[i].Name().c_str(), name))
				return m_Params[i].ParamVal();

		return 0;
	}

	/// <summary>
	/// Assign a value to the parameter with the specified name and call virtual Precalc() if found.
	/// </summary>
	/// <param name="name">The name of the parameter to assign to</param>
	/// <param name="val">The value to assign</param>
	/// <returns>True if the name matched, else false.</returns>
	virtual bool SetParamVal(const char* name, T val)
	{
		bool b = false;

		ForEach(m_Params, [&](ParamWithName<T>& param)
		{
			if (!_stricmp(param.Name().c_str(), name))
			{
				param.Set(val);
				b = true;
			}
		});

		if (b)
			this->Precalc();

		return b;
	}

	/// <summary>
	/// Assign a value to the parameter at the specified index and call virtual Precalc() if found.
	/// </summary>
	/// <param name="index">The index of the parameter to assign to</param>
	/// <param name="val">The value to assign</param>
	/// <returns>True if the index was in range, else false.</returns>
	virtual bool SetParamVal(int index, T val)
	{
		bool b = false;

		if (index < m_Params.size())
			m_Params[index].Set(val);

		if (b)
			this->Precalc();

		return b;
	}

	/// <summary>
	/// Severe hack to get g++ to compile this.
	/// </summary>
	virtual void Precalc() override { }

	/// <summary>
	/// Place the parametric variation in a random state by setting all of the
	/// non-precalc params to values between -1 and 1;
	/// </summary>
	/// <param name="rand">The rand.</param>
	virtual void Random(QTIsaac<ISAAC_SIZE, ISAAC_INT>& rand) override
	{
		Variation<T>::Random(rand);
		ForEach(m_Params, [&](ParamWithName<T>& param) { param.Set(rand.Frand11<T>()); });
		this->Precalc();
	}

	/// <summary>
	/// Assign all 0 to all parameters and call virtual Precalc().
	/// </summary>
	void Clear()
	{
		ForEach(m_Params, [&](ParamWithName<T>& param) { *(param.Param()) = 0; });
		this->Precalc();
	}

	/// <summary>
	/// Return a vector of all parameter names, optionally including precalcs.
	/// </summary>
	/// <param name="includePrecalcs">Whether to include the names of precalcs in the returned vector</param>
	/// <returns>A vector of all parameter names</returns>
	vector<string> ParamNames(bool includePrecalcs = false)
	{
		vector<string> vec;

		vec.reserve(m_Params.size());
		ForEach(m_Params, [&](const ParamWithName<T>& param)
		{
			if ((includePrecalcs && param.IsPrecalc()) || !param.IsPrecalc())
				vec.push_back(param.Name());
		});

		return vec;
	}

	/// <summary>
	/// Return the name, weight and parameters of the variation as a string.
	/// </summary>
	/// <returns>The name, weight and parameters of the variation</returns>
	virtual string ToString() const
	{
		ostringstream ss;

		ss << Variation<T>::ToString() << endl;
		ForEach(m_Params, [&](const ParamWithName<T>& param) { ss << param.ToString() << endl; });

		return ss.str();
	}

	/// <summary>
	/// Accessors.
	/// </summary>
	ParamWithName<T>* Params() { return &m_Params[0]; }
	unsigned int ParamCount() { return (unsigned int)m_Params.size(); }
	const vector<ParamWithName<T>>& ParamsVec() const { return m_Params; }

protected:
	/// <summary>
	/// Copy the non-precalc parameter values of type U to the pointer locations stored in the params vector of type T,
	/// where T is usually the same type as U.
	/// This will copy the values to the members of derived classes.
	/// </summary>
	/// <param name="params">The vector of parameters whose values will be copied</param>
	template <typename U>
	void CopyParamVals(const vector<ParamWithName<U>>& params)
	{
		if (m_Params.size() == params.size())
		{
			for (size_t i = 0; i < m_Params.size(); i++)
				if (!m_Params[i].IsPrecalc())
					m_Params[i].Set(T(params[i].ParamVal()));

			this->Precalc();
		}
	}

	vector<ParamWithName<T>> m_Params;//The params pointer vector which stores pointer to parameter members of derived classes.
};

/// <summary>
/// Macro to define a default copy constructor, a copy constructor for a different template type, and a virtual Copy() function
/// for classes derived directly from Variation.
/// Defining assignment operators isn't really needed because Variations are always held as pointers.
/// </summary>

#define VARUSINGS \
	using Variation<T>::m_Weight; \
	using Variation<T>::m_Xform; \
	using Variation<T>::m_VariationId; \
	using Variation<T>::m_Name; \
	using Variation<T>::m_VarType; \
	using Variation<T>::m_AssignType; \
	using Variation<T>::SetType; \
	using Variation<T>::IndexInXform; \
	using Variation<T>::XformIndexInEmber; \
	using Variation<T>::Prefix;

#ifdef DO_DOUBLE
#define VARCOPYDOUBLE(name) \
	virtual void Copy(Variation<double>*& var) const override \
	{ \
		if (var != nullptr) \
			delete var; \
		\
		var = new name<double>(*this); \
	} \

#else
#define VARCOPYDOUBLE(name)
#endif // DO_DOUBLE

#define VARCOPY(name) \
	VARUSINGS \
	public: \
	name(const name<T>& var) \
		: Variation<T>(var) \
	{ \
	} \
	\
	template <typename U> \
	name(const name<U>& var) \
		: Variation<T>(var) \
	{ \
	} \
	\
	virtual Variation<T>* Copy() override \
	{ \
		return new name<T>(*this); \
	} \
	\
	virtual void Copy(Variation<float>*& var) const override \
	{ \
		if (var != nullptr) \
			delete var; \
		\
		var = new name<float>(*this); \
	} \
	\
	VARCOPYDOUBLE(name) \

#define PREPOSTVARCOPY(name, base) \
	name(const name<T>& var) \
		: base<T>(var) \
	{ \
	} \
	\
	template <typename U> \
	name(const name<U>& var) \
		: base<T>(var) \
	{ \
	} \
	\
	virtual Variation<T>* Copy() override \
	{ \
		return new name<T>(*this); \
	} \
	\
	virtual void Copy(Variation<float>*& var) const override \
	{ \
		if (var != nullptr) \
			delete var; \
		\
		var = new name<float>(*this); \
	} \
	\
	VARCOPYDOUBLE(name) \

/// <summary>
/// Macro to create pre and post counterparts to a variation.
/// Assign type defaults to set.
/// </summary>

#define MAKEPREPOSTVAR(varName, stringName, enumName) MAKEPREPOSTVARASSIGN(varName, stringName, enumName, ASSIGNTYPE_SET)
#define MAKEPREPOSTVARASSIGN(varName, stringName, enumName, assignType) \
	template <typename T> \
	class EMBER_API Pre##varName##Variation : public varName##Variation<T> \
	{ \
	VARUSINGS \
	public: \
		Pre##varName##Variation(T weight = 1.0) : varName##Variation<T>(weight) \
		{ \
			m_VariationId = VAR_PRE_##enumName; \
			m_Name = "pre_"#stringName; \
			m_AssignType = assignType; \
			SetType(); \
		} \
		\
		PREPOSTVARCOPY(Pre##varName##Variation, varName##Variation) \
	}; \
	\
	template <typename T> \
	class EMBER_API Post##varName##Variation : public varName##Variation<T> \
	{ \
	VARUSINGS \
	public:\
		Post##varName##Variation(T weight = 1.0) : varName##Variation<T>(weight) \
		{ \
			m_VariationId = VAR_POST_##enumName; \
			m_Name = "post_"#stringName; \
			m_AssignType = assignType; \
			SetType(); \
		} \
		\
		PREPOSTVARCOPY(Post##varName##Variation, varName##Variation) \
	};

/// <summary>
/// Macro to define a copy constructor, a copy constructor for a different template type, and a virtual Copy() function
/// for classes derived from ParametricVariation.
/// Another major shortcoming of C++: Ideally, Init() should be a virtual function defined in ParametricVariation.
/// It would be called in that constructor, and defined in each derived class. However, that can't be done because the vtable
/// is not setup during construction.
/// Instead, every class must define it as a non-virtual function and explicitly call it in its constructor.
/// </summary>

#define PARVARUSINGS \
	using ParametricVariation<T>::m_Params; \
	using ParametricVariation<T>::CopyParamVals;

#define PARVARCOPY(name) \
	VARUSINGS \
	PARVARUSINGS \
	public: \
	name(const name<T>& var) \
		: ParametricVariation<T>(var) \
	{ \
		Init(); /* Assign the addresses of the members to the vector. */ \
		CopyParamVals(var.ParamsVec()); /* Copy values from var's vector and precalc. */ \
	} \
	\
	template <typename U> \
	name(const name<U>& var) \
		: ParametricVariation<T>(var) \
	{ \
		Init(); /* Assign the addresses of the members to the vector. */ \
		CopyParamVals(var.ParamsVec()); /* Copy values from var's vector and precalc. */ \
	} \
	\
	virtual Variation<T>* Copy() override \
	{ \
		return new name<T>(*this); \
	} \
	\
	virtual void Copy(Variation<float>*& var) const override \
	{ \
		if (var != nullptr) \
			delete var; \
		\
		var = new name<float>(*this); \
	} \
	\
	VARCOPYDOUBLE(name) \

#define PREPOSTPARVARCOPY(name, base) \
	name(const name<T>& var) \
		: base<T>(var) \
	{ \
		Init(); /* Assign the addresses of the members to the vector. */ \
		CopyParamVals(var.ParamsVec()); /* Copy values from var's vector and precalc. */ \
	} \
	\
	template <typename U> \
	name(const name<U>& var) \
		: base<T>(var) \
	{ \
		Init(); /* Assign the addresses of the members to the vector. */ \
		CopyParamVals(var.ParamsVec()); /* Copy values from var's vector and precalc. */ \
	} \
	\
	virtual Variation<T>* Copy() override \
	{ \
		return new name<T>(*this); \
	} \
	\
	virtual void Copy(Variation<float>*& var) const override \
	{ \
		if (var != nullptr) \
			delete var; \
		\
		var = new name<float>(*this); \
	} \
	\
	VARCOPYDOUBLE(name)

/// <summary>
/// Macro to create pre and post counterparts to a parametric variation.
/// Assign type defaults to set.
/// This uses the severe hack of calling Init() again after the type has been set
/// avoid having to change the constructor arguments for about 300 variations.
/// </summary>

#define MAKEPREPOSTPARVAR(varName, stringName, enumName) MAKEPREPOSTPARVARASSIGN(varName, stringName, enumName, ASSIGNTYPE_SET)
#define MAKEPREPOSTPARVARASSIGN(varName, stringName, enumName, assignType) \
	template <typename T> \
	class EMBER_API Pre##varName##Variation : public varName##Variation <T> \
	{ \
	VARUSINGS \
	PARVARUSINGS \
	using varName##Variation<T>::Init; \
	public:\
		Pre##varName##Variation(T weight = 1.0) : varName##Variation<T>(weight) \
		{ \
			m_VariationId = VAR_PRE_##enumName; \
			m_Name = "pre_"#stringName; \
			m_AssignType = assignType; \
			SetType(); \
			Init(); \
		} \
		\
		PREPOSTPARVARCOPY(Pre##varName##Variation, varName##Variation) \
	}; \
	\
	template <typename T> \
	class EMBER_API Post##varName##Variation : public varName##Variation<T> \
	{ \
	VARUSINGS \
	PARVARUSINGS \
	using varName##Variation<T>::Init; \
	public:\
		Post##varName##Variation(T weight = 1.0) : varName##Variation<T>(weight) \
		{ \
			m_VariationId = VAR_POST_##enumName; \
			m_Name = "post_"#stringName; \
			m_AssignType = assignType; \
			SetType(); \
			Init(); \
		} \
		\
		PREPOSTPARVARCOPY(Post##varName##Variation, varName##Variation) \
	};
}
