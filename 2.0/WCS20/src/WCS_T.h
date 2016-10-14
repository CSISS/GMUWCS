/******************************************************************************
 * $Id: WCS_T.h 2011-07-19 16:24:00Z $
 *
 * Project:  The Open Geospatial Consortium (OGC) Web Coverage Service (WCS)
 * 			 for Earth Observation: Open Source Reference Implementation
 * Purpose:  WCS_T class definition, define a series of virtual functions
 *           which will be implemented in WCS operation-related class
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

#ifndef WCS_T_H_
#define WCS_T_H_

#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>

#include <cpl_minixml.h>

#include "wcsUtil.h"
#include "wcstdsinc.h"
#include "WCS_Configure.h"
#include "BoundingBox.h"
#include "netcdfcpp.h"

typedef std::map<string, vector<string > > STRING2VECTOR;
using namespace std;

/* ******************************************************************** */
/*                              DatasetObject                           */
/* ******************************************************************** */

//! DatasetObject class is used to record Dataset coverage attributes.

class DatasetObject
{
public:
	string m_covName;
	string m_covPath;
	string m_covGDALID;
	string m_beginTime;
	string m_endTime;
	string m_iso19115_metadata;
	double m_minx;
	double m_maxx;
	double m_miny;
	double m_maxy;
};

/* ******************************************************************** */
/*                            StitchedMosaicObject                      */
/* ******************************************************************** */

//! StitchedMosaicObject class is used to record stitched mosaic coverage attributes.

class StitchedMosaicObject
{
public:
	vector<DatasetObject> mv_dataset;
	string m_covName;
	string m_beginTime;
	string m_endTime;
	double m_minx;
	double m_maxx;
	double m_miny;
	double m_maxy;
};

/* ******************************************************************** */
/*                            DatasetSeriesObject                       */
/* ******************************************************************** */

//! DatasetSeriesObject class is used to record dataset series coverage attributes.

class DatasetSeriesObject
{
public:
	vector<DatasetObject> mv_dataset;
	vector<StitchedMosaicObject> mv_stitchedMosaic;
	string m_covType;
	string m_covCollectionName;
	string m_covName;
	string m_beginTime;
	string m_endTime;
	double m_minx;
	double m_maxx;
	double m_miny;
	double m_maxy;
};

/* ******************************************************************** */
/*                                      WCS_T                           */
/* ******************************************************************** */

//! WCS_T is a upper class for handling WCS request.

class WCS_T
{
protected:
	WCS_Configure *mp_Conf;
	vector<DatasetObject> mv_datasetCoverage;
	vector<StitchedMosaicObject> mv_stitchedMosaicCoverage;
	vector<DatasetSeriesObject> mv_datasetSeriesCoverage;

	string 	ms_datasetConfPath;
	string 	ms_stitchedMosaicConfPath;
	string 	ms_datasetSeriesConfPath;
	string	ms_iso19115Contents;
	string	ms_eoMetadataContents;
	string  ms_requestFullURL;

	string 	ms_dataDirectoryPath;

public:
	int 	mb_SoapRequest;
	string 	ms_Service_op_URL;
	vector<string> mvs_SupportFormats;
	vector<string> mv_CovIDs;

public:
	WCS_T();
	WCS_T(const string& conf);
	virtual ~WCS_T();

	CPLErr InitializeConfigurationFiles();
	DatasetSeriesObject InitializeDatasetSeriesByID(string& sCovID);
	DatasetObject InitializeDatasetByID(string& sCovID);

	void SetSoapMessage(const int& soapMsg)
	{
		mb_SoapRequest = soapMsg;
	}

	int IsSoapRequested()
	{
		return mb_SoapRequest;
	}

	void SendHttpHead();

	virtual CPLErr GetReqMessageFromXMLTree(CPLXMLNode *xmlRoot)
	{
		return CE_Failure;
	}

	virtual CPLErr GetReqMessageFromURLString(const string&)
	{
		return CE_Failure;
	}

	virtual void WCST_Respond(string& sOutFileName)
	{
		return;
	}

	virtual void WCST_Respond()
	{
		return;
	}

};

CPL_C_START

WCS_T* WCSTOpenFromXMLString(const string&, const string&);
WCS_T* WCSTOpenFromXMLFile(const string&, const string&);
WCS_T* WCSTOpenFromURLString(string, const string&);
WCS_T* WCSTCreateFromXMLTree(CPLXMLNode *xmlRoot, const string&);

vector<string> WCSTGetXMLValueList(CPLXMLNode *psRoot, const char* pszPath);
vector<string> WCSTRegisterCoverageID(vector<string>& fileList);
vector<CPLXMLNode*> WCSTGetXMLNodeList(CPLXMLNode *psRoot, const char* pszPath);

void WCSTClose(WCS_T*);
void WCSTDestroyDataset(AbstractDataset* absDS);

AbstractDataset* WCSTCreateDataset(const string& covID, vector<int>& oBandList, const int isSample=0);

CPL_C_END

#endif /* WCS_T_H_ */
