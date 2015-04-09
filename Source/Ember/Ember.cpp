#include "EmberPch.h"
#include "EmberDefines.h"
#include "Isaac.h"
#include "Curves.h"
#include "Ember.h"
#include "Utils.h"
#include "Iterator.h"
#include "Palette.h"
#include "PaletteList.h"
#include "Point.h"
#include "Variation.h"
#include "Variations01.h"
#include "Variations02.h"
#include "Variations03.h"
#include "Variations04.h"
#include "Variations05.h"
#include "VariationsDC.h"
#include "VariationList.h"
#include "Affine2D.h"
#include "Xform.h"
#include "EmberToXml.h"
#include "XmlToEmber.h"
#include "SpatialFilter.h"
#include "DensityFilter.h"
#include "TemporalFilter.h"
#include "Interpolate.h"
#include "Renderer.h"
#include "Timing.h"
#include "SheepTools.h"

/// <summary>
/// Explicit instantiation of all templated classes which aren't implemented in cpp files.
/// All new templated classes, such as new variations, must be added here.
/// Additional instances of static class member variables.
/// </summary>

namespace EmberNs
{
bool Timing::m_TimingInit = false;
uint Timing::m_ProcessorCount;
template<> unique_ptr<QTIsaac<ISAAC_SIZE, ISAAC_INT>> QTIsaac<ISAAC_SIZE, ISAAC_INT>::GlobalRand = unique_ptr<QTIsaac<ISAAC_SIZE, ISAAC_INT>>(new QTIsaac<ISAAC_SIZE, ISAAC_INT>());

#define EXPORTPREPOSTREGVAR(varName, T) \
	template EMBER_API class varName##Variation<T>; \
	template EMBER_API class Pre##varName##Variation<T>; \
	template EMBER_API class Post##varName##Variation<T>;

#define EXPORT_SINGLE_TYPE_EMBER(T) \
	template<> const char* PaletteList<T>::m_DefaultFilename = "flam3-palettes.xml"; \
	template<> map<string, vector<Palette<T>>> PaletteList<T>::m_Palettes = map<string, vector<Palette<T>>>(); \
	template<> bool XmlToEmber<T>::m_Init = false; \
	template<> vector<string> XmlToEmber<T>::m_FlattenNames = vector<string>(); \
	template<> vector<pair<string, string>> XmlToEmber<T>::m_BadParamNames = vector<pair<string, string>>(); \
	template<> vector<pair<pair<string, string>, vector<string>>> XmlToEmber<T>::m_BadVariationNames = vector<pair<pair<string, string>, vector<string>>>(); \
	template EMBER_API class Point<T>; \
	template EMBER_API struct Color<T>; \
	template EMBER_API class Palette<T>; \
	template EMBER_API class PaletteList<T>; \
	template EMBER_API class Iterator<T>; \
	template EMBER_API class StandardIterator<T>; \
	template EMBER_API class XaosIterator<T>; \
	template EMBER_API class Xform<T>; \
	template EMBER_API class IteratorHelper<T>; \
	template EMBER_API class Variation<T>; \
	template EMBER_API class ParamWithName<T>; \
	template EMBER_API class ParametricVariation<T>; \
	EXPORTPREPOSTREGVAR(Linear, T) \
	EXPORTPREPOSTREGVAR(Sinusoidal, T) \
	EXPORTPREPOSTREGVAR(Spherical, T) \
	EXPORTPREPOSTREGVAR(Swirl, T) \
	EXPORTPREPOSTREGVAR(Horseshoe, T) \
	EXPORTPREPOSTREGVAR(Polar, T) \
	EXPORTPREPOSTREGVAR(Handkerchief, T) \
	EXPORTPREPOSTREGVAR(Heart, T) \
	EXPORTPREPOSTREGVAR(Disc, T) \
	EXPORTPREPOSTREGVAR(Spiral, T) \
	EXPORTPREPOSTREGVAR(Hyperbolic, T) \
	EXPORTPREPOSTREGVAR(Diamond, T) \
	EXPORTPREPOSTREGVAR(Ex, T) \
	EXPORTPREPOSTREGVAR(Julia, T) \
	EXPORTPREPOSTREGVAR(Bent, T) \
	EXPORTPREPOSTREGVAR(Waves, T) \
	EXPORTPREPOSTREGVAR(Fisheye, T) \
	EXPORTPREPOSTREGVAR(Popcorn, T) \
	EXPORTPREPOSTREGVAR(Exponential, T) \
	EXPORTPREPOSTREGVAR(Power, T) \
	EXPORTPREPOSTREGVAR(Cosine, T) \
	EXPORTPREPOSTREGVAR(Rings, T) \
	EXPORTPREPOSTREGVAR(Fan, T) \
	EXPORTPREPOSTREGVAR(Blob, T) \
	EXPORTPREPOSTREGVAR(Pdj, T) \
	EXPORTPREPOSTREGVAR(Fan2, T) \
	EXPORTPREPOSTREGVAR(Rings2, T) \
	EXPORTPREPOSTREGVAR(Eyefish, T) \
	EXPORTPREPOSTREGVAR(Bubble, T) \
	EXPORTPREPOSTREGVAR(Cylinder, T) \
	EXPORTPREPOSTREGVAR(Perspective, T) \
	EXPORTPREPOSTREGVAR(Noise, T) \
	EXPORTPREPOSTREGVAR(JuliaNGeneric, T) \
	EXPORTPREPOSTREGVAR(JuliaScope, T) \
	EXPORTPREPOSTREGVAR(Blur, T) \
	EXPORTPREPOSTREGVAR(GaussianBlur, T) \
	EXPORTPREPOSTREGVAR(RadialBlur, T) \
	EXPORTPREPOSTREGVAR(Pie, T) \
	EXPORTPREPOSTREGVAR(Ngon, T) \
	EXPORTPREPOSTREGVAR(Curl, T) \
	EXPORTPREPOSTREGVAR(Rectangles, T) \
	EXPORTPREPOSTREGVAR(Arch, T) \
	EXPORTPREPOSTREGVAR(Tangent, T) \
	EXPORTPREPOSTREGVAR(Square, T) \
	EXPORTPREPOSTREGVAR(Rays, T) \
	EXPORTPREPOSTREGVAR(Blade, T) \
	EXPORTPREPOSTREGVAR(Secant2, T) \
	EXPORTPREPOSTREGVAR(TwinTrian, T) \
	EXPORTPREPOSTREGVAR(Cross, T) \
	EXPORTPREPOSTREGVAR(Disc2, T) \
	EXPORTPREPOSTREGVAR(SuperShape, T) \
	EXPORTPREPOSTREGVAR(Flower, T) \
	EXPORTPREPOSTREGVAR(Conic, T) \
	EXPORTPREPOSTREGVAR(Parabola, T) \
	EXPORTPREPOSTREGVAR(Bent2, T) \
	EXPORTPREPOSTREGVAR(Bipolar, T) \
	EXPORTPREPOSTREGVAR(Boarders, T) \
	EXPORTPREPOSTREGVAR(Butterfly, T) \
	EXPORTPREPOSTREGVAR(Cell, T) \
	EXPORTPREPOSTREGVAR(Cpow, T) \
	EXPORTPREPOSTREGVAR(Curve, T) \
	EXPORTPREPOSTREGVAR(Edisc, T) \
	EXPORTPREPOSTREGVAR(Elliptic, T) \
	EXPORTPREPOSTREGVAR(Escher, T) \
	EXPORTPREPOSTREGVAR(Foci, T) \
	EXPORTPREPOSTREGVAR(LazySusan, T) \
	EXPORTPREPOSTREGVAR(Loonie, T) \
	EXPORTPREPOSTREGVAR(Modulus, T) \
	EXPORTPREPOSTREGVAR(Oscilloscope, T) \
	EXPORTPREPOSTREGVAR(Polar2, T) \
	EXPORTPREPOSTREGVAR(Popcorn2, T) \
	EXPORTPREPOSTREGVAR(Scry, T) \
	EXPORTPREPOSTREGVAR(Separation, T) \
	EXPORTPREPOSTREGVAR(Split, T) \
	EXPORTPREPOSTREGVAR(Splits, T) \
	EXPORTPREPOSTREGVAR(Stripes, T) \
	EXPORTPREPOSTREGVAR(Wedge, T) \
	EXPORTPREPOSTREGVAR(WedgeJulia, T) \
	EXPORTPREPOSTREGVAR(WedgeSph, T) \
	EXPORTPREPOSTREGVAR(Whorl, T) \
	EXPORTPREPOSTREGVAR(Waves2, T) \
	EXPORTPREPOSTREGVAR(Exp, T) \
	EXPORTPREPOSTREGVAR(Log, T) \
	EXPORTPREPOSTREGVAR(Sin, T) \
	EXPORTPREPOSTREGVAR(Cos, T) \
	EXPORTPREPOSTREGVAR(Tan, T) \
	EXPORTPREPOSTREGVAR(Sec, T) \
	EXPORTPREPOSTREGVAR(Csc, T) \
	EXPORTPREPOSTREGVAR(Cot, T) \
	EXPORTPREPOSTREGVAR(Sinh, T) \
	EXPORTPREPOSTREGVAR(Cosh, T) \
	EXPORTPREPOSTREGVAR(Tanh, T) \
	EXPORTPREPOSTREGVAR(Sech, T) \
	EXPORTPREPOSTREGVAR(Csch, T) \
	EXPORTPREPOSTREGVAR(Coth, T) \
	EXPORTPREPOSTREGVAR(Auger, T) \
	EXPORTPREPOSTREGVAR(Flux, T) \
	EXPORTPREPOSTREGVAR(Hemisphere, T) \
	EXPORTPREPOSTREGVAR(Epispiral, T) \
	EXPORTPREPOSTREGVAR(Bwraps, T) \
	EXPORTPREPOSTREGVAR(Extrude, T) \
	EXPORTPREPOSTREGVAR(BlurCircle, T) \
	EXPORTPREPOSTREGVAR(BlurZoom, T) \
	EXPORTPREPOSTREGVAR(BlurPixelize, T) \
	EXPORTPREPOSTREGVAR(Crop, T) \
	EXPORTPREPOSTREGVAR(BCircle, T) \
	EXPORTPREPOSTREGVAR(BlurLinear, T) \
	EXPORTPREPOSTREGVAR(BlurSquare, T) \
	EXPORTPREPOSTREGVAR(Boarders2, T) \
	EXPORTPREPOSTREGVAR(Cardioid, T) \
	EXPORTPREPOSTREGVAR(Checks, T) \
	EXPORTPREPOSTREGVAR(Circlize, T) \
	EXPORTPREPOSTREGVAR(Circlize2, T) \
	EXPORTPREPOSTREGVAR(CosWrap, T) \
	EXPORTPREPOSTREGVAR(DeltaA, T) \
	EXPORTPREPOSTREGVAR(Expo, T) \
	EXPORTPREPOSTREGVAR(FDisc, T) \
	EXPORTPREPOSTREGVAR(Fibonacci, T) \
	EXPORTPREPOSTREGVAR(Fibonacci2, T) \
	EXPORTPREPOSTREGVAR(Glynnia, T) \
	EXPORTPREPOSTREGVAR(GridOut, T) \
	EXPORTPREPOSTREGVAR(Hole, T) \
	EXPORTPREPOSTREGVAR(Hypertile, T) \
	EXPORTPREPOSTREGVAR(Hypertile1, T) \
	EXPORTPREPOSTREGVAR(Hypertile2, T) \
	EXPORTPREPOSTREGVAR(Hypertile3D, T) \
	EXPORTPREPOSTREGVAR(Hypertile3D1, T) \
	EXPORTPREPOSTREGVAR(Hypertile3D2, T) \
	EXPORTPREPOSTREGVAR(IDisc, T) \
	EXPORTPREPOSTREGVAR(Julian2, T) \
	EXPORTPREPOSTREGVAR(JuliaQ, T) \
	EXPORTPREPOSTREGVAR(Murl, T) \
	EXPORTPREPOSTREGVAR(Murl2, T) \
	EXPORTPREPOSTREGVAR(NPolar, T) \
	EXPORTPREPOSTREGVAR(Ortho, T) \
	EXPORTPREPOSTREGVAR(Poincare, T) \
	EXPORTPREPOSTREGVAR(Poincare3D, T) \
	EXPORTPREPOSTREGVAR(Polynomial, T) \
	EXPORTPREPOSTREGVAR(PSphere, T) \
	EXPORTPREPOSTREGVAR(Rational3, T) \
	EXPORTPREPOSTREGVAR(Ripple, T) \
	EXPORTPREPOSTREGVAR(Sigmoid, T) \
	EXPORTPREPOSTREGVAR(SinusGrid, T) \
	EXPORTPREPOSTREGVAR(Stwin, T) \
	EXPORTPREPOSTREGVAR(TwoFace, T) \
	EXPORTPREPOSTREGVAR(Unpolar, T) \
	EXPORTPREPOSTREGVAR(WavesN, T) \
	EXPORTPREPOSTREGVAR(XHeart, T) \
	EXPORTPREPOSTREGVAR(Barycentroid, T) \
	EXPORTPREPOSTREGVAR(BiSplit, T) \
	EXPORTPREPOSTREGVAR(Crescents, T) \
	EXPORTPREPOSTREGVAR(Mask, T) \
	EXPORTPREPOSTREGVAR(Cpow2, T) \
	EXPORTPREPOSTREGVAR(Curl3D, T) \
	EXPORTPREPOSTREGVAR(Disc3D, T) \
	EXPORTPREPOSTREGVAR(Funnel, T) \
	EXPORTPREPOSTREGVAR(Linear3D, T) \
	EXPORTPREPOSTREGVAR(PowBlock, T) \
	EXPORTPREPOSTREGVAR(Squirrel, T) \
	EXPORTPREPOSTREGVAR(Ennepers, T) \
	EXPORTPREPOSTREGVAR(SphericalN, T) \
	EXPORTPREPOSTREGVAR(Kaleidoscope, T) \
	EXPORTPREPOSTREGVAR(GlynnSim1, T) \
	EXPORTPREPOSTREGVAR(GlynnSim2, T) \
	EXPORTPREPOSTREGVAR(GlynnSim3, T) \
	EXPORTPREPOSTREGVAR(Starblur, T) \
	EXPORTPREPOSTREGVAR(Sineblur, T) \
	EXPORTPREPOSTREGVAR(Circleblur, T) \
	EXPORTPREPOSTREGVAR(CropN, T) \
	EXPORTPREPOSTREGVAR(ShredRad, T) \
	EXPORTPREPOSTREGVAR(Blob2, T) \
	EXPORTPREPOSTREGVAR(Julia3D, T) \
	EXPORTPREPOSTREGVAR(Julia3Dz, T) \
	EXPORTPREPOSTREGVAR(LinearT, T) \
	EXPORTPREPOSTREGVAR(LinearT3D, T) \
	EXPORTPREPOSTREGVAR(Ovoid, T) \
	EXPORTPREPOSTREGVAR(Ovoid3D, T) \
	EXPORTPREPOSTREGVAR(Spirograph, T) \
	EXPORTPREPOSTREGVAR(Petal, T) \
	EXPORTPREPOSTREGVAR(RoundSpher, T) \
	EXPORTPREPOSTREGVAR(RoundSpher3D, T) \
	EXPORTPREPOSTREGVAR(SpiralWing, T) \
	EXPORTPREPOSTREGVAR(Squarize, T) \
	EXPORTPREPOSTREGVAR(Sschecks, T) \
	EXPORTPREPOSTREGVAR(PhoenixJulia, T) \
	EXPORTPREPOSTREGVAR(Mobius, T) \
	EXPORTPREPOSTREGVAR(MobiusN, T) \
	EXPORTPREPOSTREGVAR(MobiusStrip, T) \
	EXPORTPREPOSTREGVAR(Lissajous, T) \
	EXPORTPREPOSTREGVAR(Svf, T) \
	EXPORTPREPOSTREGVAR(Target, T) \
	EXPORTPREPOSTREGVAR(Taurus, T) \
	EXPORTPREPOSTREGVAR(Collideoscope, T) \
	EXPORTPREPOSTREGVAR(BMod, T) \
	EXPORTPREPOSTREGVAR(BSwirl, T) \
	EXPORTPREPOSTREGVAR(BTransform, T) \
	EXPORTPREPOSTREGVAR(BCollide, T) \
	EXPORTPREPOSTREGVAR(Eclipse, T) \
	EXPORTPREPOSTREGVAR(FlipCircle, T) \
	EXPORTPREPOSTREGVAR(FlipY, T) \
	EXPORTPREPOSTREGVAR(ECollide, T) \
	EXPORTPREPOSTREGVAR(EJulia, T) \
	EXPORTPREPOSTREGVAR(EMod, T) \
	EXPORTPREPOSTREGVAR(EMotion, T) \
	EXPORTPREPOSTREGVAR(EPush, T) \
	EXPORTPREPOSTREGVAR(ERotate, T) \
	EXPORTPREPOSTREGVAR(EScale, T) \
	EXPORTPREPOSTREGVAR(ESwirl, T) \
	EXPORTPREPOSTREGVAR(LazyTravis, T) \
	EXPORTPREPOSTREGVAR(Squish, T) \
	EXPORTPREPOSTREGVAR(Circus, T) \
	EXPORTPREPOSTREGVAR(Tancos, T) \
	EXPORTPREPOSTREGVAR(Rippled, T) \
	EXPORTPREPOSTREGVAR(Flatten, T) \
	EXPORTPREPOSTREGVAR(Zblur, T) \
	EXPORTPREPOSTREGVAR(Blur3D, T) \
	EXPORTPREPOSTREGVAR(ZScale, T) \
	EXPORTPREPOSTREGVAR(ZTranslate, T) \
	EXPORTPREPOSTREGVAR(ZCone, T) \
	EXPORTPREPOSTREGVAR(RotateX, T) \
	EXPORTPREPOSTREGVAR(RotateY, T) \
	EXPORTPREPOSTREGVAR(RotateZ, T) \
	EXPORTPREPOSTREGVAR(MirrorX, T) \
	EXPORTPREPOSTREGVAR(MirrorY, T) \
	EXPORTPREPOSTREGVAR(MirrorZ, T) \
	EXPORTPREPOSTREGVAR(Depth, T) \
	EXPORTPREPOSTREGVAR(RBlur, T) \
	EXPORTPREPOSTREGVAR(JuliaNab, T) \
	EXPORTPREPOSTREGVAR(Sintrange, T) \
	EXPORTPREPOSTREGVAR(Voron, T) \
	EXPORTPREPOSTREGVAR(Waffle, T) \
	EXPORTPREPOSTREGVAR(Square3D, T) \
	EXPORTPREPOSTREGVAR(SuperShape3D, T) \
	EXPORTPREPOSTREGVAR(Sphyp3D, T) \
	EXPORTPREPOSTREGVAR(Circlecrop, T) \
	EXPORTPREPOSTREGVAR(Julian3Dx, T) \
	EXPORTPREPOSTREGVAR(Fourth, T) \
	EXPORTPREPOSTREGVAR(Mobiq, T) \
	EXPORTPREPOSTREGVAR(Spherivoid, T) \
	EXPORTPREPOSTREGVAR(Farblur, T) \
	EXPORTPREPOSTREGVAR(CurlSP, T) \
	EXPORTPREPOSTREGVAR(Heat, T) \
	EXPORTPREPOSTREGVAR(Interference2, T) \
	EXPORTPREPOSTREGVAR(Sinq, T) \
	EXPORTPREPOSTREGVAR(Sinhq, T) \
	EXPORTPREPOSTREGVAR(Secq, T) \
	EXPORTPREPOSTREGVAR(Sechq, T) \
	EXPORTPREPOSTREGVAR(Tanq, T) \
	EXPORTPREPOSTREGVAR(Tanhq, T) \
	EXPORTPREPOSTREGVAR(Cosq, T) \
	EXPORTPREPOSTREGVAR(Coshq, T) \
	EXPORTPREPOSTREGVAR(Cotq, T) \
	EXPORTPREPOSTREGVAR(Cothq, T) \
	EXPORTPREPOSTREGVAR(Cscq, T) \
	EXPORTPREPOSTREGVAR(Cschq, T) \
	EXPORTPREPOSTREGVAR(Estiq, T) \
	EXPORTPREPOSTREGVAR(Loq, T) \
	EXPORTPREPOSTREGVAR(Curvature, T) \
	EXPORTPREPOSTREGVAR(Qode, T) \
	EXPORTPREPOSTREGVAR(BlurHeart, T) \
	EXPORTPREPOSTREGVAR(Truchet, T) \
	EXPORTPREPOSTREGVAR(Gdoffs, T) \
	EXPORTPREPOSTREGVAR(Octagon, T) \
	EXPORTPREPOSTREGVAR(Trade, T) \
	EXPORTPREPOSTREGVAR(Juliac, T) \
	EXPORTPREPOSTREGVAR(Blade3D, T) \
	EXPORTPREPOSTREGVAR(Blob3D, T) \
	EXPORTPREPOSTREGVAR(Blocky, T) \
	EXPORTPREPOSTREGVAR(Bubble2, T) \
	EXPORTPREPOSTREGVAR(CircleLinear, T) \
	EXPORTPREPOSTREGVAR(CircleRand, T) \
	EXPORTPREPOSTREGVAR(CircleTrans1, T) \
	EXPORTPREPOSTREGVAR(Cubic3D, T) \
	EXPORTPREPOSTREGVAR(CubicLattice3D, T) \
	EXPORTPREPOSTREGVAR(Foci3D, T) \
	EXPORTPREPOSTREGVAR(Ho, T) \
	EXPORTPREPOSTREGVAR(Julia3Dq, T) \
	EXPORTPREPOSTREGVAR(Line, T) \
	EXPORTPREPOSTREGVAR(Loonie3D, T) \
	EXPORTPREPOSTREGVAR(Mcarpet, T) \
	EXPORTPREPOSTREGVAR(Waves23D, T) \
	EXPORTPREPOSTREGVAR(Pie3D, T) \
	EXPORTPREPOSTREGVAR(Popcorn23D, T) \
	EXPORTPREPOSTREGVAR(Sinusoidal3D, T) \
	EXPORTPREPOSTREGVAR(Scry3D, T) \
	EXPORTPREPOSTREGVAR(Shredlin, T) \
	EXPORTPREPOSTREGVAR(SplitBrdr, T) \
	EXPORTPREPOSTREGVAR(Wdisc, T) \
	EXPORTPREPOSTREGVAR(Falloff, T) \
	EXPORTPREPOSTREGVAR(Falloff2, T) \
	EXPORTPREPOSTREGVAR(Falloff3, T) \
	EXPORTPREPOSTREGVAR(Xtrb, T) \
	template EMBER_API class DCBubbleVariation<T>; \
	EXPORTPREPOSTREGVAR(DCCarpet, T) \
	EXPORTPREPOSTREGVAR(DCCube, T) \
	template EMBER_API class DCCylinderVariation<T>; \
	EXPORTPREPOSTREGVAR(DCGridOut, T) \
	template EMBER_API class DCLinearVariation<T>; \
	EXPORTPREPOSTREGVAR(DCZTransl, T) \
	EXPORTPREPOSTREGVAR(DCTriangle, T) \
	template EMBER_API class VariationList<T>; \
	template EMBER_API class SpatialFilter<T>; \
	template EMBER_API class GaussianFilter<T>; \
	template EMBER_API class HermiteFilter<T>; \
	template EMBER_API class BoxFilter<T>; \
	template EMBER_API class TriangleFilter<T>; \
	template EMBER_API class BellFilter<T>; \
	template EMBER_API class BsplineFilter<T>; \
	template EMBER_API class MitchellFilter<T>; \
	template EMBER_API class BlackmanFilter<T>; \
	template EMBER_API class CatromFilter<T>; \
	template EMBER_API class HanningFilter<T>; \
	template EMBER_API class HammingFilter<T>; \
	template EMBER_API class Lanczos3Filter<T>; \
	template EMBER_API class Lanczos2Filter<T>; \
	template EMBER_API class QuadraticFilter<T>; \
	template EMBER_API class DensityFilter<T>; \
	template EMBER_API class TemporalFilter<T>; \
	template EMBER_API class ExpTemporalFilter<T>; \
	template EMBER_API class GaussianTemporalFilter<T>; \
	template EMBER_API class BoxTemporalFilter<T>; \
	template EMBER_API class SpatialFilterCreator<T>; \
	template EMBER_API class TemporalFilterCreator<T>; \
	template EMBER_API class Interpolater<T>; \
	template EMBER_API class Ember<T>; \
	/*template EMBER_API class RenderCallback<T>;*/ \
	template EMBER_API class CarToRas<T>; \
	template EMBER_API class Curves<T>; \
	template EMBER_API class XmlToEmber<T>; \
	template EMBER_API class EmberToXml<T>;

EXPORT_SINGLE_TYPE_EMBER(float)

#define EXPORT_TWO_TYPE_EMBER(T, bucketT) \
	template EMBER_API class SheepTools<T, bucketT>;

EXPORT_TWO_TYPE_EMBER(float, float)

#ifdef DO_DOUBLE
	EXPORT_SINGLE_TYPE_EMBER(double)
	EXPORT_TWO_TYPE_EMBER(double, double)
#endif
}
