#include "FractoriumPch.h"
#include "FractoriumSettings.h"

/// <summary>
/// Constructor that passes the parent to the base and sets up reasonable defaults
/// if the settings file was not present or corrupted.
/// </summary>
/// <param name="parent">The parent widget</param>
FractoriumSettings::FractoriumSettings(QObject* parent)
	: QSettings(QSettings::IniFormat, QSettings::UserScope, "Fractorium", "Fractorium", parent)
{
	EnsureDefaults();
}

/// <summary>
/// Make sure options have reasonable values in them first.
/// </summary>
void FractoriumSettings::EnsureDefaults()
{
	if (FinalWidth() == 0)
		FinalWidth(1920);

	if (FinalHeight() == 0)
		FinalHeight(1080);

	if (FinalQuality() == 0)
		FinalQuality(1000);

	if (FinalTemporalSamples() == 0)
		FinalTemporalSamples(1000);

	if (FinalSupersample() == 0)
		FinalSupersample(2);

	if (XmlWidth() == 0)
		XmlWidth(1920);

	if (XmlHeight() == 0)
		XmlHeight(1080);

	if (XmlTemporalSamples() == 0)
		XmlTemporalSamples(1000);

	if (XmlQuality() == 0)
		XmlQuality(1000);

	if (XmlSupersample() == 0)
		XmlSupersample(2);

	if (ThreadCount() == 0 || ThreadCount() > Timing::ProcessorCount())
		ThreadCount(max(1, Timing::ProcessorCount() - 1));//Default to one less to keep the UI responsive for first time users.

	if (FinalThreadCount() == 0 || FinalThreadCount() > Timing::ProcessorCount())
		FinalThreadCount(Timing::ProcessorCount());

	if (CpuSubBatch() < 1)
		CpuSubBatch(10);

	if (OpenCLSubBatch() < 1)
		OpenCLSubBatch(1);

	//There normally wouldn't be any more than 10 OpenCL platforms and devices
	//on the system, so if a value greater than that is read, then the settings file
	//was corrupted.
	if (PlatformIndex() > 10)
		PlatformIndex(0);

	if (DeviceIndex() > 10)
		DeviceIndex(0);

	if (FinalScale() > SCALE_HEIGHT)
		FinalScale(0);

	if (FinalPlatformIndex() > 10)
		FinalPlatformIndex(0);

	if (FinalDeviceIndex() > 10)
		FinalDeviceIndex(0);

	if (OpenXmlExt() == "")
		OpenXmlExt("Flame (*.flame)");

	if (SaveXmlExt() == "")
		SaveXmlExt("Flame (*.flame)");

	if (OpenImageExt() == "")
		OpenImageExt("Png (*.png)");

	if (SaveImageExt() == "")
		SaveImageExt("Png (*.png)");

	if (FinalDoAllExt() != "jpg" && FinalDoAllExt() != "png")
		FinalDoAllExt("png");
}

/// <summary>
/// Interactive renderer settings.
/// </summary>

bool FractoriumSettings::EarlyClip()                          { return value(EARLYCLIP).toBool();            }
void FractoriumSettings::EarlyClip(bool b)                    { setValue(EARLYCLIP, b);                      }
															  
bool FractoriumSettings::Transparency()                       { return value(TRANSPARENCY).toBool();         }
void FractoriumSettings::Transparency(bool b)                 { setValue(TRANSPARENCY, b);                   }

bool FractoriumSettings::Double()							  { return value(DOUBLEPRECISION).toBool();		 }
void FractoriumSettings::Double(bool b)						  { setValue(DOUBLEPRECISION, b);				 }

bool FractoriumSettings::OpenCL()                             { return value(OPENCL).toBool();               }
void FractoriumSettings::OpenCL(bool b)                       { setValue(OPENCL, b);                         }

unsigned int FractoriumSettings::PlatformIndex()              { return value(PLATFORMINDEX).toUInt();        }
void FractoriumSettings::PlatformIndex(unsigned int i)        { setValue(PLATFORMINDEX, i);                  }
															  
unsigned int FractoriumSettings::DeviceIndex()                { return value(DEVICEINDEX).toUInt();          }
void FractoriumSettings::DeviceIndex(unsigned int i)          { setValue(DEVICEINDEX, i);                    }

unsigned int FractoriumSettings::ThreadCount()                { return value(THREADCOUNT).toUInt();          }
void FractoriumSettings::ThreadCount(unsigned int i)          { setValue(THREADCOUNT, i);                    }
															  
bool FractoriumSettings::CpuDEFilter()                        { return value(CPUDEFILTER).toBool();          }
void FractoriumSettings::CpuDEFilter(bool b)                  { setValue(CPUDEFILTER, b);                    }
															  
bool FractoriumSettings::OpenCLDEFilter()                     { return value(OPENCLDEFILTER).toBool();       }
void FractoriumSettings::OpenCLDEFilter(bool b)               { setValue(OPENCLDEFILTER, b);                 }
															  
unsigned int FractoriumSettings::CpuSubBatch()				  { return value(CPUSUBBATCH).toUInt();		     }
void FractoriumSettings::CpuSubBatch(unsigned int b)		  { setValue(CPUSUBBATCH, b);					 }
															  
unsigned int FractoriumSettings::OpenCLSubBatch()			  { return value(OPENCLSUBBATCH).toUInt();	     }
void FractoriumSettings::OpenCLSubBatch(unsigned int b)		  { setValue(OPENCLSUBBATCH, b);				 }

/// <summary>
/// Final render settings.
/// </summary>

bool FractoriumSettings::FinalEarlyClip()                     { return value(FINALEARLYCLIP).toBool();       }
void FractoriumSettings::FinalEarlyClip(bool b)               { setValue(FINALEARLYCLIP, b);                 }
															  
bool FractoriumSettings::FinalTransparency()                  { return value(FINALTRANSPARENCY).toBool();    }
void FractoriumSettings::FinalTransparency(bool b)            { setValue(FINALTRANSPARENCY, b);              }
															  
bool FractoriumSettings::FinalOpenCL()                        { return value(FINALOPENCL).toBool();          }
void FractoriumSettings::FinalOpenCL(bool b)                  { setValue(FINALOPENCL, b);                    }
									
bool FractoriumSettings::FinalDouble()						  { return value(FINALDOUBLEPRECISION).toBool(); }
void FractoriumSettings::FinalDouble(bool b)				  { setValue(FINALDOUBLEPRECISION, b);			 }

bool FractoriumSettings::FinalSaveXml()						  { return value(FINALSAVEXML).toBool();		 }
void FractoriumSettings::FinalSaveXml(bool b)				  { setValue(FINALSAVEXML, b);					 }

bool FractoriumSettings::FinalDoAll()						  { return value(FINALDOALL).toBool();		     }
void FractoriumSettings::FinalDoAll(bool b)					  { setValue(FINALDOALL, b);					 }
															  
bool FractoriumSettings::FinalDoSequence()					  { return value(FINALDOSEQUENCE).toBool();	     }
void FractoriumSettings::FinalDoSequence(bool b)			  { setValue(FINALDOSEQUENCE, b);				 }

bool FractoriumSettings::FinalKeepAspect()					  { return value(FINALKEEPASPECT).toBool();		 }
void FractoriumSettings::FinalKeepAspect(bool b)			  { setValue(FINALKEEPASPECT, b);				 }

unsigned int FractoriumSettings::FinalScale()				  { return value(FINALSCALE).toUInt();			 }
void FractoriumSettings::FinalScale(unsigned int i)			  { setValue(FINALSCALE, i);					 }

QString FractoriumSettings::FinalDoAllExt()					  { return value(FINALDOALLEXT).toString();	     }
void FractoriumSettings::FinalDoAllExt(QString s)			  { setValue(FINALDOALLEXT, s);				     }
															  
unsigned int FractoriumSettings::FinalPlatformIndex()         { return value(FINALPLATFORMINDEX).toUInt();   }
void FractoriumSettings::FinalPlatformIndex(unsigned int i)   { setValue(FINALPLATFORMINDEX, i);             }
															  
unsigned int FractoriumSettings::FinalDeviceIndex()           { return value(FINALDEVICEINDEX).toUInt();     }
void FractoriumSettings::FinalDeviceIndex(unsigned int i)     { setValue(FINALDEVICEINDEX, i);               }
															  
unsigned int FractoriumSettings::FinalThreadCount()           { return value(FINALTHREADCOUNT).toUInt();     }
void FractoriumSettings::FinalThreadCount(unsigned int i)     { setValue(FINALTHREADCOUNT, i);               }
															  
unsigned int FractoriumSettings::FinalWidth()                 { return value(FINALWIDTH).toUInt();           }
void FractoriumSettings::FinalWidth(unsigned int i)           { setValue(FINALWIDTH, i);                     }
															  
unsigned int FractoriumSettings::FinalHeight()                { return value(FINALHEIGHT).toUInt();          }
void FractoriumSettings::FinalHeight(unsigned int i)          { setValue(FINALHEIGHT, i);                    }
															  
unsigned int FractoriumSettings::FinalQuality()               { return value(FINALQUALITY).toUInt();         }
void FractoriumSettings::FinalQuality(unsigned int i)         { setValue(FINALQUALITY, i);                   }

unsigned int FractoriumSettings::FinalTemporalSamples()       { return value(FINALTEMPORALSAMPLES).toUInt(); }
void FractoriumSettings::FinalTemporalSamples(unsigned int i) { setValue(FINALTEMPORALSAMPLES, i);           }

unsigned int FractoriumSettings::FinalSupersample()           { return value(FINALSUPERSAMPLE).toUInt();     }
void FractoriumSettings::FinalSupersample(unsigned int i)     { setValue(FINALSUPERSAMPLE, i);               }

/// <summary>
/// Xml file saving settings.
/// </summary>

unsigned int FractoriumSettings::XmlWidth()                   { return value(XMLWIDTH).toUInt();             }
void FractoriumSettings::XmlWidth(unsigned int i)             { setValue(XMLWIDTH, i);                       }
															  
unsigned int FractoriumSettings::XmlHeight()                  { return value(XMLHEIGHT).toUInt();            }
void FractoriumSettings::XmlHeight(unsigned int i)            { setValue(XMLHEIGHT, i);                      }
															  
unsigned int FractoriumSettings::XmlTemporalSamples()         { return value(XMLTEMPORALSAMPLES).toUInt();   }
void FractoriumSettings::XmlTemporalSamples(unsigned int i)   { setValue(XMLTEMPORALSAMPLES, i);             }
															  
unsigned int FractoriumSettings::XmlQuality()                 { return value(XMLQUALITY).toUInt();           }
void FractoriumSettings::XmlQuality(unsigned int i)           { setValue(XMLQUALITY, i);                     }
															  
unsigned int FractoriumSettings::XmlSupersample()             { return value(XMLSUPERSAMPLE).toUInt();       }
void FractoriumSettings::XmlSupersample(unsigned int i)       { setValue(XMLSUPERSAMPLE, i);                 }
													  
QString FractoriumSettings::Id()                              { return value(IDENTITYID).toString();         }
void FractoriumSettings::Id(QString s)                        { setValue(IDENTITYID, s);                     }
															  
QString FractoriumSettings::Url()                             { return value(IDENTITYURL).toString();        }
void FractoriumSettings::Url(QString s)                       { setValue(IDENTITYURL, s);                    }
															  
QString FractoriumSettings::Nick()                            { return value(IDENTITYNICK).toString();       }
void FractoriumSettings::Nick(QString s)                      { setValue(IDENTITYNICK, s);                   }

/// <summary>
/// General operations settings.
/// </summary>

QString FractoriumSettings::OpenFolder()					  { return value(OPENFOLDER).toString();		 }
void FractoriumSettings::OpenFolder(QString s)				  { setValue(OPENFOLDER, s);					 }
															  											   
QString FractoriumSettings::SaveFolder()					  { return value(SAVEFOLDER).toString();		 }
void FractoriumSettings::SaveFolder(QString s)				  { setValue(SAVEFOLDER, s);					 }
															  
QString FractoriumSettings::OpenXmlExt()					  { return value(OPENXMLEXT).toString();		 }
void FractoriumSettings::OpenXmlExt(QString s)				  { setValue(OPENXMLEXT, s);					 }
															  
QString FractoriumSettings::SaveXmlExt()					  { return value(SAVEXMLEXT).toString();		 }
void FractoriumSettings::SaveXmlExt(QString s)				  { setValue(SAVEXMLEXT, s);					 }
															  
QString FractoriumSettings::OpenImageExt()					  { return value(OPENIMAGEEXT).toString();	     }
void FractoriumSettings::OpenImageExt(QString s)			  { setValue(OPENIMAGEEXT, s);				     }
															  											     
QString FractoriumSettings::SaveImageExt()					  { return value(SAVEIMAGEEXT).toString();	     }
void FractoriumSettings::SaveImageExt(QString s)			  { setValue(SAVEIMAGEEXT, s);				     }
