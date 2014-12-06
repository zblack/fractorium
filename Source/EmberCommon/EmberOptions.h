#pragma once

#include "EmberCommon.h"

/// <summary>
/// EmberOptionEntry and EmberOptions classes.
/// </summary>

static const char* DescriptionString = "Ember - Fractal flames C++ port and enhancement with OpenCL GPU support";

/// <summary>
/// Enum for specifying which command line programs an option is meant to be used with.
/// If an option is used with multiple programs, their values are ORed together.
/// </summary>
enum eOptionUse
{
	OPT_USE_RENDER  = 1,
	OPT_USE_ANIMATE = 1 << 1,
	OPT_USE_GENOME  = 1 << 2,
	OPT_RENDER_ANIM = OPT_USE_RENDER  | OPT_USE_ANIMATE,
	OPT_ANIM_GENOME = OPT_USE_ANIMATE | OPT_USE_GENOME,
	OPT_USE_ALL     = OPT_USE_RENDER  | OPT_USE_ANIMATE | OPT_USE_GENOME
};

/// <summary>
/// Unique identifiers for every available option across all programs.
/// </summary>
enum eOptionIDs
{
	//Diagnostic args.
	OPT_HELP,
	OPT_VERSION,
	OPT_VERBOSE,
	OPT_DEBUG,
	OPT_DUMP_ARGS,
	OPT_PROGRESS,
	OPT_DUMP_OPENCL_INFO,

	//Boolean args.
	OPT_OPENCL,
	OPT_EARLYCLIP,
	OPT_POS_Y_UP,
	OPT_TRANSPARENCY,
	OPT_NAME_ENABLE,
	OPT_INT_PALETTE,
	OPT_HEX_PALETTE,
	OPT_INSERT_PALETTE,
	OPT_JPEG_COMMENTS,
	OPT_PNG_COMMENTS,
	OPT_WRITE_GENOME,
	OPT_ENCLOSED,
	OPT_NO_EDITS,
	OPT_UNSMOOTH_EDGE,
	OPT_LOCK_ACCUM,
	OPT_DUMP_KERNEL,

	//Value args.
	OPT_OPENCL_PLATFORM,//Int value args.
	OPT_OPENCL_DEVICE,
	OPT_SEED,
	OPT_NTHREADS,
	OPT_STRIPS,
	OPT_SUPERSAMPLE,
	OPT_BITS,
	OPT_BPC,
	OPT_SBS,
	OPT_PRINT_EDIT_DEPTH,
	OPT_JPEG,
	OPT_BEGIN,
	OPT_END,
	OPT_FRAME,
	OPT_TIME,
	OPT_DTIME,
	OPT_NFRAMES,
	OPT_SYMMETRY,
	OPT_SHEEP_GEN,
	OPT_SHEEP_ID,
	OPT_LOOPS,
	OPT_REPEAT,
	OPT_TRIES,
	OPT_MAX_XFORMS,

	OPT_SS,//Float value args.
	OPT_QS,
	OPT_PIXEL_ASPECT,
	OPT_STAGGER,
	OPT_AVG_THRESH,
	OPT_BLACK_THRESH,
	OPT_WHITE_LIMIT,
	OPT_SPEED,
	OPT_OFFSETX,
	OPT_OFFSETY,
	OPT_USEMEM,

	OPT_ISAAC_SEED,//String value args.
	OPT_IN,
	OPT_OUT,
	OPT_PREFIX,
	OPT_SUFFIX,
	OPT_FORMAT,
	OPT_PALETTE_FILE,
	//OPT_PALETTE_IMAGE,
	OPT_ID,
	OPT_URL,
	OPT_NICK,
	OPT_COMMENT,
	OPT_TEMPLATE,
	OPT_CLONE,
	OPT_CLONE_ALL,
	OPT_CLONE_ACTION,
	OPT_ANIMATE,
	OPT_MUTATE,
	OPT_CROSS0,
	OPT_CROSS1,
	OPT_METHOD,
	OPT_INTER,
	OPT_ROTATE,
	OPT_STRIP,
	OPT_SEQUENCE,
	OPT_USE_VARS,
	OPT_DONT_USE_VARS,
	OPT_EXTRAS
};

class EmberOptions;

/// <summary>
/// A single option.
/// Template argument expected to be bool, int, unsigned int, double or string.
/// </summary>
template <typename T>
class EmberOptionEntry
{
friend class EmberOptions;

private:
	/// <summary>
	/// Default constructor. This should never be used, instead use the one that takes arguments.
	/// </summary>
	EmberOptionEntry()
	{
		m_OptionUse = OPT_USE_ALL;
		m_Option.nArgType = SO_NONE;
		m_Option.nId = 0;
		m_Option.pszArg = _T("--fillmein");
		m_DocString = "Dummy doc";
	}

public:
	/// <summary>
	/// Constructor that takes arguments.
	/// </summary>
	/// <param name="optUsage">The specified program usage</param>
	/// <param name="optId">The option identifier enum</param>
	/// <param name="arg">The command line argument (--arg)</param>
	/// <param name="defaultVal">The default value to use the option was not given on the command line</param>
	/// <param name="argType">The format the argument should be given in</param>
	/// <param name="docString">The documentation string describing what the argument means</param>
	EmberOptionEntry(eOptionUse optUsage, eOptionIDs optId, const CharT* arg, T defaultVal, ESOArgType argType, string docString)
	{
		m_OptionUse = optUsage;
		m_Option.nId = (int)optId;
		m_Option.pszArg = arg;
		m_Option.nArgType = argType;
		m_DocString = docString;
		m_NameWithoutDashes = Trim(string(arg), '-');
		m_Val = Arg<T>((char*)m_NameWithoutDashes.c_str(), defaultVal);
	}

	/// <summary>
	/// Copy constructor.
	/// </summary>
	/// <param name="entry">The EmberOptionEntry object to copy</param>
	EmberOptionEntry(const EmberOptionEntry& entry)
	{
		*this = entry;
	}

	/// <summary>
	/// Functor accessors.
	/// </summary>
	inline T    operator() (void) { return m_Val; }
	inline void operator() (T t)  { m_Val = t;    }

private:
	eOptionUse m_OptionUse;
	CSimpleOpt::SOption m_Option;
	string m_DocString;
	string m_NameWithoutDashes;
	T m_Val;
};

/// <summary>
/// Macros for setting up and parsing various option types.
/// </summary>

//Bool.
#define Eob EmberOptionEntry<bool>
#define INITBOOLOPTION(member, option) \
		member = option; \
		m_BoolArgs.push_back(&member)

#define PARSEBOOLOPTION(opt, member) \
	case (opt): \
		member(true); \
		break

//Int.
#define Eoi EmberOptionEntry<int>
#define INITINTOPTION(member, option) \
		member = option; \
		m_IntArgs.push_back(&member)

#define PARSEINTOPTION(opt, member) \
	case (opt): \
		sscanf_s(args.OptionArg(), "%d", &member.m_Val); \
		break

//Uint.
#define Eou EmberOptionEntry<unsigned int>
#define INITUINTOPTION(member, option) \
		member = option; \
		m_UintArgs.push_back(&member)

#define PARSEUINTOPTION(opt, member) \
	case (opt): \
		sscanf_s(args.OptionArg(), "%u", &member.m_Val); \
		break

//Double.
#define Eod EmberOptionEntry<double>
#define INITDOUBLEOPTION(member, option) \
		member = option; \
		m_DoubleArgs.push_back(&member)

#define PARSEDOUBLEOPTION(opt, member) \
	case (opt): \
		sscanf_s(args.OptionArg(), "%lf", &member.m_Val); \
		break

//String.
#define Eos EmberOptionEntry<string>
#define INITSTRINGOPTION(member, option) \
		member = option; \
		m_StringArgs.push_back(&member)

#define PARSESTRINGOPTION(opt, member) \
	case (opt): \
		member.m_Val = args.OptionArg(); \
		break

/// <summary>
/// Class for holding all available options across all command line programs.
/// Some are used only for a single program, while others are used for more than one.
/// This prevents having to keep separate documentation strings around for different programs.
/// </summary>
class EmberOptions
{
public:
	/// <summary>
	/// Constructor that populates all available options.
	/// </summary>
	EmberOptions()
	{
		m_BoolArgs.reserve(25);
		m_IntArgs.reserve(25);
		m_UintArgs.reserve(25);
		m_DoubleArgs.reserve(25);
		m_StringArgs.reserve(35);

		//Diagnostic bools.
		INITBOOLOPTION(Help,           Eob(OPT_USE_ALL,     OPT_HELP,             _T("--help"),                 false,                SO_NONE,    "\t--help                   Show this screen.\n"));
		INITBOOLOPTION(Version,        Eob(OPT_USE_ALL,     OPT_VERSION,          _T("--version"),              false,                SO_NONE,    "\t--version                Show version.\n"));
		INITBOOLOPTION(Verbose,        Eob(OPT_USE_ALL,     OPT_VERBOSE,          _T("--verbose"),              false,                SO_NONE,    "\t--verbose                Verbose output.\n"));
		INITBOOLOPTION(Debug,          Eob(OPT_USE_ALL,     OPT_DEBUG,            _T("--debug"),                false,                SO_NONE,    "\t--debug                  Debug output.\n"));
		INITBOOLOPTION(DumpArgs,       Eob(OPT_USE_ALL,     OPT_DUMP_ARGS,        _T("--dumpargs"),             false,                SO_NONE,    "\t--dumpargs               Print all arguments entered from either the command line or environment variables.\n"));
		INITBOOLOPTION(DoProgress,     Eob(OPT_USE_ALL,     OPT_PROGRESS,         _T("--progress"),             false,                SO_NONE,    "\t--progress               Display progress. This will slow down processing by about 10%%.\n"));
		INITBOOLOPTION(OpenCLInfo,     Eob(OPT_USE_ALL,     OPT_DUMP_OPENCL_INFO, _T("--openclinfo"),           false,                SO_NONE,    "\t--openclinfo             Display platforms and devices for OpenCL.\n"));

		//Execution bools.
		INITBOOLOPTION(EmberCL,        Eob(OPT_USE_ALL,     OPT_OPENCL,           _T("--opencl"),               false,                SO_NONE,    "\t--opencl                 Use OpenCL renderer (EmberCL) for rendering [default: false].\n"));
		INITBOOLOPTION(EarlyClip,      Eob(OPT_USE_ALL,     OPT_EARLYCLIP,        _T("--earlyclip"),            false,                SO_NONE,    "\t--earlyclip              Perform clipping of RGB values before spatial filtering for better antialiasing and resizing [default: false].\n"));
		INITBOOLOPTION(YAxisUp,        Eob(OPT_USE_ALL,     OPT_POS_Y_UP,         _T("--yaxisup"),              false,                SO_NONE,    "\t--yaxisup                Orient the image with the positive y axis pointing up [default: false].\n"));
		INITBOOLOPTION(Transparency,   Eob(OPT_USE_ALL,     OPT_TRANSPARENCY,     _T("--transparency"),         false,                SO_NONE,    "\t--transparency           Include alpha channel in final output [default: false except for PNG].\n"));
		INITBOOLOPTION(NameEnable,     Eob(OPT_USE_RENDER,  OPT_NAME_ENABLE,      _T("--name_enable"),          false,                SO_NONE,    "\t--name_enable            Use the name attribute contained in the xml as the output filename [default: false].\n"));
		INITBOOLOPTION(IntPalette,     Eob(OPT_RENDER_ANIM, OPT_INT_PALETTE,      _T("--intpalette"),           false,                SO_NONE,    "\t--intpalette             Force palette RGB values to be integers [default: false (float)].\n"));
		INITBOOLOPTION(HexPalette,     Eob(OPT_USE_ALL,     OPT_HEX_PALETTE,      _T("--hex_palette"),          true,                 SO_NONE,    "\t--hex_palette            Force palette RGB values to be hex [default: true].\n"));
		INITBOOLOPTION(InsertPalette,  Eob(OPT_RENDER_ANIM, OPT_INSERT_PALETTE,   _T("--insert_palette"),       false,                SO_NONE,    "\t--insert_palette         Insert the palette into the image for debugging purposes [default: false].\n"));
		INITBOOLOPTION(JpegComments,   Eob(OPT_RENDER_ANIM, OPT_JPEG_COMMENTS,    _T("--enable_jpeg_comments"), true,                 SO_NONE,    "\t--enable_jpeg_comments   Enables comments in the jpeg header [default: true].\n"));
		INITBOOLOPTION(PngComments,    Eob(OPT_RENDER_ANIM, OPT_PNG_COMMENTS,     _T("--enable_png_comments"),  true,                 SO_NONE,    "\t--enable_png_comments    Enables comments in the png header [default: true].\n"));
		INITBOOLOPTION(WriteGenome,    Eob(OPT_USE_ANIMATE, OPT_WRITE_GENOME,     _T("--write_genome"),         false,                SO_NONE,    "\t--write_genome           Write out flame associated with center of motion blur window [default: false].\n"));
		INITBOOLOPTION(Enclosed,       Eob(OPT_USE_GENOME,  OPT_ENCLOSED,         _T("--enclosed"),             true,                 SO_NONE,    "\t--enclosed               Use enclosing XML tags [default: false].\n"));
		INITBOOLOPTION(NoEdits,        Eob(OPT_USE_GENOME,  OPT_NO_EDITS,         _T("--noedits"),              false,                SO_NONE,    "\t--noedits                Exclude edit tags when writing Xml [default: false].\n"));
		INITBOOLOPTION(UnsmoothEdge,   Eob(OPT_USE_GENOME,  OPT_UNSMOOTH_EDGE,    _T("--unsmoother"),           false,                SO_NONE,    "\t--unsmoother             Do not use smooth blending for sheep edges [default: false].\n"));
		INITBOOLOPTION(LockAccum,	   Eob(OPT_USE_ALL,		OPT_LOCK_ACCUM,       _T("--lock_accum"),           false,                SO_NONE,    "\t--lock_accum             Lock threads when accumulating to the histogram using the CPU. This will drop performance to that of single threading [default: false].\n"));
		INITBOOLOPTION(DumpKernel,	   Eob(OPT_USE_RENDER,	OPT_DUMP_KERNEL,      _T("--dump_kernel"),          false,                SO_NONE,    "\t--dump_kernel            Print the iteration kernel string when using OpenCL (ignored for CPU) [default: false].\n"));

		//Int.
		INITINTOPTION(Symmetry,        Eoi(OPT_USE_GENOME,  OPT_SYMMETRY,         _T("--symmetry"),             0,                    SO_REQ_SEP, "\t--symmetry=<val>         Set symmetry of result [default: 0].\n"));
		INITINTOPTION(SheepGen,        Eoi(OPT_USE_GENOME,  OPT_SHEEP_GEN,        _T("--sheep_gen"),            -1,                   SO_REQ_SEP, "\t--sheep_gen=<val>        Sheep generation of this flame [default: -1].\n"));
		INITINTOPTION(SheepId,         Eoi(OPT_USE_GENOME,  OPT_SHEEP_ID,         _T("--sheep_id"),             -1,                   SO_REQ_SEP, "\t--sheep_id=<val>         Sheep ID of this flame [default: -1].\n"));
		INITUINTOPTION(Platform,       Eou(OPT_USE_ALL,     OPT_OPENCL_PLATFORM,  _T("--platform"),             0,                    SO_REQ_SEP, "\t--platform               The OpenCL platform index to use [default: 0].\n"));
		INITUINTOPTION(Device,         Eou(OPT_USE_ALL,     OPT_OPENCL_DEVICE,    _T("--device"),               0,                    SO_REQ_SEP, "\t--device                 The OpenCL device index within the specified platform to use [default: 0].\n"));
		INITUINTOPTION(Seed,           Eou(OPT_USE_ALL,     OPT_SEED,             _T("--seed"),                 0,                    SO_REQ_SEP, "\t--seed=<val>             Integer seed to use for the random number generator [default: random].\n"));
		INITUINTOPTION(ThreadCount,    Eou(OPT_USE_ALL,     OPT_NTHREADS,         _T("--nthreads"),             0,                    SO_REQ_SEP, "\t--nthreads=<val>         The number of threads to use [default: use all available cores].\n"));
		INITUINTOPTION(Strips,		   Eou(OPT_USE_RENDER,  OPT_STRIPS,           _T("--nstrips"),              1,                    SO_REQ_SEP, "\t--nstrips=<val>          The number of fractions to split a single render frame into. Useful for print size renders or low memory systems [default: 1].\n"));
		INITUINTOPTION(Supersample,    Eou(OPT_RENDER_ANIM, OPT_SUPERSAMPLE,      _T("--supersample"),          0,                    SO_REQ_SEP, "\t--supersample=<val>      The supersample value used to override the one specified in the file [default: 0 (use value from file)].\n"));
		INITUINTOPTION(BitsPerChannel, Eou(OPT_RENDER_ANIM, OPT_BPC,              _T("--bpc"),                  8,                    SO_REQ_SEP, "\t--bpc=<val>              Bits per channel. 8 or 16 for PNG, 8 for all others [default: 8].\n"));
		INITUINTOPTION(SubBatchSize,   Eou(OPT_USE_ALL,		OPT_SBS,			  _T("--sub_batch_size"),		DEFAULT_SBS,		  SO_REQ_SEP, "\t--sub_batch_size=<val>   The chunk size that iterating will be broken into [default: 10k].\n"));
		INITUINTOPTION(Bits,           Eou(OPT_USE_ALL,     OPT_BITS,             _T("--bits"),                 33,                   SO_REQ_SEP, "\t--bits=<val>             Determines the types used for the histogram and accumulator [default: 33].\n"
																																							  "\t\t\t\t\t32:  Histogram: float, Accumulator: float.\n"
																																							  "\t\t\t\t\t33:  Histogram: float, Accumulator: float.\n"//This differs from the original which used an int hist for bits 33.
																																							  "\t\t\t\t\t64:  Histogram: double, Accumulator: double.\n"));

		INITUINTOPTION(PrintEditDepth, Eou(OPT_USE_ALL,     OPT_PRINT_EDIT_DEPTH, _T("--print_edit_depth"),     0,                    SO_REQ_SEP, "\t--print_edit_depth=<val> Depth to truncate <edit> tag structure when converting a flame to xml. 0 prints all <edit> tags [default: 0].\n"));
		INITUINTOPTION(JpegQuality,    Eou(OPT_RENDER_ANIM, OPT_JPEG,             _T("--jpeg"),                 95,                   SO_REQ_SEP, "\t--jpeg=<val>             Jpeg quality 0-100 for compression [default: 95].\n"));
		INITUINTOPTION(FirstFrame,     Eou(OPT_USE_ANIMATE, OPT_BEGIN,            _T("--begin"),                UINT_MAX,             SO_REQ_SEP, "\t--begin=<val>            Time of first frame to render [default: first time specified in file].\n"));
		INITUINTOPTION(LastFrame,      Eou(OPT_USE_ANIMATE, OPT_END,              _T("--end"),	                UINT_MAX,             SO_REQ_SEP, "\t--end=<val>              Time of last frame to render [default: last time specified in the input file].\n"));
		INITUINTOPTION(Time,           Eou(OPT_ANIM_GENOME, OPT_TIME,             _T("--time"),                 0,                    SO_REQ_SEP, "\t--time=<val>             Time of first and last frame (ie do one frame).\n"));
		INITUINTOPTION(Frame,          Eou(OPT_ANIM_GENOME, OPT_FRAME,            _T("--frame"),                0,                    SO_REQ_SEP, "\t--frame=<val>            Synonym for \"time\".\n"));
		INITUINTOPTION(Dtime,          Eou(OPT_USE_ANIMATE, OPT_DTIME,            _T("--dtime"),                1,                    SO_REQ_SEP, "\t--dtime=<val>            Time between frames [default: 1].\n"));
		INITUINTOPTION(Frames,         Eou(OPT_USE_GENOME,  OPT_NFRAMES,          _T("--nframes"),              20,                   SO_REQ_SEP, "\t--nframes=<val>          Number of frames for each stage of the animation [default: 20].\n"));
		INITUINTOPTION(Loops,          Eou(OPT_USE_GENOME,  OPT_LOOPS,            _T("--loops"),                1,                    SO_REQ_SEP, "\t--loops=<val>            Number of times to rotate each control point in sequence [default: 1].\n"));
		INITUINTOPTION(Repeat,         Eou(OPT_USE_GENOME,  OPT_REPEAT,           _T("--repeat"),               1,                    SO_REQ_SEP, "\t--repeat=<val>           Number of new flames to create. Ignored if sequence, inter or rotate were specified [default: 1].\n"));
		INITUINTOPTION(Tries,          Eou(OPT_USE_GENOME,  OPT_TRIES,            _T("--tries"),                10,                   SO_REQ_SEP, "\t--tries=<val>            Number times to try creating a flame that meets the specified constraints. Ignored if sequence, inter or rotate were specified [default: 10].\n"));
		INITUINTOPTION(MaxXforms,      Eou(OPT_USE_GENOME,  OPT_MAX_XFORMS,       _T("--maxxforms"),            UINT_MAX,             SO_REQ_SEP, "\t--maxxforms=<val>        The maximum number of xforms allowed in the final output.\n"));

		//Double.
		INITDOUBLEOPTION(SizeScale,    Eod(OPT_RENDER_ANIM, OPT_SS,               _T("--ss"),                   1,                    SO_REQ_SEP, "\t--ss=<val>               Size scale. All dimensions are scaled by this amount [default: 1.0].\n"));
		INITDOUBLEOPTION(QualityScale, Eod(OPT_RENDER_ANIM, OPT_QS,               _T("--qs"),                   1,                    SO_REQ_SEP, "\t--qs=<val>               Quality scale. All quality values are scaled by this amount [default: 1.0].\n"));
		INITDOUBLEOPTION(AspectRatio,  Eod(OPT_USE_ALL,     OPT_PIXEL_ASPECT,     _T("--pixel_aspect"),         1,                    SO_REQ_SEP, "\t--pixel_aspect=<val>     Aspect ratio of pixels (width over height), eg. 0.90909 for NTSC [default: 1.0].\n"));
		INITDOUBLEOPTION(Stagger,      Eod(OPT_USE_GENOME,  OPT_STAGGER,          _T("--stagger"),              0,                    SO_REQ_SEP, "\t--stagger=<val>          Affects simultaneity of xform interpolation during flame interpolation.\n"
																												 											  "\t                         Represents how 'separate' the xforms are interpolated. Set to 1 for each\n"
																												 											  "\t                         xform to be interpolated individually, fractions control interpolation overlap [default: 0].\n"));
		INITDOUBLEOPTION(AvgThresh,    Eod(OPT_USE_GENOME,  OPT_AVG_THRESH,       _T("--avg"),                  20.0,                 SO_REQ_SEP, "\t--avg=<val>              Minimum average pixel channel sum (r + g + b) threshold from 0 - 765. Ignored if sequence, inter or rotate were specified [default: 20].\n"));
		INITDOUBLEOPTION(BlackThresh,  Eod(OPT_USE_GENOME,  OPT_BLACK_THRESH,     _T("--black"),                0.01,                 SO_REQ_SEP, "\t--black=<val>            Minimum number of allowed black pixels as a percentage from 0 - 1. Ignored if sequence, inter or rotate were specified [default: 0.01].\n"));
		INITDOUBLEOPTION(WhiteLimit,   Eod(OPT_USE_GENOME,  OPT_WHITE_LIMIT,      _T("--white"),                0.05,                 SO_REQ_SEP, "\t--white=<val>            Maximum number of allowed white pixels as a percentage from 0 - 1. Ignored if sequence, inter or rotate were specified [default: 0.05].\n"));
		INITDOUBLEOPTION(Speed,        Eod(OPT_USE_GENOME,  OPT_SPEED,            _T("--speed"),                0.1,                  SO_REQ_SEP, "\t--speed=<val>            Speed as a percentage from 0 - 1 that the affine transform of an existing flame mutates with the new flame. Ignored if sequence, inter or rotate were specified [default: 0.1].\n"));
		INITDOUBLEOPTION(OffsetX,      Eod(OPT_USE_GENOME,  OPT_OFFSETX,          _T("--offsetx"),              0.0,                  SO_REQ_SEP, "\t--offsetx=<val>          Amount to jitter each flame horizontally when applying genome tools [default: 0].\n"));
		INITDOUBLEOPTION(OffsetY,      Eod(OPT_USE_GENOME,  OPT_OFFSETY,          _T("--offsety"),              0.0,                  SO_REQ_SEP, "\t--offsety=<val>          Amount to jitter each flame vertically when applying genome tools [default: 0].\n"));
		INITDOUBLEOPTION(UseMem,       Eod(OPT_USE_RENDER,  OPT_USEMEM,           _T("--use_mem"),              0.0,                  SO_REQ_SEP, "\t--use_mem=<val>          Number of bytes of memory to use [default: max system memory].\n"));

		//String.
		INITSTRINGOPTION(IsaacSeed,    Eos(OPT_USE_ALL,     OPT_ISAAC_SEED,       _T("--isaac_seed"),           "",                   SO_REQ_SEP, "\t--isaac_seed=<val>       Character-based seed for the random number generator [default: random].\n"));
		INITSTRINGOPTION(Input,        Eos(OPT_RENDER_ANIM, OPT_IN,               _T("--in"),                   "",                   SO_REQ_SEP, "\t--in=<val>               Name of the input file.\n"));
		INITSTRINGOPTION(Out,          Eos(OPT_RENDER_ANIM, OPT_OUT,              _T("--out"),                  "",                   SO_REQ_SEP, "\t--out=<val>              Name of a single output file. Not recommended when rendering more than one image.\n"));
		INITSTRINGOPTION(Prefix,       Eos(OPT_RENDER_ANIM, OPT_PREFIX,           _T("--prefix"),               "",                   SO_REQ_SEP, "\t--prefix=<val>           Prefix to prepend to all output files.\n"));
		INITSTRINGOPTION(Suffix,       Eos(OPT_RENDER_ANIM, OPT_SUFFIX,           _T("--suffix"),               "",                   SO_REQ_SEP, "\t--suffix=<val>           Suffix to append to all output files.\n"));
		INITSTRINGOPTION(Format,       Eos(OPT_RENDER_ANIM, OPT_FORMAT,           _T("--format"),               "png",                SO_REQ_SEP, "\t--format=<val>           Format of the output file. Valid values are: bmp, jpg, png, ppm [default: jpg].\n"));
		INITSTRINGOPTION(PalettePath,  Eos(OPT_USE_ALL,     OPT_PALETTE_FILE,     _T("--flam3_palettes"),       "flam3-palettes.xml", SO_REQ_SEP, "\t--flam3_palettes=<val>   Path and name of the palette file [default: flam3-palettes.xml].\n"));
		//INITSTRINGOPTION(PaletteImage, Eos(OPT_USE_ALL,     OPT_PALETTE_IMAGE,    _T("--image"),                "",                   SO_REQ_SEP, "\t--image=<val>            Replace palette with png, jpg, or ppm image.\n"));
		INITSTRINGOPTION(Id,           Eos(OPT_USE_ALL,     OPT_ID,               _T("--id"),                   "",                   SO_REQ_SEP, "\t--id=<val>               ID to use in <edit> tags / image comments.\n"));
		INITSTRINGOPTION(Url,          Eos(OPT_USE_ALL,     OPT_URL,              _T("--url"),                  "",                   SO_REQ_SEP, "\t--url=<val>              URL to use in <edit> tags / image comments.\n"));
		INITSTRINGOPTION(Nick,         Eos(OPT_USE_ALL,     OPT_NICK,             _T("--nick"),                 "",                   SO_REQ_SEP, "\t--nick=<val>             Nickname to use in <edit> tags / image comments.\n"));
		INITSTRINGOPTION(Comment,      Eos(OPT_USE_GENOME,  OPT_COMMENT,          _T("--comment"),              "",                   SO_REQ_SEP, "\t--comment=<val>          Comment to use in <edit> tags.\n"));

		INITSTRINGOPTION(TemplateFile, Eos(OPT_USE_GENOME,  OPT_TEMPLATE,         _T("--template"),             "",                   SO_REQ_SEP, "\t--template=<val>         Apply defaults based on this flame.\n"));
		INITSTRINGOPTION(Clone,        Eos(OPT_USE_GENOME,  OPT_CLONE,            _T("--clone"),                "",                   SO_REQ_SEP, "\t--clone=<val>            Clone random flame in input.\n"));
		INITSTRINGOPTION(CloneAll,     Eos(OPT_USE_GENOME,  OPT_CLONE_ALL,        _T("--clone_all"),            "",                   SO_REQ_SEP, "\t--clone_all=<val>        Clones all flames in the input file. Useful for applying template to all flames.\n"));
		INITSTRINGOPTION(CloneAction,  Eos(OPT_USE_GENOME,  OPT_CLONE_ACTION,     _T("--clone_action"),         "",                   SO_REQ_SEP, "\t--clone_action=<val>     A description of the clone action taking place.\n"));
		INITSTRINGOPTION(Animate,      Eos(OPT_USE_GENOME,  OPT_ANIMATE,          _T("--animate"),              "",                   SO_REQ_SEP, "\t--animate=<val>          Interpolates between all flames in the input file, using times specified in file.\n"));
		INITSTRINGOPTION(Mutate,       Eos(OPT_USE_GENOME,  OPT_MUTATE,           _T("--mutate"),               "",                   SO_REQ_SEP, "\t--mutate=<val>           Randomly mutate a random flame from the input file.\n"));
		INITSTRINGOPTION(Cross0,       Eos(OPT_USE_GENOME,  OPT_CROSS0,           _T("--cross0"),               "",                   SO_REQ_SEP, "\t--cross0=<val>           Randomly select one flame from the input file to genetically cross...\n"));
		INITSTRINGOPTION(Cross1,       Eos(OPT_USE_GENOME,  OPT_CROSS1,           _T("--cross1"),               "",                   SO_REQ_SEP, "\t--cross1=<val>           ...with one flame from this file.\n"));
		INITSTRINGOPTION(Method,       Eos(OPT_USE_GENOME,  OPT_METHOD,           _T("--method"),               "",                   SO_REQ_SEP, "\t--method=<val>           Method used for genetic cross: alternate, interpolate, or union. For mutate: all_vars, one_xform, add_symmetry, post_xforms, color_palette, delete_xform, all_coefs [default: random].\n"));//Original ommitted this important documentation for mutate!
		INITSTRINGOPTION(Inter,        Eos(OPT_USE_GENOME,  OPT_INTER,            _T("--inter"),                "",                   SO_REQ_SEP, "\t--inter=<val>            Interpolate the input file.\n"));
		INITSTRINGOPTION(Rotate,       Eos(OPT_USE_GENOME,  OPT_ROTATE,           _T("--rotate"),               "",                   SO_REQ_SEP, "\t--rotate=<val>           Rotate the input file.\n"));
		INITSTRINGOPTION(Strip,        Eos(OPT_USE_GENOME,  OPT_STRIP,            _T("--strip"),                "",                   SO_REQ_SEP, "\t--strip=<val>            Break strip out of each flame in the input file.\n"));
		INITSTRINGOPTION(Sequence,     Eos(OPT_USE_GENOME,  OPT_SEQUENCE,         _T("--sequence"),             "",                   SO_REQ_SEP, "\t--sequence=<val>         360 degree rotation 'loops' times of each control point in the input file plus rotating transitions.\n"));
		INITSTRINGOPTION(UseVars,      Eos(OPT_USE_GENOME,  OPT_USE_VARS,         _T("--use_vars"),             "",                   SO_REQ_SEP, "\t--use_vars=<val>         Comma separated list of variation #'s to use when generating a random flame.\n"));
		INITSTRINGOPTION(DontUseVars,  Eos(OPT_USE_GENOME,  OPT_DONT_USE_VARS,    _T("--dont_use_vars"),        "",                   SO_REQ_SEP, "\t--dont_use_vars=<val>    Comma separated list of variation #'s to NOT use when generating a random flame.\n"));
		INITSTRINGOPTION(Extras,       Eos(OPT_USE_GENOME,  OPT_EXTRAS,           _T("--extras"),               "",                   SO_REQ_SEP, "\t--extras=<val>           Extra attributes to place in the flame section of the Xml.\n"));
	}

	/// <summary>
	/// Parse and populate the supplied command line options for the specified program usage.
	/// If --help or --version were specified, information will be printed
	/// and parsing will cease.
	/// </summary>
	/// <param name="argc">The number of command line arguments passed</param>
	/// <param name="argv">The command line arguments passed</param>
	/// <param name="optUsage">The program for which these options are to be parsed and used.</param>
	/// <returns>True if --help or --version specified, else false</returns>
	bool Populate(int argc, _TCHAR* argv[], eOptionUse optUsage)
	{
		EmberOptions options;
		vector<CSimpleOpt::SOption> sOptions = options.GetSimpleOptions();
		CSimpleOpt args(argc, argv, sOptions.data());

		//Process args.
		while (args.Next())
		{
			ESOError errorCode = args.LastError();

			if (errorCode == SO_SUCCESS)
			{
				switch (args.OptionId())
				{
					case OPT_HELP://Bool args.
					{
						ShowUsage(optUsage);
						return true;
					}
					case OPT_VERSION:
					{
						cout << EmberVersion() << endl;
						return true;
					}
					PARSEBOOLOPTION(OPT_VERBOSE, Verbose);
					PARSEBOOLOPTION(OPT_DEBUG, Debug);
					PARSEBOOLOPTION(OPT_DUMP_ARGS, DumpArgs);
					PARSEBOOLOPTION(OPT_PROGRESS, DoProgress);
					PARSEBOOLOPTION(OPT_DUMP_OPENCL_INFO, OpenCLInfo);
					PARSEBOOLOPTION(OPT_OPENCL, EmberCL);
					PARSEBOOLOPTION(OPT_EARLYCLIP, EarlyClip);
					PARSEBOOLOPTION(OPT_POS_Y_UP, YAxisUp);
					PARSEBOOLOPTION(OPT_TRANSPARENCY, Transparency);
					PARSEBOOLOPTION(OPT_NAME_ENABLE, NameEnable);
					PARSEBOOLOPTION(OPT_INT_PALETTE, IntPalette);
					PARSEBOOLOPTION(OPT_HEX_PALETTE, HexPalette);
					PARSEBOOLOPTION(OPT_INSERT_PALETTE, InsertPalette);
					PARSEBOOLOPTION(OPT_JPEG_COMMENTS, JpegComments);
					PARSEBOOLOPTION(OPT_PNG_COMMENTS, PngComments);
					PARSEBOOLOPTION(OPT_WRITE_GENOME, WriteGenome);
					PARSEBOOLOPTION(OPT_ENCLOSED, Enclosed);
					PARSEBOOLOPTION(OPT_NO_EDITS, NoEdits);
					PARSEBOOLOPTION(OPT_UNSMOOTH_EDGE, UnsmoothEdge);
					PARSEBOOLOPTION(OPT_LOCK_ACCUM, LockAccum);
					PARSEBOOLOPTION(OPT_DUMP_KERNEL, DumpKernel);

					PARSEINTOPTION(OPT_SYMMETRY, Symmetry);//Int args
					PARSEINTOPTION(OPT_SHEEP_GEN, SheepGen);
					PARSEINTOPTION(OPT_SHEEP_ID, SheepId);
					PARSEUINTOPTION(OPT_OPENCL_PLATFORM, Platform);//Unsigned int args.
					PARSEUINTOPTION(OPT_OPENCL_DEVICE, Device);
					PARSEUINTOPTION(OPT_SEED, Seed);
					PARSEUINTOPTION(OPT_NTHREADS, ThreadCount);
					PARSEUINTOPTION(OPT_STRIPS, Strips);
					PARSEUINTOPTION(OPT_SUPERSAMPLE, Supersample);
					PARSEUINTOPTION(OPT_BITS, Bits);
					PARSEUINTOPTION(OPT_BPC, BitsPerChannel);
					PARSEUINTOPTION(OPT_SBS, SubBatchSize);
					PARSEUINTOPTION(OPT_PRINT_EDIT_DEPTH, PrintEditDepth);
					PARSEUINTOPTION(OPT_JPEG, JpegQuality);
					PARSEUINTOPTION(OPT_BEGIN, FirstFrame);
					PARSEUINTOPTION(OPT_END, LastFrame);
					PARSEUINTOPTION(OPT_FRAME, Frame);
					PARSEUINTOPTION(OPT_TIME, Time);
					PARSEUINTOPTION(OPT_DTIME, Dtime);
					PARSEUINTOPTION(OPT_NFRAMES, Frames);
					PARSEUINTOPTION(OPT_LOOPS, Loops);
					PARSEUINTOPTION(OPT_REPEAT, Repeat);
					PARSEUINTOPTION(OPT_TRIES, Tries);
					PARSEUINTOPTION(OPT_MAX_XFORMS, MaxXforms);

					PARSEDOUBLEOPTION(OPT_SS, SizeScale);//Float args.
					PARSEDOUBLEOPTION(OPT_QS, QualityScale);
					PARSEDOUBLEOPTION(OPT_PIXEL_ASPECT, AspectRatio);
					PARSEDOUBLEOPTION(OPT_STAGGER, Stagger);
					PARSEDOUBLEOPTION(OPT_AVG_THRESH, AvgThresh);
					PARSEDOUBLEOPTION(OPT_BLACK_THRESH, BlackThresh);
					PARSEDOUBLEOPTION(OPT_WHITE_LIMIT, WhiteLimit);
					PARSEDOUBLEOPTION(OPT_SPEED, Speed);
					PARSEDOUBLEOPTION(OPT_OFFSETX, OffsetX);
					PARSEDOUBLEOPTION(OPT_OFFSETY, OffsetY);
					PARSEDOUBLEOPTION(OPT_USEMEM, UseMem);

					PARSESTRINGOPTION(OPT_ISAAC_SEED, IsaacSeed);//String args.
					PARSESTRINGOPTION(OPT_IN, Input);
					PARSESTRINGOPTION(OPT_OUT, Out);
					PARSESTRINGOPTION(OPT_PREFIX, Prefix);
					PARSESTRINGOPTION(OPT_SUFFIX, Suffix);
					PARSESTRINGOPTION(OPT_FORMAT, Format);
					PARSESTRINGOPTION(OPT_PALETTE_FILE, PalettePath);
					//PARSESTRINGOPTION(OPT_PALETTE_IMAGE, PaletteImage);
					PARSESTRINGOPTION(OPT_ID, Id);
					PARSESTRINGOPTION(OPT_URL, Url);
					PARSESTRINGOPTION(OPT_NICK, Nick);
					PARSESTRINGOPTION(OPT_COMMENT, Comment);
					PARSESTRINGOPTION(OPT_TEMPLATE, TemplateFile);
					PARSESTRINGOPTION(OPT_CLONE, Clone);
					PARSESTRINGOPTION(OPT_CLONE_ALL, CloneAll);
					PARSESTRINGOPTION(OPT_CLONE_ACTION, CloneAction);
					PARSESTRINGOPTION(OPT_ANIMATE, Animate);
					PARSESTRINGOPTION(OPT_MUTATE, Mutate);
					PARSESTRINGOPTION(OPT_CROSS0, Cross0);
					PARSESTRINGOPTION(OPT_CROSS1, Cross1);
					PARSESTRINGOPTION(OPT_METHOD, Method);
					PARSESTRINGOPTION(OPT_INTER, Inter);
					PARSESTRINGOPTION(OPT_ROTATE, Rotate);
					PARSESTRINGOPTION(OPT_STRIP, Strip);
					PARSESTRINGOPTION(OPT_SEQUENCE, Sequence);
					PARSESTRINGOPTION(OPT_USE_VARS, UseVars);
					PARSESTRINGOPTION(OPT_DONT_USE_VARS, DontUseVars);
					PARSESTRINGOPTION(OPT_EXTRAS, Extras);
					default:
					{
						break;//Do nothing.
					}
				}
			}
			else
			{
				cout << "Invalid argument: " << args.OptionText() << endl;
				cout << "\tReason: " << GetLastErrorText(errorCode) << endl;
			}
		}

		return false;
	}

	/// <summary>
	/// Return a vector of all available options for the specified program.
	/// </summary>
	/// <param name="optUsage">The specified program usage</param>
	/// <returns>A vector of all available options for the specified program</returns>
	vector<CSimpleOpt::SOption> GetSimpleOptions(eOptionUse optUsage = OPT_USE_ALL)
	{
		vector<CSimpleOpt::SOption> entries;
		CSimpleOpt::SOption endOption = SO_END_OF_OPTIONS;
		entries.reserve(75);

		ForEach(m_BoolArgs,   [&](Eob* entry) { if (entry->m_OptionUse & optUsage) entries.push_back(entry->m_Option); });
		ForEach(m_IntArgs,    [&](Eoi* entry) { if (entry->m_OptionUse & optUsage) entries.push_back(entry->m_Option); });
		ForEach(m_UintArgs,   [&](Eou* entry) { if (entry->m_OptionUse & optUsage) entries.push_back(entry->m_Option); });
		ForEach(m_DoubleArgs, [&](Eod* entry) { if (entry->m_OptionUse & optUsage) entries.push_back(entry->m_Option); });
		ForEach(m_StringArgs, [&](Eos* entry) { if (entry->m_OptionUse & optUsage) entries.push_back(entry->m_Option); });

		entries.push_back(endOption);
		return entries;
	}

	/// <summary>
	/// Return a string with the descriptions of all available options for the specified program.
	/// </summary>
	/// <param name="optUsage">The specified program usage</param>
	/// <returns>A string with the descriptions of all available options for the specified program</returns>
	string GetUsage(eOptionUse optUsage = OPT_USE_ALL)
	{
		ostringstream os;

		ForEach(m_BoolArgs,   [&](Eob* entry) { if (entry->m_OptionUse & optUsage) os << entry->m_DocString << endl; });
		ForEach(m_IntArgs,    [&](Eoi* entry) { if (entry->m_OptionUse & optUsage) os << entry->m_DocString << endl; });
		ForEach(m_UintArgs,   [&](Eou* entry) { if (entry->m_OptionUse & optUsage) os << entry->m_DocString << endl; });
		ForEach(m_DoubleArgs, [&](Eod* entry) { if (entry->m_OptionUse & optUsage) os << entry->m_DocString << endl; });
		ForEach(m_StringArgs, [&](Eos* entry) { if (entry->m_OptionUse & optUsage) os << entry->m_DocString << endl; });

		return os.str();
	}

	/// <summary>
	/// Return a string with all of the names and values for all available options for the specified program.
	/// </summary>
	/// <param name="optUsage">The specified program usage</param>
	/// <returns>A string with all of the names and values for all available options for the specified program</returns>
	string GetValues(eOptionUse optUsage = OPT_USE_ALL)
	{
		ostringstream os;

		os << std::boolalpha;
		ForEach(m_BoolArgs,   [&](Eob* entry) { if (entry->m_OptionUse & optUsage) os << entry->m_NameWithoutDashes << ": " << (*entry)() << endl; });
		ForEach(m_IntArgs,    [&](Eoi* entry) { if (entry->m_OptionUse & optUsage) os << entry->m_NameWithoutDashes << ": " << (*entry)() << endl; });
		ForEach(m_UintArgs,   [&](Eou* entry) { if (entry->m_OptionUse & optUsage) os << entry->m_NameWithoutDashes << ": " << (*entry)() << endl; });
		ForEach(m_DoubleArgs, [&](Eod* entry) { if (entry->m_OptionUse & optUsage) os << entry->m_NameWithoutDashes << ": " << (*entry)() << endl; });
		ForEach(m_StringArgs, [&](Eos* entry) { if (entry->m_OptionUse & optUsage) os << entry->m_NameWithoutDashes << ": " << (*entry)() << endl; });

		return os.str();
	}

	/// <summary>
	/// Print description string, version and description of all available options for the specified program.
	/// </summary>
	/// <param name="optUsage">The specified program usage</param>
	void ShowUsage(eOptionUse optUsage)
	{
		cout << DescriptionString << " version " << EmberVersion() << endl << endl;

		if (optUsage == OPT_USE_RENDER)
		{
			cout << "Usage:\n"
				"\tEmberRender.exe --in=test.flam3 [--out=outfile --format=png --verbose --progress --opencl]\n" << endl;
		}
		else if (optUsage == OPT_USE_ANIMATE)
		{
			cout << "Usage:\n"
				"\tEmberAnimate.exe --in=sequence.flam3 [--format=png --verbose --progress --opencl]\n" << endl;
		}
		else if (optUsage == OPT_USE_GENOME)
		{
			cout << "Usage:\n"
				"\tEmberGenome.exe --sequence=test.flam3 > sequenceout.flam3\n" << endl;
		}

		cout << GetUsage(optUsage) << endl;
	}

	/// <summary>
	/// Return the last option parsing error text as a string.
	/// </summary>
	/// <param name="errorCode">The code of the last parsing error</param>
	/// <returns>The last option parsing error text as a string</returns>
	string GetLastErrorText(int errorCode)
	{
		switch (errorCode)
		{
			case SO_SUCCESS:            return "Success";
			case SO_OPT_INVALID:        return "Unrecognized option";
			case SO_OPT_MULTIPLE:       return "Option matched multiple strings";
			case SO_ARG_INVALID:        return "Option does not accept argument";
			case SO_ARG_INVALID_TYPE:   return "Invalid argument format";
			case SO_ARG_MISSING:        return "Required argument is missing";
			case SO_ARG_INVALID_DATA:   return "Invalid argument data";
			default:                    return "Unknown error";
		}
	}

	//Break from the usual m_* notation for members here because
	//each of these is a functor, so it looks nicer and is less typing
	//to just say opt.Member().
	EmberOptionEntry<bool> Help;//Diagnostic bool.
	EmberOptionEntry<bool> Version;
	EmberOptionEntry<bool> Verbose;
	EmberOptionEntry<bool> Debug;
	EmberOptionEntry<bool> DumpArgs;
	EmberOptionEntry<bool> DoProgress;
	EmberOptionEntry<bool> OpenCLInfo;

	EmberOptionEntry<bool> EmberCL;//Value bool.
	EmberOptionEntry<bool> EarlyClip;
	EmberOptionEntry<bool> YAxisUp;
	EmberOptionEntry<bool> Transparency;
	EmberOptionEntry<bool> NameEnable;
	EmberOptionEntry<bool> IntPalette;
	EmberOptionEntry<bool> HexPalette;
	EmberOptionEntry<bool> InsertPalette;
	EmberOptionEntry<bool> JpegComments;
	EmberOptionEntry<bool> PngComments;
	EmberOptionEntry<bool> WriteGenome;
	EmberOptionEntry<bool> Enclosed;
	EmberOptionEntry<bool> NoEdits;
	EmberOptionEntry<bool> UnsmoothEdge;
	EmberOptionEntry<bool> LockAccum;
	EmberOptionEntry<bool> DumpKernel;

	EmberOptionEntry<int> Symmetry;//Value int.
	EmberOptionEntry<int> SheepGen;
	EmberOptionEntry<int> SheepId;
	EmberOptionEntry<unsigned int> Platform;//Value unsigned int.
	EmberOptionEntry<unsigned int> Device;
	EmberOptionEntry<unsigned int> Seed;
	EmberOptionEntry<unsigned int> ThreadCount;
	EmberOptionEntry<unsigned int> Strips;
	EmberOptionEntry<unsigned int> Supersample;
	EmberOptionEntry<unsigned int> BitsPerChannel;
	EmberOptionEntry<unsigned int> SubBatchSize;
	EmberOptionEntry<unsigned int> Bits;
	EmberOptionEntry<unsigned int> PrintEditDepth;
	EmberOptionEntry<unsigned int> JpegQuality;
	EmberOptionEntry<unsigned int> FirstFrame;
	EmberOptionEntry<unsigned int> LastFrame;
	EmberOptionEntry<unsigned int> Frame;
	EmberOptionEntry<unsigned int> Time;
	EmberOptionEntry<unsigned int> Dtime;
	EmberOptionEntry<unsigned int> Frames;
	EmberOptionEntry<unsigned int> Loops;
	EmberOptionEntry<unsigned int> Repeat;
	EmberOptionEntry<unsigned int> Tries;
	EmberOptionEntry<unsigned int> MaxXforms;

	EmberOptionEntry<double> SizeScale;//Value double.
	EmberOptionEntry<double> QualityScale;
	EmberOptionEntry<double> AspectRatio;
	EmberOptionEntry<double> Stagger;
	EmberOptionEntry<double> AvgThresh;
	EmberOptionEntry<double> BlackThresh;
	EmberOptionEntry<double> WhiteLimit;
	EmberOptionEntry<double> Speed;
	EmberOptionEntry<double> OffsetX;
	EmberOptionEntry<double> OffsetY;
	EmberOptionEntry<double> UseMem;

	EmberOptionEntry<string> IsaacSeed;//Value string.
	EmberOptionEntry<string> Input;
	EmberOptionEntry<string> Out;
	EmberOptionEntry<string> Prefix;
	EmberOptionEntry<string> Suffix;
	EmberOptionEntry<string> Format;
	EmberOptionEntry<string> PalettePath;
	//EmberOptionEntry<string> PaletteImage;
	EmberOptionEntry<string> Id;
	EmberOptionEntry<string> Url;
	EmberOptionEntry<string> Nick;
	EmberOptionEntry<string> Comment;
	EmberOptionEntry<string> TemplateFile;
	EmberOptionEntry<string> Clone;
	EmberOptionEntry<string> CloneAll;
	EmberOptionEntry<string> CloneAction;
	EmberOptionEntry<string> Animate;
	EmberOptionEntry<string> Mutate;
	EmberOptionEntry<string> Cross0;
	EmberOptionEntry<string> Cross1;
	EmberOptionEntry<string> Method;
	EmberOptionEntry<string> Inter;
	EmberOptionEntry<string> Rotate;
	EmberOptionEntry<string> Strip;
	EmberOptionEntry<string> Sequence;
	EmberOptionEntry<string> UseVars;
	EmberOptionEntry<string> DontUseVars;
	EmberOptionEntry<string> Extras;

private:
	vector<EmberOptionEntry<bool>*> m_BoolArgs;
	vector<EmberOptionEntry<int>*> m_IntArgs;
	vector<EmberOptionEntry<unsigned int>*> m_UintArgs;
	vector<EmberOptionEntry<double>*> m_DoubleArgs;
	vector<EmberOptionEntry<string>*> m_StringArgs;
};
