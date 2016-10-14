/******************************************************************************
 * $Id: WCS_GetCoverage.h 2011-07-19 16:24:00Z $
 *
 * Project:  The Open Geospatial Consortium (OGC) Web Coverage Service (WCS)
 * 			 for Earth Observation: Open Source Reference Implementation
 * Purpose:  WCS_GetCoverage class definition
 * Author:   Yuanzheng Shao, yshao3@gmu.edu
 *
 ******************************************************************************
 * Copyright (c) 2011, Liping Di <ldi@gmu.edu>, Yuanzheng Shao <yshao3@gmu.edu>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ****************************************************************************/

#ifndef WCS_GETCOVERAGE_H_
#define WCS_GETCOVERAGE_H_
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "WCS_T.h"

/* ******************************************************************** */
/*                          WCS_DescribeCoverage                        */
/* ******************************************************************** */

//! This class is used to handle GetCoverage request.

class WCS_GetCoverage: public WCS_T
{
protected:
	auto_ptr<AbstractDataset> mp_AbsDS;

	double md_RequestMinX;		//Request bounding box, xmin
	double md_RequestMinY;		//Request bounding box, ymin
	double md_RequestMaxX;		//Request bounding box, xmax
	double md_RequestMaxY;		//Request bounding box, ymin

	int mi_OutputWidth;
	int mi_OutputHeight;

	vector<int> mvi_OutputWH;
	vector<double> mvd_OutputResXY;

	string ms_CovID;			//Coverage identifier used to display, hide the internal path
	string ms_CovGDALID;		//Real coverage identifier could be recognized by GDAL
	string ms_RequestCRS_URN;	//subset=Lat,http://www.opengis.net/def/crs/EPSG/0/4326(20,40)
	string ms_ResponseCRS_URN;	//http://www.opengis.net/def/crs/EPSG/0/32612, or EPSG:32612?
	string ms_OutputFormat;		//image/geotiff
	string ms_OutputFormatCode;	//GTIFF
	string ms_OutputContentType;//Content-Type: image/png
	string ms_RequestBeginTime;	//2010-06-06T12:12:12Z
	string ms_RequestEndTime;	//2010-06-06T12:12:12Z
	string ms_Interpolation;	//nearest&bilinea&cubic&

	int mb_IsStore;				//Return XML which include the URL for output file
	int mb_SubsetSpatial;		//Does user specify spatial subset?
	int mb_MultiPart;			//Does user specify multiple part output?

	OGRSpatialReference mo_NativeCRS;
	OGRSpatialReference mo_RequestedCRS;
	OGRSpatialReference mo_ResponseCRS;

	vector<int> mvi_BandList;
	double md_OutGeoTransform[6];

	GDALResampleAlg me_Interplation;

protected:
	string CreateOutputFileSuffix();
	CPLErr CreateISO19115Metadata(DatasetObject dsObj);
	CPLErr CreateEOMetadata(const string& sOutFileName);
	CPLErr GetCoverageInitial();
	CPLErr SetCRSFromURN(OGRSpatialReference& crs_ID, const char* CRS_urn);
	CPLErr CreateBinaryFile(const string& sOutFileName);
    CPLErr CreateHDFEOS2File(const string& sSourceFile, string hdfeosFile);
	CPLErr CreateOutputFile(const string& sOutFileName);
	CPLErr SetOutputResolution();
	CPLErr HttpDirectoryRespond(const string& sOutFileName);
	CPLErr HttpStoreRespond(const string& sOutFileName);
	CPLErr HttpMultiPartsDirectoryRespond(const string& sOutFileName);
	CPLErr ExeCommand(string logFilePath, string cmd);

public:
	WCS_GetCoverage();
	WCS_GetCoverage(const string& conf);
	virtual ~WCS_GetCoverage();

	virtual CPLErr GetReqMessageFromXMLTree(CPLXMLNode *xmlRoot);
	virtual CPLErr GetReqMessageFromURLString(const string&);

	virtual void SetSoapMessage(const int& soapMsg)
	{
		mb_SoapRequest = soapMsg;
	}

	virtual int IsSoapRequested()
	{
		return mb_SoapRequest;
	}

	virtual void WCST_Respond(string& sOutFileName);
	virtual void WCST_Respond();

};

#endif /* WCS_GETCOVERAGE_H_ */
