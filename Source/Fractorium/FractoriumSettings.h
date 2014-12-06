#pragma once

#include "FractoriumPch.h"

/// <summary>
/// FractoriumSettings class.
/// </summary>

#define EARLYCLIP            "render/earlyclip"
#define YAXISUP				 "render/yaxisup"
#define TRANSPARENCY         "render/transparency"
#define OPENCL               "render/opencl"
#define DOUBLEPRECISION		 "render/dp64"
#define SHOWALLXFORMS	     "render/dragshowallxforms"
#define PLATFORMINDEX        "render/platformindex"
#define DEVICEINDEX          "render/deviceindex"
#define THREADCOUNT          "render/threadcount"
#define CPUDEFILTER			 "render/cpudefilter"
#define OPENCLDEFILTER       "render/opencldefilter"
#define CPUSUBBATCH		     "render/cpusubbatch"
#define OPENCLSUBBATCH	     "render/openclsubbatch"

#define FINALEARLYCLIP       "finalrender/earlyclip"
#define FINALYAXISUP         "finalrender/finalyaxisup"
#define FINALTRANSPARENCY    "finalrender/transparency"
#define FINALOPENCL          "finalrender/opencl"
#define FINALDOUBLEPRECISION "finalrender/dp64"
#define FINALSAVEXML		 "finalrender/savexml"
#define FINALDOALL		     "finalrender/doall"
#define FINALDOSEQUENCE	     "finalrender/dosequence"
#define FINALKEEPASPECT		 "finalrender/keepaspect"
#define FINALSCALE			 "finalrender/scale"
#define FINALEXT			 "finalrender/ext"
#define FINALPLATFORMINDEX   "finalrender/platformindex"
#define FINALDEVICEINDEX     "finalrender/deviceindex"
#define FINALTHREADCOUNT     "finalrender/threadcount"
#define FINALQUALITY	     "finalrender/quality"
#define FINALTEMPORALSAMPLES "finalrender/temporalsamples"
#define FINALSUPERSAMPLE     "finalrender/supersample"
#define FINALSTRIPS		     "finalrender/strips"

#define XMLWIDTH			 "xml/width"
#define XMLHEIGHT			 "xml/height"
#define XMLTEMPORALSAMPLES	 "xml/temporalsamples"
#define XMLQUALITY			 "xml/quality"
#define XMLSUPERSAMPLE		 "xml/supersample"

#define OPENFOLDER			 "path/open"
#define SAVEFOLDER			 "path/save"

#define OPENXMLEXT			 "file/openxmlext"
#define SAVEXMLEXT			 "file/savexmlext"
#define OPENIMAGEEXT		 "file/openimageext"
#define SAVEIMAGEEXT		 "file/saveimageext"

#define IDENTITYID			 "identity/id"
#define IDENTITYURL			 "identity/url"
#define IDENTITYNICK		 "identity/nick"

/// <summary>
/// Class for preserving various program options between
/// runs of Fractorium. Each of these generally corresponds
/// to items in the options dialog and the final render dialog.
/// </summary>
class FractoriumSettings : public QSettings
{
	Q_OBJECT
public:
	FractoriumSettings(QObject* parent);
	void EnsureDefaults();

	bool EarlyClip();
	void EarlyClip(bool b);
	
	bool YAxisUp();
	void YAxisUp(bool b);

	bool Transparency();
	void Transparency(bool b);
	
	bool OpenCL();
	void OpenCL(bool b);
		
	bool Double();
	void Double(bool b);

	bool ShowAllXforms();
	void ShowAllXforms(bool b);

	uint PlatformIndex();
	void PlatformIndex(uint b);

	uint DeviceIndex();
	void DeviceIndex(uint b);

	uint ThreadCount();
	void ThreadCount(uint b);

	bool CpuDEFilter();
	void CpuDEFilter(bool b);

	bool OpenCLDEFilter();
	void OpenCLDEFilter(bool b);

	uint CpuSubBatch();
	void CpuSubBatch(uint b);

	uint OpenCLSubBatch();
	void OpenCLSubBatch(uint b);
	
	bool FinalEarlyClip();
	void FinalEarlyClip(bool b);
	
	bool FinalYAxisUp();
	void FinalYAxisUp(bool b);

	bool FinalTransparency();
	void FinalTransparency(bool b);

	bool FinalOpenCL();
	void FinalOpenCL(bool b);

	bool FinalDouble();
	void FinalDouble(bool b);
	
	bool FinalSaveXml();
	void FinalSaveXml(bool b);

	bool FinalDoAll();
	void FinalDoAll(bool b);

	bool FinalDoSequence();
	void FinalDoSequence(bool b);

	bool FinalKeepAspect();
	void FinalKeepAspect(bool b);
	
	uint FinalScale();
	void FinalScale(uint i);
	
	QString FinalExt();
	void FinalExt(const QString& s);

	uint FinalPlatformIndex();
	void FinalPlatformIndex(uint b);

	uint FinalDeviceIndex();
	void FinalDeviceIndex(uint b);

	uint FinalThreadCount();
	void FinalThreadCount(uint b);

	uint FinalQuality();
	void FinalQuality(uint i);

	uint FinalTemporalSamples();
	void FinalTemporalSamples(uint i);

	uint FinalSupersample();
	void FinalSupersample(uint i);

	uint FinalStrips();
	void FinalStrips(uint i);

	uint XmlTemporalSamples();
	void XmlTemporalSamples(uint i);

	uint XmlQuality();
	void XmlQuality(uint i);

	uint XmlSupersample();
	void XmlSupersample(uint i);

	QString OpenFolder();
	void OpenFolder(const QString& s);

	QString SaveFolder();
	void SaveFolder(const QString& s);

	QString OpenXmlExt();
	void OpenXmlExt(const QString& s);

	QString SaveXmlExt();
	void SaveXmlExt(const QString& s);

	QString OpenImageExt();
	void OpenImageExt(const QString& s);

	QString SaveImageExt();
	void SaveImageExt(const QString& s);

	QString Id();
	void Id(const QString& s);

	QString Url();
	void Url(const QString& s);

	QString Nick();
	void Nick(const QString& s);
};
