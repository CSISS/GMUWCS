/******************************************************************************
 * $Id: WCS_GetCoverage.cpp 2011-07-19 16:24:00Z $
 *
 * Project:  The Open Geospatial Consortium (OGC) Web Coverage Service (WCS)
 * 			 for Earth Observation: Open Source Reference Implementation
 * Purpose:  WCS_GetCoverage class implementation
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

#include "WCS_GetCoverage.h"
#include "WCS_DescribeCoverage.h"

#include <math.h>
#include <iostream>
#include <fstream>
#include "hdf.h"
#include "mfhdf.h"

/************************************************************************/
/* ==================================================================== */
/*                            WCS_GetCoverage                           */
/* ==================================================================== */
/************************************************************************/

/**
 * \class WCS_GetCoverage "WCS_GetCoverage.h"
 *
 * This class is used to handle GetCoverage request. Several functions
 * for parsing request parameters, generating the response file, are provided.
 */


WCS_GetCoverage::WCS_GetCoverage()
{

}

/************************************************************************/
/*                            WCS_GetCoverage()                         */
/************************************************************************/

/**
 * \brief Constructor of a WCS_GetCoverage object.
 *
 * This is the accepted method of creating an WCS_GetCoverage object.
 *
 * @param conf String of the full path of the configuration file.
 */

WCS_GetCoverage::WCS_GetCoverage(const string& conf) :
		WCS_T(conf)
{
	mb_SoapRequest = 0;
	ms_CovID = "";
	ms_RequestCRS_URN = "";
	ms_ResponseCRS_URN = "";
	mvi_BandList.clear();
	ms_OutputFormat = "";
	mb_SubsetSpatial = false;
	mb_IsStore = false;
	mb_MultiPart = false;

	ms_Interpolation = "near";//GDALWARP rules
	me_Interplation = GRA_NearestNeighbour;
}

WCS_GetCoverage::~WCS_GetCoverage()
{
	WCSTDestroyDataset(mp_AbsDS.release());
}

/************************************************************************/
/*                      GetReqMessageFromXMLTree()                      */
/************************************************************************/

/**
 * \brief Fetch the WCS_GetCoverage request parameters from XML tree object.
 *
 * This method is used to fetch the WCS_GetCoverage parameters from an
 * XML object (HTTP POST method).
 *
 *Sample request XML document
 * @code
<?xml version="1.0" encoding="UTF-8"?>
<wcs:GetCoverage
  xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xmlns:wcs="http://www.opengis.net/wcs/2.0"
  xmlns:gml="http://www.opengis.net/gml/3.2"
  xsi:schemaLocation=
    "http://schemas.opengis.net/wcs/2.0 ../wcsAll.xsd"
  service="WCS" version="2.0.0">
  <wcs:CoverageId>MOD13C1.A2007145.005.2007184062524.hdf:NDVI</wcs:CoverageId>
  <wcs:trimDimension>
    <wcs:dimension>Long</wcs:dimension>
    <wcs:trimLow>20</wcs:trimLow>
    <wcs:trimHigh>29</wcs:trimHigh>
  </wcs:trimDimension>
  <wcs:trimDimension>
    <wcs:dimension>Lat</wcs:dimension>
    <wcs:trimLow>20</wcs:trimLow>
    <wcs:trimHigh>30</wcs:trimHigh>
  </wcs:trimDimension>
  <wcs:Output format="image/geotiff" store="false">
  	<wcs:outputCRS>EPSG:4326</wcs:outputCRS>
  </wcs:Output>
</wcs:GetCoverage>
 * @endcode
 *
 * @param xmlRoot XML Node object created by GDAL library.
 *
 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr WCS_GetCoverage::GetReqMessageFromXMLTree(CPLXMLNode *xmlRoot)
{
	if (!xmlRoot)
	{
		SetWCS_ErrorLocator("WCS_GetCoverage::GetReqMessageFromXMLTree()");
		WCS_Error(CE_Failure, OGC_WCS_NoApplicableCode,
				"Invalid XML Node.");
		return CE_Failure;
	}

	vector<string> strVec;
	string tmpStr;

	/*Parse IDENTIFIER*/
	ms_CovID = CPLGetXMLValue(xmlRoot, "CoverageId", "");

	if (ms_CovID == "")
	{
		SetWCS_ErrorLocator("IDENTIFIER");
		WCS_Error(CE_Failure, OGC_WCS_NoSuchCoverage,
				"The value of coverage identifier is null.");
		return CE_Failure;
	}

	/*Parse DomainSubset*/
	vector<CPLXMLNode *> trimNodes = WCSTGetXMLNodeList(xmlRoot, "trimDimension");
	for(unsigned int i = 0; i < trimNodes.size(); i++)
	{
		string dimensionName = CPLGetXMLValue(trimNodes[i], "dimension", "");
		string trimLowName = CPLGetXMLValue(trimNodes[i], "trimLow", "");
		string trimHighName = CPLGetXMLValue(trimNodes[i], "trimHigh", "");
		if(EQUAL(dimensionName.c_str(), "Long") || EQUAL(dimensionName.c_str(), "Lon") || EQUAL(dimensionName.c_str(), "x"))
		{
			mb_SubsetSpatial = true;
			ms_RequestCRS_URN = "EPSG:4326";//Hard cord, specification is not ready about specifying CRS in request XML
			convertFromString(md_RequestMinX, trimLowName);
			convertFromString(md_RequestMaxX, trimHighName);
		}
		else if(EQUAL(dimensionName.c_str(), "Lat") || EQUAL(dimensionName.c_str(), "y"))
		{
			mb_SubsetSpatial = true;
			ms_RequestCRS_URN = "EPSG:4326";//Hard cord, specification is not ready about specifying CRS in request XML
			convertFromString(md_RequestMinY, trimLowName);
			convertFromString(md_RequestMaxY, trimHighName);
		}
		else if(EQUAL(dimensionName.c_str(), "phenomenonTime") || EQUAL(dimensionName.c_str(), "Time"))
		{
			ms_RequestBeginTime = trimLowName;
			ms_RequestEndTime = trimHighName;
		}
	}

	/*Parse Output element*/
	CPLXMLNode *outNode = CPLGetXMLNode(xmlRoot, "Output");
	if (outNode)
	{
		ms_OutputFormat = CPLGetXMLValue(outNode, "Format", "");
		ms_ResponseCRS_URN = CPLGetXMLValue(outNode, "OUTPUTCRS", "");

		string tmpStr = CPLGetXMLValue(outNode, "Store", "");
		if (tmpStr != "")
		{
			tmpStr = StrTrim(tmpStr);
			mb_IsStore = EQUAL(tmpStr.c_str(),"TRUE") ? true : false;
		}
	}

	return CE_None;
}

/************************************************************************/
/*                    GetReqMessageFromURLString()                      */
/************************************************************************/

/**
 * \brief Fetch the request parameters from URL string.
 *
 * This method is used to fetch the GetCoverage parameters from an URL
 * string (HTTP GET method).
 *
 * @param urlStr String of the DescibeCoverage or DescribeEOCoverageSet request.
 *
 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr WCS_GetCoverage::GetReqMessageFromURLString(const string& urlStr)
{
	string tmpStr, tmpStr1;
	vector<string> strVec;
	KVPsReader kvps(urlStr, '&');

	ms_requestFullURL = mp_Conf->Get_SERVICE_ACCESS_URL() + urlStr;
	ms_requestFullURL = StrReplace(ms_requestFullURL, "&", "%26");
	ms_requestFullURL = StrReplace(ms_requestFullURL, "\"", "%22");

	ms_CovID = kvps.getValue("COVERAGEID", "");
	if (ms_CovID == "")
	{
		SetWCS_ErrorLocator("COVERAGE");
		WCS_Error(CE_Failure, OGC_WCS_MissingParameterValue,
				"No Coverage Value.");

		return CE_Failure;
	}

	//Yuanzheng Shao, 2012-07-23
	ms_CovGDALID = ms_CovID;

	ms_OutputFormat = kvps.getValue("FORMAT", "");
	if (ms_OutputFormat == "")
	{
		SetWCS_ErrorLocator("FORMAT");
		WCS_Error(CE_Failure, OGC_WCS_MissingParameterValue,
				"No FORMAT Value.");

		return CE_Failure;
	}

	tmpStr = kvps.getValue("STORE", "");
	if (tmpStr != "")
	{
		tmpStr = StrTrim(tmpStr);
		if (EQUAL(tmpStr.c_str(),"TRUE"))
			mb_IsStore = true;
	}

	strVec = kvps.getValues("SIZE");
	if(strVec.size() > 0)
	{
		int width = 100, height = 100;
		for(unsigned int i = 0; i < strVec.size(); i++)
		{
			string tmpv = strVec[i];
			if(Find_Compare_SubStr(tmpv, "Long") || Find_Compare_SubStr(tmpv, "Lon")  || Find_Compare_SubStr(tmpv, "x"))
				convertFromString(width, GetSingleValue(tmpv));
			else if(Find_Compare_SubStr(tmpv, "Lat") || Find_Compare_SubStr(tmpv, "y"))
				convertFromString(height, GetSingleValue(tmpv));
		}

		mvi_OutputWH.push_back(width);
		mvi_OutputWH.push_back(height);

		if (mvi_OutputWH[0] <= 0 || mvi_OutputWH[1] <= 0)
		{
			SetWCS_ErrorLocator("WIDTH/HEIGHT");
			WCS_Error(CE_Failure, OGC_WCS_InvalidParameterValue, "Invalid Parameter Value of WIDTH/HEIGHT.");
			return CE_Failure;
		}
	}

	strVec.clear();
	strVec = kvps.getValues("resolution");
	if(strVec.size() > 0)
	{
		double resx = 1, resy = 1;
		for(unsigned int i = 0; i < strVec.size(); i++)
		{
			string tmpv = strVec[i];
			if(Find_Compare_SubStr(tmpv, "Long") || Find_Compare_SubStr(tmpv, "Lon") || Find_Compare_SubStr(tmpv, "x"))
				convertFromString(resx, GetSingleValue(tmpv));
			else if(Find_Compare_SubStr(tmpv, "Lat") || Find_Compare_SubStr(tmpv, "y"))
				convertFromString(resy, GetSingleValue(tmpv));
		}

		mvd_OutputResXY.push_back(resx);
		mvd_OutputResXY.push_back(resy);

		if (mvd_OutputResXY[0] <= 0 || mvd_OutputResXY[1] <= 0)
		{
			SetWCS_ErrorLocator("ResolutionX");
			WCS_Error(CE_Failure, OGC_WCS_InvalidParameterValue, "Invalid Parameter Value of Resolution.");
			return CE_Failure;
		}
	}

	tmpStr = kvps.getValue("mediatype", "");
	if(tmpStr != "" && EQUAL(tmpStr.c_str(), "multipart/mixed"))
		mb_MultiPart = true;

	tmpStr = kvps.getValue("interpolation", "");
	if(tmpStr != "")
	{
		if(EQUAL(tmpStr.c_str(), "nearest"))
		{
			me_Interplation = GRA_NearestNeighbour;
			ms_Interpolation = "near";
		}
		else if(EQUAL(tmpStr.c_str(), "bilinear"))
		{
			me_Interplation = GRA_Bilinear;
			ms_Interpolation = "bilinear";
		}
		else if(EQUAL(tmpStr.c_str(), "cubic"))
		{
			me_Interplation = GRA_Cubic;
			ms_Interpolation = "cubic";
		}
		else if(EQUAL(tmpStr.c_str(), "cubicspline"))
		{
			me_Interplation = GRA_CubicSpline;
			ms_Interpolation = "cubicspline";
		}
		else if(EQUAL(tmpStr.c_str(), "lanczos"))
		{
			me_Interplation = GRA_Lanczos;
			ms_Interpolation = "lanczos";
		}
		else
		{
			SetWCS_ErrorLocator("interpolation");
			WCS_Error(CE_Failure, OGC_WCS_InvalidParameterValue, "Invalid Parameter Value of interpolation, please select from \"nearest\"(default), \"bilinear\", \"cubic\", \"cubicspline\" and \"lanczos\".");
			return CE_Failure;
		}
	}

	ms_ResponseCRS_URN = kvps.getValue("OUTPUTCRS", "");
	if(ms_ResponseCRS_URN != "")
	{
		if (OGRERR_NONE != mo_ResponseCRS.SetFromUserInput(ms_ResponseCRS_URN.c_str()))
		{
			SetWCS_ErrorLocator("OUTPUTCRS");
			WCS_Error(CE_Failure, OGC_WCS_InvalidParameterValue, "Invalid Parameter Value of OUTPUTCRS.");
			return CE_Failure;
		}
	}

	tmpStr = kvps.getValue("rangesubset", "");
	if(tmpStr != "")
	{
		vector<string> strSet;
		unsigned int n = CsvburstCpp(tmpStr, strSet, ',');
		for(unsigned int i = 0; i < n; i++)
		{
			int tmpV;
			convertFromString(tmpV, strSet[i]);
			mvi_BandList.push_back(tmpV);
		}
	}

	vector<string> valueList = kvps.getValues("subset");
	if(valueList.size() > 0)
	{
		for(unsigned int i = 0; i < valueList.size(); i++)
		{
			string tmpv = valueList[i];
			if(Find_Compare_SubStr(tmpv, "Lat"))
			{
				mb_SubsetSpatial = true;
				vector<double> latV;
				ms_RequestCRS_URN = GetSubSetLatLon(tmpv, latV);
				md_RequestMinY = latV[0];
				md_RequestMaxY = latV[1];
			}
			else if(Find_Compare_SubStr(tmpv, "Long") || Find_Compare_SubStr(tmpv, "Lon"))
			{
				mb_SubsetSpatial = true;
				vector<double> latV;
				ms_RequestCRS_URN = GetSubSetLatLon(tmpv, latV);
				md_RequestMinX = latV[0];
				md_RequestMaxX = latV[1];
			}
			else if(Find_Compare_SubStr(tmpv, "phenomenonTime"))
			{
				vector<string> timeV;
				GetSubSetTime(tmpv, timeV);
				if(timeV.size() == 1)
				{
					ms_RequestBeginTime = timeV[0];
					ms_RequestEndTime = "";
				}
				else if(timeV.size() == 2)
				{
					ms_RequestBeginTime = timeV[0];
					ms_RequestEndTime = timeV[1];
				}
			}
			else
			{
				SetWCS_ErrorLocator("GetCOVERAGE");
				WCS_Error(CE_Failure, OGC_WCS_InvalidParameterValue, "Unknown subset type.");

				return CE_Failure;
			}
		}
	}

	if(ms_RequestCRS_URN != "")
	{
		if (OGRERR_NONE != mo_RequestedCRS.SetFromUserInput(ms_RequestCRS_URN.c_str()))
		{
			SetWCS_ErrorLocator("subsetRequestCRS");
			WCS_Error(CE_Failure, OGC_WCS_InvalidParameterValue, "Invalid Parameter Value of Request CRS.");
			return CE_Failure;
		}
	}

	return CE_None;
}

/************************************************************************/
/*                       CreateISO19115Metadata()                       */
/************************************************************************/

/**
 * \brief Create an XML document following ISO 19115 and ISO 19115-2 standards.
 *
 * This method is used to create an XML document following ISO 19115 and ISO
 * 19115-2 standards, which is based on specified dataset object. There is a
 * template file which is available through configuration file. After reading
 * the template to memory, some keywords will be replace based on specified
 * dataset object.
 *
 * @param dsObj Dataset object.
 *
 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr WCS_GetCoverage::CreateISO19115Metadata(DatasetObject dsObj)
{
	ms_iso19115Contents = StrReplace(ms_iso19115Contents, "@FILEID@", ms_CovID);
	ms_iso19115Contents = StrReplace(ms_iso19115Contents, "@DATESTAMP@", GetTimeString(2));
	ms_iso19115Contents = StrReplace(ms_iso19115Contents, "@RESOURCEFORMAT@", ms_OutputFormat);
	ms_iso19115Contents = StrReplace(ms_iso19115Contents, "@GEOXMIN@", convertToString(md_RequestMinX));
	ms_iso19115Contents = StrReplace(ms_iso19115Contents, "@GEOXMAX@", convertToString(md_RequestMaxX));
	ms_iso19115Contents = StrReplace(ms_iso19115Contents, "@GEOYMIN@", convertToString(md_RequestMinY));
	ms_iso19115Contents = StrReplace(ms_iso19115Contents, "@GEOYMAX@", convertToString(md_RequestMaxY));
	ms_iso19115Contents = StrReplace(ms_iso19115Contents, "@STARTDATETIME@", dsObj.m_beginTime);
	ms_iso19115Contents = StrReplace(ms_iso19115Contents, "@ENDDATETIME@", dsObj.m_endTime);
	ms_iso19115Contents = StrReplace(ms_iso19115Contents, "@RUNTIMEPARA@", ms_requestFullURL);

	ms_iso19115Contents = StrReplace(ms_iso19115Contents, "\n", "");
	//ms_iso19115Contents = StrReplace(ms_iso19115Contents, " ", "");

	return CE_None;
}

/************************************************************************/
/*                          CreateEOMetadata()                          */
/************************************************************************/

/**
 * \brief Create an EO metadata for the output.
 *
 * This method is used to create ann EO metadata for the output.
 *
 * @param sOutFileName The path of output file.
 *
 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr WCS_GetCoverage::CreateEOMetadata(const string& sOutFileName)
{
	GDALDataset* outDS = (GDALDataset*) GDALOpen(sOutFileName.c_str(), GA_ReadOnly);
	AbstractDataset* absDS = WCSTCreateDataset(ms_CovGDALID, mvi_BandList, 1);
	string covSubType = absDS->GetCoverageSubType();

	string modOutFileName;
	if(Find_Compare_SubStr(sOutFileName, ".tmp.warp.tif"))
		modOutFileName = sOutFileName.substr(0, sOutFileName.length()-13);
	else
		modOutFileName = sOutFileName;

	string filename = CPLGetFilename(modOutFileName.c_str());
	double geomatrix[6];
	outDS->GetGeoTransform(geomatrix);

	string WCSURL = mp_Conf->Get_OUTPUT_PREFIX_URL();
	string sOutFileURL = WCSURL + CPLGetFilename(modOutFileName.c_str());

	string eoNamespace = "xmlns:eop=\"http://www.opengis.net/eop/2.0\" "
			"xmlns:gml=\"http://www.opengis.net/gml/3.2\" "
			"xmlns:gmlcov=\"http://www.opengis.net/gmlcov/1.0\" "
			"xmlns:om=\"http://www.opengis.net/om/2.0\" "
			"xmlns:ows=\"http://www.opengis.net/ows/2.0\" "
			"xmlns:swe=\"http://www.opengis.net/swe/2.0\" "
			"xmlns:wcseo=\"http://www.opengis.net/wcseo/1.0\" "
			"xmlns:xlink=\"http://www.w3.org/1999/xlink\" "
			"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
			"xsi:schemaLocation=\"http://www.opengis.net/wcseo/1.0 http://schemas.opengis.net/wcseo/1.0/wcsEOAll.xsd\"";

	string beginDT = EQUAL(absDS->GetCoverageBeginTime().c_str(), "") ? ms_RequestBeginTime : absDS->GetCoverageBeginTime();
	string endDT = EQUAL(absDS->GetCoverageEndTime().c_str(), "") ? ms_RequestEndTime : absDS->GetCoverageEndTime();
	int fields = absDS->GetBandList().size();

	ostringstream outStream;
	outStream << "<wcseo:RectifiedDataset gml:id=\"" + filename + "\" " << eoNamespace << ">" << endl;
	outStream << "  <gml:boundedBy>" <<endl;
	outStream << "    <gml:Envelope axisLabels=\"lon lat\" srsDimension=\"2\" srsName=\"http://www.opengis.net/def/crs/EPSG/0/4326\" uomLabels=\"deg deg\">" << endl;
	outStream << "      <gml:lowerCorner>" << md_RequestMinX << " " << md_RequestMinY << "</gml:lowerCorner>" <<endl;
	outStream << "      <gml:upperCorner>" << md_RequestMaxX << " " << md_RequestMaxY << "</gml:upperCorner>" <<endl;
	outStream << "    </gml:Envelope>" <<endl;
	outStream << "  </gml:boundedBy>" <<endl;

	outStream << "  <gml:domainSet>" <<endl;
	outStream << "    <gml:RectifiedGrid dimension=\"2\" gml:id=\"" << filename + "_grid" << "\" >" <<endl;
	outStream << "      <gml:limits>" <<endl;
	outStream << "        <gml:GridEnvelope>" <<endl;
	outStream << "          <gml:low>0 0</gml:low>" <<endl;
	outStream << "          <gml:high>" << outDS->GetRasterXSize()-1 << " " << outDS->GetRasterYSize()-1 << "</gml:high>" <<endl;
	outStream << "        </gml:GridEnvelope>" <<endl;
	outStream << "      </gml:limits>" <<endl;
	outStream << "      <gml:axisLabels>lon lat</gml:axisLabels>" <<endl;
	outStream << "      <gml:origin>" <<endl;
	outStream << "        <gml:Point gml:id=\"" << filename + "_grid_origin\" srsName=\"http://www.opengis.net/def/crs/EPSG/0/4326\">" <<endl;
	outStream << "          <gml:pos>" << geomatrix[0] << " " << geomatrix[3] <<"</gml:pos>" <<endl;
	outStream << "        </gml:Point>" <<endl;
	outStream << "      </gml:origin>" <<endl;
	outStream << "      <gml:offsetVector srsName=\"http://www.opengis.net/def/crs/EPSG/0/4326\">" << geomatrix[1] << " 0.0</gml:offsetVector>" <<endl;
	outStream << "      <gml:offsetVector srsName=\"http://www.opengis.net/def/crs/EPSG/0/4326\">0.0 " << geomatrix[5] << "</gml:offsetVector>" <<endl;
	outStream << "    </gml:RectifiedGrid>" <<endl;
	outStream << "  </gml:domainSet>" <<endl;

	outStream << "  <gml:rangeSet>" <<endl;
	outStream << "    <gml:File>" <<endl;
	outStream << "      <gml:rangeParameters xlink:arcrole=\"fileReference\" xlink:role=\"" << ms_OutputContentType << "\" />" <<endl;
	outStream << "      <gml:fileReference>" << sOutFileURL << "</gml:fileReference>" <<endl;
	outStream << "      <gml:fileStructure/>" <<endl;
	outStream << "      <gml:mimeType>" << ms_OutputContentType << "</gml:mimeType>" <<endl;
	outStream << "    </gml:File>" <<endl;
	outStream << "  </gml:rangeSet>" <<endl;

	outStream << "  <gmlcov:rangeType>" <<endl;
	outStream << "    <swe:DataRecord>" <<endl;
	for(int i = 1; i <= fields; i++)
	{
		double dfMin=0.0, dfMax=0.0, dfMean=0.0, dfStdDev=0.0;
		GDALRasterBandH	hBand = GDALGetRasterBand(absDS->GetGDALDataset(), i);
		GDALGetRasterStatistics( hBand, true, true, &dfMin, &dfMax, &dfMean, &dfStdDev );
	outStream << "     <swe:field name=\"" << StrTrims(absDS->GetDatasetName(), "\"") + "_field_" + convertToString(i) << "\">" <<endl;
	outStream << "        <swe:Quantity definition=\"http://www.opengis.net/def/property/OGC/0/" << absDS->GetFieldQuantityDef() << "\">" <<endl;
	outStream << "        <swe:description>" << absDS->GetDataTypeName() + ", the number " << convertToString(i) << " filed of " <<absDS->GetDatasetName() << "</swe:description>" <<endl;
	outStream << "          <swe:nilValues>"<< absDS->GetMissingValue()<<"</swe:nilValues>" <<endl;
	outStream << "          <swe:constraint>" <<endl;
	outStream << "            <swe:AllowedValues>" <<endl;
	outStream << "	              <swe:min>" << dfMin << "</swe:min>" <<endl;
	outStream << "	              <swe:max>" << dfMax << "</swe:max>" <<endl;
	outStream << "            </swe:AllowedValues>" <<endl;
	outStream << "          </swe:constraint>" <<endl;
	outStream << "        </swe:Quantity>" <<endl;
	outStream << "      </swe:field>" <<endl;
	}
	outStream << "    </swe:DataRecord>" <<endl;
	outStream << "  </gmlcov:rangeType>" <<endl;

	outStream << "  <gmlcov:metadata>" <<endl;
	if(absDS->GetMetaDataList().size() > 1)	
		outStream << "	    " << absDS->GetMetaDataList().at(1) <<endl;
	outStream << "    <wcseo:lineage>" <<endl;
	outStream << "      <wcseo:referenceGetCoverage>" <<endl;
	outStream << "        <ows:Reference xlink:href=\"" << ms_requestFullURL << "\"/>" <<endl;
	outStream << "      </wcseo:referenceGetCoverage>" <<endl;
	outStream << "      <gml:timePosition>" << GetTimeString(2) << "</gml:timePosition>" <<endl;
	outStream << "    </wcseo:lineage>" <<endl;
	outStream << "  </gmlcov:metadata>" <<endl;
	outStream << "</wcseo:RectifiedDataset>" << endl;

	ms_eoMetadataContents = outStream.str();

	WCSTDestroyDataset(absDS);
	GDALClose(outDS);

	return CE_None;
}

/************************************************************************/
/*                         GetCoverageInitial()                         */
/************************************************************************/

/**
 * \brief Initialize the coverage object.
 *
 * This method is used to initialize the AbstractDataset object based
 * on coverage identifier and band list, also query the DatasetObject
 * from the dataset/datasetSeries configuration files.
 *
 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr WCS_GetCoverage::GetCoverageInitial()
{
	AbstractDataset* absDS = absDS = WCSTCreateDataset(ms_CovID.c_str(), mvi_BandList, 0);;

	mp_AbsDS.reset(absDS);

	double geomatrix[6];
	double geoMinMax[4];
	absDS->GetGeoTransform(geomatrix);
	absDS->GetGeoMinMax(geoMinMax);

	if(mb_SubsetSpatial)
	{
		md_OutGeoTransform[0] = md_RequestMinX;
		md_OutGeoTransform[3] = md_RequestMaxY;
	}
	else
	{
		md_RequestMinX = geoMinMax[0];
		md_RequestMaxX = geoMinMax[1];
		md_RequestMinY = geoMinMax[2];
		md_RequestMaxY = geoMinMax[3];

		md_OutGeoTransform[0] = geomatrix[0];
		md_OutGeoTransform[3] = geomatrix[3];
	}

	md_OutGeoTransform[1] = geomatrix[1];
	md_OutGeoTransform[2] = geomatrix[2];
	md_OutGeoTransform[4] = geomatrix[4];
	md_OutGeoTransform[5] = geomatrix[5];

	string::size_type idx = ms_OutputFormat.find("/");
	if (idx != string::npos)
		ms_OutputFormat = ms_OutputFormat.substr(idx + 1);

	//CreateISO19115Metadata(dsObj);//Build ISO-19115 meta data

	return CE_None;
}

/************************************************************************/
/*                          HttpDirectoryRespond()                      */
/************************************************************************/

/**
 * \brief Delivery the output file stream to user directly.
 *
 * This method is used to delivery the output file stream to user directly.
 *
 * @param sOutFileName The path of the response file needs to be delivered..
 *
 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr WCS_GetCoverage::HttpDirectoryRespond(const string& sOutFileName)
{
	ifstream ifs(sOutFileName.c_str(), ios::binary);
	if (!ifs)
	{
		SetWCS_ErrorLocator("WCS_GetCoverage::HttpDirectoryRespond()");
		WCS_Error(CE_Failure, OGC_WCS_InvalidParameterValue,
				"Failed to open output file.");

		return CE_Failure;
	}

	long temp = ifs.tellg();
	ifs.seekg(0, ios_base::end);
	long filesize = ifs.tellg();
	ifs.seekg(0, ios::beg);

	ms_OutputContentType += "\r\nContent-Disposition: attachment; filename=";
	ms_OutputContentType += CPLGetFilename(sOutFileName.c_str());

	cout << "Content-Length: " << filesize << endl;
	cout << ms_OutputContentType << endl << endl;
	cout << ifs.rdbuf() << endl;

	return CE_None;
}

/************************************************************************/
/*                            HttpStoreRespond()                        */
/************************************************************************/

/**
 * \brief Delivery the output file URL to user.
 *
 * This method is used to delivery the output file URL to user.
 *
 * @param sOutFileName The path of the response file needs to be delivered..
 *
 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr WCS_GetCoverage::HttpStoreRespond(const string& sOutFileName)
{
	string TMPURL = mp_Conf->Get_OUTPUT_PREFIX_URL();
	string sTmpOutFileName = TMPURL + CPLGetFilename(sOutFileName.c_str());

	string oTmp;
	oTmp = "<Coverages xmlns=\"http://www.opengis.net/wcs/1.0\"\n";
	oTmp += "xmlns:ows=\"http://www.opengis.net/ows\"\n";
	oTmp += "xmlns:owcs=\"http://www.opengis.net/wcs/1.1/ows\"\n";
	oTmp += "xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n";
	oTmp += "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n";
	oTmp += "xsi:schemaLocation=\"http://schemas.opengis.net/ows/1.1.0 ../owsCoverages.xsd\">\n";
	oTmp += "  <Coverage>\n";
	oTmp += "     <Abstract>Coverage created from GetCoverage operation request to a WCS</Abstract>\n";
	oTmp += "     <Reference xlink:href=\"" + sTmpOutFileName + "\"\n";
	oTmp += "     xlink:role=\"urn:ogc:def:role:WCS:1.1:coverage\"/>\n";
	oTmp += "  </Coverage>\n";
	oTmp += "</Coverages>\n";

	if (mb_SoapRequest)
	{
		cout << "Content-Type: application/xml+soap"<< endl << endl
		 << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl
				<< "<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">"
				<< endl << "<soap:Body>" << endl << oTmp << endl
				<< "/soap:Body>" << endl << "/<soap:Envelope>" << endl;
	}
	else
	{
		cout << "Content-Type: text/xml" << endl << endl;
		cout << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl << oTmp << endl;
	}

	return CE_None;
}

/************************************************************************/
/*                    HttpMultiPartsDirectoryRespond()                  */
/************************************************************************/

/**
 * \brief Delivery the output file stream to user and the metadata to browser.
 *
 * This method is used to delivery the output file stream to user and
 * the metadata to browser.
 *
 * @param sOutFileName The path of the response file needs to be delivered..
 *
 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr WCS_GetCoverage::HttpMultiPartsDirectoryRespond(const string& sOutFileName)
{
	ifstream ifs(sOutFileName.c_str(), ios::binary);
	string filename = string(CPLGetFilename(sOutFileName.c_str()));
	if (!ifs)
	{
		SetWCS_ErrorLocator("WCS_GetCoverage::HttpMultiPartsDirectoryRespond()");
		WCS_Error(CE_Failure, OGC_WCS_InvalidParameterValue,
				"Failed to open output file.");
		return CE_Failure;
	}

	//CreateEOMetadata(sOutFileName);

	int temp = ifs.tellg();
	ifs.seekg(0, ios_base::end);
	long filesize = ifs.tellg();
	ifs.seekg(0, ios::beg);

	ms_OutputContentType += "\r\nContent-Disposition: attachment; filename=";
	ms_OutputContentType += CPLGetFilename(sOutFileName.c_str());

	cout << "Content-Type: multipart/mixed; boundary=\"gmueowcs\"" << endl << endl;
	cout << "--gmueowcs" << endl;
	cout << "Content-Length: " << filesize << endl;
	cout << ms_OutputContentType << endl << endl;
	cout << ifs.rdbuf() <<endl;
	cout << "--gmueowcs" << endl;
	cout << "Content-Type: text/xml" << endl << endl;
	cout << ms_eoMetadataContents << endl << endl;
	cout << "--gmueowcs--" << endl;

	return CE_None;
}

/************************************************************************/
/*                            SetCRSFromURN()                           */
/************************************************************************/

/**
 * \brief Set the OGRSpatialReference object based on CRS code.
 *
 * This method is used to set the OGRSpatialReference object based on CRS code.
 *
 * @param crs_ID The OGRSpatialReference object to store CRS information
 * corresponding to specified CRS code.
 *
 * @param CRS_urn The CRS code.
 *
 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr WCS_GetCoverage::SetCRSFromURN(OGRSpatialReference& crs_ID, const char* CRS_urn)
{
	if (NULL == CRS_urn || *CRS_urn == '\0')
		return CE_Warning;

	if (Find_Compare_SubStr(CRS_urn, "OGC:") && Find_Compare_SubStr(CRS_urn, ":84"))
	{
		crs_ID.SetWellKnownGeogCS("WGS84");
		return CE_None;
	}
	else if (Find_Compare_SubStr(CRS_urn, ":IMAGECRS"))
	{
		crs_ID.SetLocalCS("OGC:imageCRS");
		return CE_None;
	}
	else if (OGRERR_NONE == crs_ID.SetFromUserInput(CRS_urn))
		return CE_None;
	else
	{
		SetWCS_ErrorLocator("WCS_GetCoverage::setCRSFromURN()");
		WCS_Error(CE_Failure, OGC_WCS_NoApplicableCode, "Failed to set up CRS.");
		return CE_Failure;
	}

	return CE_None;
}

/************************************************************************/
/*                       CreateOutputFileSuffix()                       */
/************************************************************************/

/**
 * \brief Create the output suffix corresponding specified format.
 *
 * This method is used to create the output suffix corresponding specified
 * format, also the format code in GDAL and the content type (MIME type)
 * will be created.

 * @return The string of suffix.
 */

string WCS_GetCoverage::CreateOutputFileSuffix()
{
	if (EQUAL(ms_OutputFormat.c_str(), "TIFF") ||
		EQUAL(ms_OutputFormat.c_str(), "GEOTIFF"))
	{
		ms_OutputFormatCode = "GTIFF";
		ms_OutputContentType = "Content-Type: image/tiff";
		return ".tif";
	}
	else if (EQUAL(ms_OutputFormat.c_str(),"NetCDF") ||
			EQUAL(ms_OutputFormat.c_str(),"x-netcdf"))
	{
		ms_OutputFormatCode = "netCDF";
		ms_OutputContentType = "Content-Type: application/x-netcdf";
		return ".nc";
	}
	else if (EQUAL(ms_OutputFormat.c_str(),"JPEG2000"))
	{
		ms_OutputFormatCode = "JPEG2000";
		ms_OutputContentType = "Content-Type: image/JPEG2000";
		return ".jp2";
	}
	else if (EQUAL(ms_OutputFormat.c_str(),"JP2KAK"))
	{
		ms_OutputFormatCode = "JP2KAK";
		ms_OutputContentType = "Content-Type: image/JPEG2000";
		return ".j2k";
	}
	else if (EQUAL(ms_OutputFormat.c_str(),"JPEG"))
	{
		ms_OutputFormatCode = "JPEG";
		ms_OutputContentType = "Content-Type: image/jpeg";
		return ".jpg";
	}
	else if (EQUAL(ms_OutputFormat.c_str(),"PNG"))
	{
		ms_OutputFormatCode = "PNG";
		ms_OutputContentType = "Content-Type: image/png";
		return ".png";
	}
	else if (EQUAL(ms_OutputFormat.c_str(),"HDF4") ||
			EQUAL(ms_OutputFormat.c_str(),"HDF4Image") ||
			EQUAL(ms_OutputFormat.c_str(),"HDF") ||
			EQUAL(ms_OutputFormat.c_str(),"x-hdf") )
	{
		ms_OutputFormatCode = "HDF4Image";
		ms_OutputContentType = "Content-Type: application/x-hdf";
		return ".hdf";
	}
	else if (EQUAL(ms_OutputFormat.c_str(),"HDF5") ||
			EQUAL(ms_OutputFormat.c_str(),"HDF5Image"))
	{
		ms_OutputFormatCode = "HDF5";
		ms_OutputContentType = "Content-Type: application/x-hdf";//Need to be checked
		return ".he5";
	}
	else if (EQUAL(ms_OutputFormat.c_str(),"GIF"))
	{
		ms_OutputFormatCode = "GIF";
		ms_OutputContentType = "Content-Type: image/gif";
		return ".gif";
	}
	else if (EQUAL(ms_OutputFormat.c_str(),"NITF") ||
			EQUAL(ms_OutputFormat.c_str(),"vnd.iptc.nitf") )
	{
		ms_OutputFormatCode = "NITF";
		ms_OutputContentType = "Content-Type: text/vnd.iptc.nitf";
		return ".ntf";
	}
	else if (EQUAL(ms_OutputFormat.c_str(),"x-hdfeos"))
	{
		ms_OutputFormatCode = "HDFEOS";//GDAL do not support HDF-EOS export, will be re-written.
		ms_OutputContentType = "Content-Type: application/x-hdfeos";
		return ".hdf";
	}
	else if (EQUAL(ms_OutputFormat.c_str(),"Binary"))
		return ".dat";
	else
	{
		ms_OutputFormatCode = "";
		SetWCS_ErrorLocator("WCS_GetCoverage::FormOutputFileSubfix()");
		WCS_Error(CE_Failure, OGC_WCS_InvalidParameterValue, "The specified format is not supported.");
		return ".dat";
	}
}

/************************************************************************/
/*                           CreateBinaryFile()                         */
/************************************************************************/

/**
 * \brief Create the file in binary stream.
 *
 * This method is used to create the file in binary stream.

 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr WCS_GetCoverage::CreateBinaryFile(const string &sOutFileName)
{
	int nXSize = mp_AbsDS->GetImageXSize();
	int nYSize = mp_AbsDS->GetImageYSize();

	int nband = mp_AbsDS->GetImageBandCount();
	GDALDataType eDataType = mp_AbsDS->GetGDALDataset()->GetRasterBand(1)->GetRasterDataType();
	int bufSize = nXSize * nYSize * GDALGetDataTypeSize(eDataType) / 8;
	char *pData = (char *) CPLMalloc(bufSize);
	if (pData == NULL)
		return CE_Failure;

	ofstream datafile(sOutFileName.c_str(), ios::binary);

	for (int i = 1; i <= nband; i++)
	{
		if (CE_None != mp_AbsDS->GetGDALDataset()->GetRasterBand(i)->RasterIO(
				GF_Read, 0, 0, nXSize, nYSize, pData, nXSize, nYSize, eDataType, 0, 0))
		{
			VSIFree((char*) pData);
			return CE_Failure;
		}
		datafile.write(pData, bufSize);
	}

	VSIFree((char*) pData);

	return CE_None;
}

//Convert the geographic coordinates from decimal to package degree format (DDDMMMSSS.SS).
double decimal2PDF(double llcoord)
{
	int degree = int(llcoord);
	double wholeMinute = (llcoord - degree)*60;
	int minute = int(wholeMinute);
	double second = (wholeMinute - minute) * 60;

	return degree * 1000000 + minute * 1000 + second;
}

CPLErr WCS_GetCoverage::CreateHDFEOS2File(const string& sSourceFile, string hdfeosFile)
{
	/*
	GDALDataset* dsSource = (GDALDataset*) GDALOpen(sSourceFile.c_str(), GA_ReadOnly);
	double fill = mp_AbsDS->GetMissingValue();
	int bands = dsSource->GetRasterCount();
	int x = dsSource->GetRasterXSize();
	int y = dsSource->GetRasterYSize();

	string gridname = mp_AbsDS->GetDataTypeName();
	string fieldname= mp_AbsDS->GetDatasetName();

	double gt[6];
	dsSource->GetGeoTransform(gt);

	OGRSpatialReference sr(dsSource->GetProjectionRef());
	char *psTargetSRS;
	sr.exportToWkt(&psTargetSRS);

	float64 xmin = gt[0];
	float64 xmax = gt[0] + x * gt[1];
	float64 ymax = gt[3];
	float64 ymin = gt[3] + y * gt[5];

	int32 gdIDout;
	int32 fileIDout, status;
	double *projParameter = NULL;
	int32 projCode,zoneCode,sphereCode,originCode=0;
	float64 upperLeft[2], lowerRight[2];
	long int projCode_l,zoneCode_l,sphereCode_l;

	sr.exportToUSGS(&projCode_l, &zoneCode_l, &projParameter, &sphereCode_l);

	projCode = (int32)projCode_l;
	zoneCode = (int32)zoneCode_l;
	sphereCode = (int32)sphereCode_l;

	if(projCode == 0)
	{
		upperLeft[0] = decimal2PDF(xmin); upperLeft[1] = decimal2PDF(ymax);
		lowerRight[0] = decimal2PDF(xmax); lowerRight[1] = decimal2PDF(ymin);
	}else
	{
		upperLeft[0] = xmin; upperLeft[1] = ymax;
		lowerRight[0] = xmax; lowerRight[1] = ymin;
	}

	char fieldDimList[32] = {"YDim,XDim"};
	int32 edge[2] = {y, x};

	if((fileIDout = GDopen((char*)hdfeosFile.c_str(), DFACC_CREATE)) == FAIL ||
	 (gdIDout = GDcreate(fileIDout, (char*)gridname.c_str(), x, y, upperLeft, lowerRight))==FAIL ||
	 GDdefproj(gdIDout, projCode, zoneCode, sphereCode, projParameter) == FAIL ||
	 GDdeforigin(gdIDout, originCode) == FAIL ||
	 GDdefcomp(gdIDout, HDFE_COMP_NONE, NULL) == FAIL)
	{
		SetWCS_ErrorLocator("WCS_GetCoverage::CreateHDFEOS2File()");
		WCS_Error(CE_Failure, OGC_WCS_NoApplicableCode, "Failed to create HDF-EOS2 file.");
		return CE_Failure;
	}


	for(int i = 1; i <= bands; i++)
	{
		string fieldnames = (bands > 1) ? fieldname + convertToString(i) : fieldname ;
		GDALDataType edt = dsSource->GetRasterBand(i)->GetRasterDataType();
		int bufSize = x * y * GDALGetDataTypeSize(edt) / 8;;
		char* pData = (char*) CPLMalloc(bufSize);
		dsSource->GetRasterBand(i)->RasterIO(GF_Read, 0, 0, x, y, pData, x, y, edt, 0, 0);

		switch (edt)
		{
		case GDT_Byte:
		{
			status = GDdeffield(gdIDout, (char*)fieldnames.c_str(), fieldDimList, DFNT_CHAR, HDFE_NOMERGE);
			break;
		}
		case GDT_UInt16:
		{
			status = GDdeffield(gdIDout, (char*)fieldnames.c_str(), fieldDimList, DFNT_UINT16, HDFE_NOMERGE);
			break;
		}
		case GDT_Int16:
		{
			status = GDdeffield(gdIDout, (char*)fieldnames.c_str(), fieldDimList, DFNT_INT16, HDFE_NOMERGE);
			break;
		}
		case GDT_UInt32:
		{
			status = GDdeffield(gdIDout, (char*)fieldnames.c_str(), fieldDimList, DFNT_UINT32, HDFE_NOMERGE);
			break;
		}
		case GDT_Int32:
		{
			status = GDdeffield(gdIDout, (char*)fieldnames.c_str(), fieldDimList, DFNT_INT32, HDFE_NOMERGE);
			break;
		}
		case GDT_Float32:
		{
			status = GDdeffield(gdIDout, (char*)fieldnames.c_str(), fieldDimList, DFNT_FLOAT32, HDFE_NOMERGE);
			break;
		}
		case GDT_Float64:
		{
			status = GDdeffield(gdIDout, (char*)fieldnames.c_str(), fieldDimList, DFNT_FLOAT64, HDFE_NOMERGE);
			break;
		}
		default:
			status = GDdeffield(gdIDout, (char*)fieldnames.c_str(), fieldDimList, DFNT_INT16, HDFE_NOMERGE);
		}

		if(status == FAIL)
		{
			SetWCS_ErrorLocator("WCS_GetCoverage::CreateHDFEOS2File()");
			WCS_Error(CE_Failure, OGC_WCS_NoApplicableCode, "Failed to attach a field to HDF-EOS2 file.");
			return CE_Failure;
		}

		if(GDwritefield(gdIDout, (char*)fieldnames.c_str(), NULL, NULL, edge, (VOIDP)pData)==FAIL)
		{
			SetWCS_ErrorLocator("WCS_GetCoverage::CreateHDFEOS2File()");
			WCS_Error(CE_Failure, OGC_WCS_NoApplicableCode, "Failed to write the data to HDF-EOS2 file.");
			return CE_Failure;
		}
		GDsetfillvalue(gdIDout, (char*)fieldnames.c_str(), &fill);

		VSIFree((char*)pData);
	}

	vector<string> meteList = mp_AbsDS->GetMetaDataList();//Adding the metadata to the output
	int meteSize = (int)meteList.size();
	for(int i = 0; i < meteSize; i++)
	{
		string curname  = meteList.at(i).substr(0, meteList.at(i).find("="));
		string curvalue = meteList.at(i).substr(meteList.at(i).find("=")+1);
		if(	!EQUAL(curname.c_str(), "TIFFTAG_XRESOLUTION") &&
			!EQUAL(curname.c_str(), "TIFFTAG_YRESOLUTION")&&
			!EQUAL(curname.c_str(), "TIFFTAG_RESOLUTIONUNIT") &&
			!EQUAL(curname.c_str(), "INPUTPOINTER"))
		{
			if(EQUAL(curname.c_str(), "EASTBOUNDINGCOORDINATE"))
				curvalue = convertToString(md_RequestMinX);
			else if(EQUAL(curname.c_str(), "WESTBOUNDINGCOORDINATE"))
				curvalue = convertToString(md_RequestMaxX);
			else if(EQUAL(curname.c_str(), "SOUTHBOUNDINGCOORDINATE"))
				curvalue = convertToString(md_RequestMinY);
			else if(EQUAL(curname.c_str(), "NORTHBOUNDINGCOORDINATE"))
				curvalue = convertToString(md_RequestMaxY);

			GDwriteattr(gdIDout, (char*)curname.c_str(), DFNT_CHAR, curvalue.length(), (VOIDP)curvalue.c_str());
		}
	}
	GDwriteattr(gdIDout, (char*)"EOMetadataContents", DFNT_CHAR, ms_eoMetadataContents.length(), (VOIDP)ms_eoMetadataContents.c_str());

	GDdetach(gdIDout);
	GDclose(fileIDout);

	GDALClose(dsSource);
	return CE_None;
*/
	return CE_None;
}


/************************************************************************/
/*                         SetOutputResolution()                        */
/************************************************************************/

/**
 * \brief Set the output resolution based on specified resolution or width/height.
 *
 * This method is used to set the output resolution based on specified
 * resolution or width/height.

 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr WCS_GetCoverage::SetOutputResolution()
{
	if (mvi_OutputWH.empty() && mvd_OutputResXY.empty())
		return CE_None;

	if (!mvi_OutputWH.empty() && mvd_OutputResXY.empty())
	{
		md_OutGeoTransform[1] = mi_OutputWidth * md_OutGeoTransform[1]	/ mvi_OutputWH[0];
		md_OutGeoTransform[5] = mi_OutputHeight * md_OutGeoTransform[5]	/ mvi_OutputWH[1];
		mi_OutputWidth = mvi_OutputWH[0];
		mi_OutputHeight = mvi_OutputWH[1];
	}
	else if (mvi_OutputWH.empty() && !mvd_OutputResXY.empty())
	{
		mi_OutputWidth = mi_OutputWidth * md_OutGeoTransform[1]	/ mvd_OutputResXY[0];
		mi_OutputHeight = mi_OutputHeight * fabs(md_OutGeoTransform[5])	/ mvd_OutputResXY[1];
		md_OutGeoTransform[1] = mvd_OutputResXY[0];
		md_OutGeoTransform[5] = mvd_OutputResXY[1];
	}
	else
	{
		SetWCS_ErrorLocator("WCS_GetCoverage::SetOutputResolution()");
		WCS_Error(CE_Failure, OGC_WCS_InvalidParameterValue, "Invalid Parameter Value of \"Width/Height/ResX/ResY\".");
		return CE_Failure;
	}

	return CE_None;
}

/************************************************************************/
/*                              ExeCommand()                            */
/************************************************************************/

/**
 * \brief Execute the command line by calling system function.
 *
 * This method is used to execute the command line by calling system function.

 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr WCS_GetCoverage::ExeCommand(string logFilePath, string cmd)
{
	int logf = open(logFilePath.c_str(),  O_RDWR | O_APPEND);
	if(logf == -1)
		logf = open(logFilePath.c_str(),  O_CREAT, S_IWRITE);
	if(logf < 0) logf = 0;

	int savefd = dup(STDOUT_FILENO);
	dup2(logf, STDOUT_FILENO);

	int exestatus = system(cmd.c_str());

	dup2(savefd, STDOUT_FILENO);
	if(exestatus < 0)
	{
		SetWCS_ErrorLocator("WCS_GetCoverage::ExeCommand()");
		WCS_Error(CE_Failure, OGC_WCS_InvalidParameterValue, "Failed to executed the commane line.");
		return CE_Failure;
	}
	return CE_None;
}

/************************************************************************/
/*                          eateOutputFile()                            */
/************************************************************************/

/**
 * \brief Create the output file.
 *
 * This method is used to create the output file.

 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr WCS_GetCoverage::CreateOutputFile(const string& sOutFileName)
{
	string tmpcoverageid = ms_CovGDALID;
	string tmpwarpgeotifffile = sOutFileName + ".tmp.warp.tif";
	string tmpgmljp2box = sOutFileName + ".txt";

	//step 1, create a temporary GeoTIFF file for TRMM data, without spatial subset.
	int bTRMMData = ms_CovGDALID.find("TRMM") != string::npos ? true : false;
	int bHDF5Data = ms_CovGDALID.find("HDF5") != string::npos ? true : false;
	int bGOESData = ms_CovGDALID.find("GOES:NETCDF") != string::npos ? true : false;
	int bNITFData = ms_CovGDALID.find("NITF") != string::npos ? true : false;
	if(bTRMMData || bHDF5Data || bGOESData) //For TRMM data and OMI data
	{
		tmpcoverageid = sOutFileName + ".tmp.tif";
		GDALDataset* srcDS = (GDALDataset*)mp_AbsDS->GetGDALDataset();
		GDALDriverH hOutDriver = GDALGetDriverByName("GTIFF");
		GDALDatasetH hOutDS = GDALCreateCopy(hOutDriver, tmpcoverageid.c_str(), srcDS, FALSE, NULL, NULL, NULL);
		GDALClose(srcDS);
		GDALClose(hOutDS);
	}

	if(bNITFData)
	{
		tmpcoverageid = mp_AbsDS->GetResourceFileName();
	}

	//step 2, warp the temporary file with the requested spatial extent, fill value and still geotiff format.
	string m_sWarpCmdPath = mp_Conf->Get_GDAL_WARP_PATH();
	string m_sWarpCmdContent = m_sWarpCmdPath + " -q -of GTiff";

	if(ms_ResponseCRS_URN != "")//User specified output CRS
	{
		if(mb_SubsetSpatial && !mo_RequestedCRS.IsSame(&mo_ResponseCRS))
		{
			My2DPoint llPt(md_RequestMinX, md_RequestMinY);
			My2DPoint urPt(md_RequestMaxX, md_RequestMaxY);
			if (CE_None != bBox_transFormmate(mo_RequestedCRS, mo_ResponseCRS, llPt, urPt))
			{
				SetWCS_ErrorLocator( "WCS_GetCoverage::CreateOutputFile()");
				WCS_Error(CE_Failure, OGC_WCS_NoApplicableCode, "Failed to transform bbox coordinate from request CRS to response CRS.");
				return CE_Failure;
			}
			md_RequestMinX = llPt.mi_X;
			md_RequestMinY = llPt.mi_Y;
			md_RequestMaxX = urPt.mi_X;
			md_RequestMaxY = urPt.mi_Y;
		}

		m_sWarpCmdContent += " -t_srs " + ms_ResponseCRS_URN;
	}
	else if(ms_RequestCRS_URN != "")
		m_sWarpCmdContent += " -t_srs " + ms_RequestCRS_URN;


	if(mb_SubsetSpatial)
		m_sWarpCmdContent += " -te " + convertToString(md_RequestMinX) + " " + convertToString(md_RequestMinY) +
						" " + convertToString(md_RequestMaxX) + " " + convertToString(md_RequestMaxY);

	if(!mvi_OutputWH.empty())
		m_sWarpCmdContent += " -ts " +  convertToString(mvi_OutputWH.at(0)) + " " + convertToString(mvi_OutputWH.at(1));
	else if(!mvd_OutputResXY.empty())
		m_sWarpCmdContent += " -tr " +  convertToString(mvd_OutputResXY.at(0)) + " " + convertToString(mvd_OutputResXY.at(1));

	m_sWarpCmdContent += " -dstnodata " + convertToString(mp_AbsDS->GetMissingValue());
	m_sWarpCmdContent += " -r " + ms_Interpolation;
	m_sWarpCmdContent += " " + tmpcoverageid + " " + tmpwarpgeotifffile;

	if(CE_None != ExeCommand(mp_Conf->Get_WCS_LOGFILE_PATH(), m_sWarpCmdContent))
	{
		SetWCS_ErrorLocator("WCS_GetCoverage::CreateOutputFile");
		WCS_Error(CE_Failure, OGC_WCS_InvalidParameterValue, "Failed to execute the GDAL command line in the back end.");
		return CE_Failure;
	}

	//Yuanzheng Shao, need to write the metadata to file in GeoTIFF ?
	CreateEOMetadata(tmpwarpgeotifffile);

	//backward compatibility
	//Yuanzheng Shao, 2012-07-22
	if(EQUAL(ms_OutputFormatCode.c_str(), "HDFEOS"))
	{
		if(CE_None == CreateHDFEOS2File(tmpwarpgeotifffile, sOutFileName))
		{
			unlink(tmpwarpgeotifffile.c_str());
			return CE_None;
		}
		else
		{
			return CE_Failure;
		}
	}
	//In order to support GMLJP2
	//JPEG2000 as output format code in GDAL. But in fact, kdu_compress command was used to convert GeoTIFF
	//to JPEG2000, which could be served with ESA JPIP server.
	else if(EQUAL(ms_OutputFormatCode.c_str(), "JPEG2000"))
	{
		WCS_DescribeCoverage* dc = new WCS_DescribeCoverage();
		string describeCoverageResponse =  dc->CreateDescibeCoverageXMLByCoverageID(ms_CovGDALID);
		string gmljp2Metadata = doPostFromString("http://129.174.131.8:9003/ows9_ic/gmljp2Converter?", describeCoverageResponse);
		ofstream gmljp2boxfile(tmpgmljp2box.c_str());
		gmljp2boxfile<<"xml "<<gmljp2Metadata;//ms_eoMetadataContents;

		ms_eoMetadataContents = gmljp2Metadata;

		string m_sKduCompressCmdPath = mp_Conf->Get_KAKADU_COMPRESS_PATH();
		string m_skduCompressCmdContent = m_sKduCompressCmdPath + " -i " + tmpwarpgeotifffile;
		m_skduCompressCmdContent += " -o " + sOutFileName + " -jp2_box " + tmpgmljp2box + " ORGgen_plt=yes Creversible=yes";
		if(CE_None != ExeCommand(mp_Conf->Get_WCS_LOGFILE_PATH(), m_skduCompressCmdContent))
		{
			SetWCS_ErrorLocator("WCS_GetCoverage::CreateOutputFile");
			WCS_Error(CE_Failure, OGC_WCS_InvalidParameterValue, "Failed to execute the Kakadu command line in the back end.");
			return CE_Failure;
		}
	}
	//GMU WCS still supports other data format export for NITF data products.
	else
	{
		//step 3, create the warp result with specified format
		GDALDataset* warpDS = (GDALDataset*) GDALOpen(tmpwarpgeotifffile.c_str(), GA_ReadOnly);

		vector<string> meteList = mp_AbsDS->GetMetaDataList();//Adding the metadata to the output
		int meteSize = (int)meteList.size();
		for(int i = 0; i < meteSize; i++)
		{
			string curname  = meteList.at(i).substr(0, meteList.at(i).find("="));
			string curvalue = meteList.at(i).substr(meteList.at(i).find("=")+1);
			if(	!EQUAL(curname.c_str(), "TIFFTAG_XRESOLUTION") &&
				!EQUAL(curname.c_str(), "TIFFTAG_YRESOLUTION")&&
				!EQUAL(curname.c_str(), "TIFFTAG_RESOLUTIONUNIT") &&
				!EQUAL(curname.c_str(), "INPUTPOINTER"))
			{

				if(EQUAL(curname.c_str(), "EASTBOUNDINGCOORDINATE"))
					curvalue = convertToString(md_RequestMinX);
				else if(EQUAL(curname.c_str(), "WESTBOUNDINGCOORDINATE"))
					curvalue = convertToString(md_RequestMaxX);
				else if(EQUAL(curname.c_str(), "SOUTHBOUNDINGCOORDINATE"))
					curvalue = convertToString(md_RequestMinY);
				else if(EQUAL(curname.c_str(), "NORTHBOUNDINGCOORDINATE"))
					curvalue = convertToString(md_RequestMaxY);

				warpDS->SetMetadataItem(curname.c_str(), curvalue.c_str(), "");
			}
		}
		warpDS->SetMetadataItem("EOMetadataContents", ms_eoMetadataContents.c_str(), "");

		GDALDriverH hReturnDriver = GDALGetDriverByName(ms_OutputFormatCode.c_str());//temporary method for TRMM data
		GDALDatasetH hReturnDS = GDALCreateCopy(hReturnDriver, sOutFileName.c_str(), warpDS, FALSE, NULL, NULL, NULL);
		GDALClose(warpDS);
		GDALClose(hReturnDS);

		//step 4, delete temporary files
		if(bTRMMData || bHDF5Data)
			unlink(tmpcoverageid.c_str());
		unlink(tmpwarpgeotifffile.c_str());
	}

	return CE_None;
}

/************************************************************************/
/*                           WCST_Respond()                             */
/************************************************************************/

/**
 * \brief The enter point for WCS_GetCoverage class (Command line environment).
 *
 * This method is the enter point for WCS_GetCoverage class, which
 * is used under command line environment. In order to debug WCS and use
 * WCS as a command line, the user could issue a request under command line.
 * The response information will be stored in the specified file.
 *
 * @param sOutFileName The path of the response file.
 */

void WCS_GetCoverage::WCST_Respond(string& sOutFileName)
{
	if (CE_None != GetCoverageInitial())
	{
		SendHttpHead();
		cout << GetWCS_ErrorMsg() << endl;
		return;
	}

	if (sOutFileName.empty() || sOutFileName == "")
		sOutFileName = MakeTempFile(mp_Conf->Get_TEMPORARY_OUTPUT_DIRECTORY(), ms_CovGDALID, CreateOutputFileSuffix());

	if(EQUAL(ms_OutputFormatCode.c_str(), ""))
	{
		SendHttpHead();
		cout << GetWCS_ErrorMsg() << endl;
		return;
	}

	CreateOutputFile(sOutFileName);

	return;
}

/************************************************************************/
/*                           WCST_Respond()                             */
/************************************************************************/

/**
 * \brief The enter point for WCS_GetCoverage class (CGI environment).
 *
 * This method is the enter point for WCS_GetCoverage class, which
 * is used under CGI environment. The response information will be displayed
 * in the browser.
 */

void WCS_GetCoverage::WCST_Respond()
{
	if (CE_None != GetCoverageInitial())
	{
		SendHttpHead();
		cout << GetWCS_ErrorMsg() << endl;
		return;
	}

	string sOutFileName = MakeTempFile(mp_Conf->Get_TEMPORARY_OUTPUT_DIRECTORY(), ms_CovGDALID, CreateOutputFileSuffix());

	if(EQUAL(ms_OutputFormatCode.c_str(), ""))
	{
		SendHttpHead();
		cout << GetWCS_ErrorMsg() << endl;
		return;
	}

	if (CE_None != CreateOutputFile(sOutFileName))
	{
		SendHttpHead();
		cout << GetWCS_ErrorMsg() << endl;
		return;
	}

	if (mb_IsStore)
		HttpStoreRespond(sOutFileName);
	else if(mb_MultiPart)
	{
		HttpMultiPartsDirectoryRespond(sOutFileName);
	}
	else
	{
		HttpDirectoryRespond(sOutFileName);
		unlink(sOutFileName.c_str());
	}

	return;
}


