/******************************************************************************
 * $Id: WCS_DescribeCoverager.cpp 2011-07-19 16:24:00Z $
 *
 * Project:  The Open Geospatial Consortium (OGC) Web Coverage Service (WCS)
 * 			 for Earth Observation: Open Source Reference Implementation
 * Purpose:  WCS_DescribeCoverage class implementation
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

#include "WCS_DescribeCoverage.h"

/************************************************************************/
/* ==================================================================== */
/*                        WCS_DescribeCoverage                          */
/* ==================================================================== */
/************************************************************************/

/**
 * \class WCS_DescribeCoverage "WCS_DescribeCoverage.h"
 *
 * This class is used to handle DescribeCoverage and DescribeEOCoverageSet
 * request. Several functions for parsing request parameters, generating
 * the response XML document, are provided.
 */


WCS_DescribeCoverage::WCS_DescribeCoverage()
{

}

/************************************************************************/
/*                        WCS_DescribeCoverage()                        */
/************************************************************************/

/**
 * \brief Constructor of a WCS_DescribeCoverage object.
 *
 * This is the accepted method of creating an WCS_DescribeCoverage object.
 *
 * @param conf String of the full path of the configuration file.
 */

WCS_DescribeCoverage::WCS_DescribeCoverage(const string& conf) : WCS_T(conf)
{
	mB_SubsetSpatialLat = false;
	mB_SubsetSpatialLon = false;
	mB_SubsetTemporalBegin = false;
	mB_SubsetTemporalEnd = false;
	mb_DescribeEOCoverage = false;
}

WCS_DescribeCoverage::~WCS_DescribeCoverage()
{

}

/************************************************************************/
/*                      GetReqMessageFromXMLTree()                      */
/************************************************************************/

/**
 * \brief Fetch the DescribeCoverage request parameters from XML tree object.
 *
 * This method is used to fetch the DescibeCoverage and DescribeEOCoverageSet
 * parameters from an XML object (HTTP POST method). The coverage identifier(s)
 * will be fetched.
 *
 *Sample request XML document
 * @code
<?xml version="1.0" encoding="UTF-8"?>
<DescribeCoverage
  xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xmlns:wcs="http://www.opengis.net/wcs/2.0"
  xmlns:gml="http://www.opengis.net/gml/3.2"
  xsi:schemaLocation="http://schemas.opengis.net/wcs/2.0 ../wcsAll.xsd"
  service="WCS" version="2.0.0">
  <wcs:CoverageId>MOD05_L2.A2008205.0255.005.2008206170326.hdf:Water_Vapor_Infrared</wcs:CoverageId>
  <wcs:CoverageId>MOD13C1.A2007145.005.2007184062524.hdf:NDVI</wcs:CoverageId>
</DescribeCoverage>
 * @endcode
 *
 * @param xmlRoot XML Node object created by GDAL library.
 *
 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr WCS_DescribeCoverage::GetReqMessageFromXMLTree(CPLXMLNode *xmlRoot)
{
	if (!xmlRoot)
	{
		SetWCS_ErrorLocator("WCS_DescribeCoverage::GetReqMessageFromXMLTree()");
		WCS_Error(CE_Failure, OGC_WCS_MissingParameterValue,
				"XMl Root Node Equal NULL.");
		return CE_Failure;
	}

	mv_CovIDs = WCSTGetXMLValueList(xmlRoot, "CoverageId");

	return CE_None;
}

/************************************************************************/
/*                    GetReqMessageFromURLString()                      */
/************************************************************************/

/**
 * \brief Fetch the request parameters from URL string.
 *
 * This method is used to fetch the DescibeCoverage and DescribeEOCoverageSet
 * parameters from an URL string (HTTP GET method). The coverage identifier(s)
 * will be fetched.
 *
 * @param urlStr String of the DescibeCoverage or DescribeEOCoverageSet request.
 *
 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr WCS_DescribeCoverage::GetReqMessageFromURLString(const string& urlStr)
{
	if (urlStr.empty() || urlStr == "")
	{
		SetWCS_ErrorLocator("WCS_DescribeCoverage::GetReqMessageFromURLString");
		WCS_Error(CE_Failure, OGC_WCS_MissingParameterValue, "No Requested Content.");
		return CE_Failure;
	}

	vector<string> strVec;
	KVPsReader kvps(urlStr, '&');

	string tmpStr1 = kvps.getValue("EOId", "");
	string tmpStr2 = kvps.getValue("coverageid", "");
	if (tmpStr1 != "")
	{
		unsigned int nDesCov = CsvburstCpp(tmpStr1, strVec, ',');
		for (unsigned int i = 0; i < nDesCov; ++i)
		{
			mv_CovIDs.push_back(StrTrim(strVec[i]));
		}
		mb_DescribeEOCoverage = true;

		vector<string> valueList = kvps.getValues("subset");
		if(valueList.size() > 0)
		{
			for(unsigned int i = 0; i < valueList.size(); i++)
			{
				string tmpv = valueList[i];
				if(Find_Compare_SubStr(tmpv, "Lat"))
				{
					mB_SubsetSpatialLat = true;
					vector<double> latV;
					GetSubSetLatLon(tmpv, latV);
					md_RequestMinY = latV[0];
					md_RequestMaxY = latV[1];
				}
				else if(Find_Compare_SubStr(tmpv, "Long") || Find_Compare_SubStr(tmpv, "lon"))
				{
					mB_SubsetSpatialLon = true;
					vector<double> latV;
					GetSubSetLatLon(tmpv, latV);
					md_RequestMinX = latV[0];
					md_RequestMaxX = latV[1];
				}
				else if(Find_Compare_SubStr(tmpv, "phenomenonTime"))
				{
					vector<string> timeV;
					GetSubSetTime(tmpv, timeV);
					if(timeV.size() == 1)
					{
						mB_SubsetTemporalBegin = true;
						mB_SubsetTemporalEnd = false;
						ms_RequestBeginTime = timeV[0];
						ms_RequestEndTime = "";
					}
					else if(timeV.size() == 2)
					{
						mB_SubsetTemporalBegin = true;
						mB_SubsetTemporalEnd = true;
						ms_RequestBeginTime = timeV[0];
						ms_RequestEndTime = timeV[1];
					}
				}
			}
		}
		else
		{
			mB_SubsetSpatialLat = false;
			mB_SubsetSpatialLon = false;
			mB_SubsetTemporalBegin = false;
			mB_SubsetTemporalEnd = false;
		}

	}
	else if(tmpStr2 != "")
	{
		unsigned int nDesCov = CsvburstCpp(tmpStr2, strVec, ',');
		for (unsigned int i = 0; i < nDesCov; ++i)
		{
			mv_CovIDs.push_back(StrTrim(strVec[i]));
		}
		mb_DescribeEOCoverage = false;
	}

	return CE_None;
}

/************************************************************************/
/*                   CreateDescribeCoverageXMLHead()                    */
/************************************************************************/

/**
 * \brief Create the XML head element for DescribeCoverage response.
 *
 * This method is used to create the XML head element for DescribeCoverage
 * response and append the response to the output stream.
 *
 * @param outStream Stream object used to generate DescribeCoverage response.
 */

void WCS_DescribeCoverage::CreateDescribeCoverageXMLHead(ostringstream& outStream)
{
	outStream
			<< "<wcs:CoverageDescriptions " << endl
			<< "xmlns:eop=\"http://www.opengis.net/eop/2.0\" " << endl
			<< "xmlns:gml=\"http://www.opengis.net/gml/3.2\" " << endl
			<< "xmlns:gmlcov=\"http://www.opengis.net/gmlcov/1.0\" " <<endl
			<< "xmlns:om=\"http://www.opengis.net/om/2.0\" " << endl
			<< "xmlns:swe=\"http://www.opengis.net/swe/2.0\" " <<endl
			<< "xmlns:wcs=\"http://www.opengis.net/wcs/2.0\" " <<endl
			<< "xmlns:wcseo=\"http://www.opengis.net/wcseo/1.0\" " <<endl
			<< "xmlns:xlink=\"http://www.w3.org/1999/xlink\" " << endl
			<< "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" " << endl
			<< "xsi:schemaLocation=\"http://www.opengis.net/wcseo/1.0 http://schemas.opengis.net/wcseo/1.0/wcsEOAll.xsd\">" << endl;
	return;
}

/************************************************************************/
/*               CreateDescribeEOCoverageSetXMLHead()                   */
/************************************************************************/

/**
 * \brief Create the XML head element for DescribeEOCoverageSet response.
 *
 * This method is used to create the XML head element for DescribeEOCoverageSet
 * response and append the response to the output stream.
 *
 * @param outStream Stream object used to generate DescribeEOCoverageSet response.
 */

void WCS_DescribeCoverage::CreateDescribeEOCoverageSetXMLHead(ostringstream& outStream)
{
	outStream
			<< "<wcseo:EOCoverageSetDescription " <<endl
			<< "xmlns:eop=\"http://www.opengis.net/eop/2.0\" " << endl
			<< "xmlns:gml=\"http://www.opengis.net/gml/3.2\" " << endl
			<< "xmlns:gmlcov=\"http://www.opengis.net/gmlcov/1.0\" " <<endl
			<< "xmlns:om=\"http://www.opengis.net/om/2.0\" " << endl
			<< "xmlns:swe=\"http://www.opengis.net/swe/2.0\" " <<endl
			<< "xmlns:wcs=\"http://www.opengis.net/wcs/2.0\" " <<endl
			<< "xmlns:wcseo=\"http://www.opengis.net/wcseo/1.0\" " <<endl
			<< "xmlns:xlink=\"http://www.w3.org/1999/xlink\" " << endl
			<< "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" " << endl
			<< "xsi:schemaLocation=\"http://www.opengis.net/wcseo/1.0 http://schemas.opengis.net/wcseo/1.0/wcsEOAll.xsd\">" << endl;
	return;
}

/************************************************************************/
/*                     QueryFromDatasetSeries()                         */
/************************************************************************/

/**
 * \brief Query the dataset array from a datasetSeries object.
 *
 * This method is used to query the dataset array from DatasetSeries object
 * based on specified spatial-temporal parameters.
 *
 * @param dso DatasetSeries object.
 *
 * @return The array of result Dataset object.
 */

vector<DatasetObject> WCS_DescribeCoverage::QueryFromDatasetSeries(DatasetSeriesObject& dso)
{
	vector<DatasetObject> doV;

	int latpass = false, lonpass = false, timepass = false;

	for(unsigned int i = 0; i < dso.mv_dataset.size(); i++)
	{
		DatasetObject ds = dso.mv_dataset.at(i);

		if(mB_SubsetSpatialLon)
			lonpass = (md_RequestMinX >= ds.m_minx && md_RequestMinX <= ds.m_maxx) || (md_RequestMaxX >= ds.m_minx && md_RequestMaxX <= ds.m_maxx);
		else
			lonpass = true;

		if(mB_SubsetSpatialLat)
			latpass = (md_RequestMinY >= ds.m_miny && md_RequestMinY <= ds.m_maxy) || (md_RequestMaxY >= ds.m_miny && md_RequestMaxY <= ds.m_maxy);
		else
			latpass = true;

		if(mB_SubsetTemporalEnd)
		{
			if((CompareDateTime_GreaterThan(ms_RequestBeginTime, ds.m_beginTime) > 0 && CompareDateTime_GreaterThan(ms_RequestBeginTime, ds.m_endTime) < 0 )
				|| (CompareDateTime_GreaterThan(ms_RequestEndTime, ds.m_beginTime) > 0 && CompareDateTime_GreaterThan(ms_RequestEndTime, ds.m_endTime) < 0 ))
				timepass= true;
			else
				timepass= false;
		}
		else if(mB_SubsetTemporalBegin)
		{
			if(CompareDateTime_GreaterThan(ms_RequestBeginTime, ds.m_beginTime) > 0 && CompareDateTime_GreaterThan(ms_RequestBeginTime, ds.m_endTime) < 0 )
				timepass= true;
			else
				timepass= false;
		}
		else
			timepass= true;

		if(latpass && lonpass &&timepass)
			doV.push_back(ds);
	}

	return doV;
}

/************************************************************************/
/*                   CreateOneCoverageDescription()                     */
/************************************************************************/

/**
 * \brief Create the coverage description contents for the Dataset object.
 *
 * This method is used to create the coverage description XMl elements for
 * specified Dataset object, The generated XML element will be appended to
 * the output stream.
 *
 * @param outStream Stream object used to generate response.
 *
 * @param dataset Dataset object which includes some attribute information about this dataset.
 */

void WCS_DescribeCoverage::CreateOneCoverageDescription(ostringstream& outStream, DatasetObject& dsObj)
{
	vector<int> bandV;//Need to be checked.
	AbstractDataset* absDS = WCSTCreateDataset(dsObj.m_covGDALID.c_str(), bandV, 1);
	string covSubType = absDS->GetCoverageSubType();
	double geomatrix[6];
	double nativebbox[4];
	absDS->GetGeoTransform(geomatrix);
	absDS->GetNativeBBox(nativebbox);
	int bandNum = absDS->GetImageBandCount();
	dsObj.m_covName = CPLGetFilename(absDS->GetResourceFileName().c_str());
	dsObj.m_minx = nativebbox[0];
	dsObj.m_maxx = nativebbox[1];
	dsObj.m_miny = nativebbox[2];
	dsObj.m_maxy = nativebbox[3];

	int utmzone = absDS->GetNativeCRS().GetUTMZone();
	int bLatlon = absDS->GetNativeCRS().IsGeographic() ? true : false;
	string axisLabels = bLatlon ? "lon lat" : "x y";
	string uomLabels =  bLatlon ? "deg deg" : "m m";
	string srsName =  bLatlon ? "http://www.opengis.net/def/crs/EPSG/0/4326" :
			"http://www.opengis.net/def/crs/EPSG/0/326" + convertToString(utmzone);

	//Create boundedBy part
	outStream << "    <wcs:CoverageDescription gml:id=\"" + dsObj.m_covName + "\">" <<endl;
	outStream << "      <gml:boundedBy>" <<endl;
	outStream << "        <gml:Envelope axisLabels=\"" << axisLabels << "\" srsDimension=\"2\" srsName=\"" << srsName << "\" uomLabels=\"" << uomLabels << "\">" << endl;
	outStream << "          <gml:lowerCorner>" << dsObj.m_minx << " " << dsObj.m_miny << "</gml:lowerCorner>" <<endl;
	outStream << "          <gml:upperCorner>" << dsObj.m_maxx << " " << dsObj.m_maxy << "</gml:upperCorner>" <<endl;
	outStream << "        </gml:Envelope>" <<endl;
	outStream << "      </gml:boundedBy>" <<endl;

	//Create CoverageId part
	outStream << "	    <wcs:CoverageId>" << dsObj.m_covGDALID << "</wcs:CoverageId>" <<endl;

	//Create ServiceParameters part
	outStream << "	    <wcs:ServiceParameters>" <<endl;
	outStream << "	      <wcs:CoverageSubtype>" << covSubType << "</wcs:CoverageSubtype>" << endl;
	outStream << "	    </wcs:ServiceParameters>" <<endl;

	//Create domainSet part
	outStream << "	    <gml:domainSet>" <<endl;
	if(EQUAL(covSubType.c_str(), "RectifiedDataset"))
	{
	outStream << "	      <gml:RectifiedGrid dimension=\"2\" gml:id=\"" << dsObj.m_covName + "_grid" << "\">" <<endl;
	outStream << "	        <gml:limits>" <<endl;
	outStream << "	          <gml:GridEnvelope>" <<endl;
	outStream << "	            <gml:low>0 0</gml:low>" <<endl;
	outStream << "	            <gml:high>" << absDS->GetImageXSize()-1 << " " << absDS->GetImageYSize()-1 << "</gml:high>" <<endl;
	outStream << "	          </gml:GridEnvelope>" <<endl;
	outStream << "	        </gml:limits>" <<endl;
	outStream << "	        <gml:axisLabels>" << axisLabels << "</gml:axisLabels>" <<endl;

	outStream << "	        <gml:origin>" <<endl;
	outStream << "	          <gml:Point gml:id=\"" << dsObj.m_covName + "_grid_origin\" srsName=\"" << srsName << "\">" <<endl;
	outStream << "	            <gml:pos>" << geomatrix[0] << " " << geomatrix[3] <<"</gml:pos>" <<endl;
	outStream << "	          </gml:Point>" <<endl;
	outStream << "	        </gml:origin>" <<endl;
	outStream << "	        <gml:offsetVector srsName=\"" << srsName << "\">" << geomatrix[1] << " 0.0</gml:offsetVector>" <<endl;
	outStream << "	        <gml:offsetVector srsName=\"" << srsName << "\">0.0 " << geomatrix[5] << "</gml:offsetVector>" <<endl;
	outStream << "	      </gml:RectifiedGrid>" <<endl;
	}
	else
	{
	outStream << "	      <gml:Grid dimension=\"2\" gml:id=\"" << dsObj.m_covName + "_swath" << "\">" <<endl;
	outStream << "	        <gml:limits>" <<endl;
	outStream << "	          <gml:GridEnvelope>" <<endl;
	outStream << "	            <gml:low>0 0</gml:low>" <<endl;
	outStream << "	            <gml:high>" << absDS->GetImageXSize() << " " << absDS->GetImageYSize() << "</gml:high>" <<endl;
	outStream << "	          </gml:GridEnvelope>" <<endl;
	outStream << "	        </gml:limits>" <<endl;
	outStream << "	        <gml:axisLabels>line frame</gml:axisLabels>" <<endl;
	outStream << "	      </gml:Grid>" <<endl;
	}
	outStream << "	    </gml:domainSet>" <<endl;

	//Create rangeType part
	outStream << "	    <gmlcov:rangeType>" <<endl;
	outStream << "	      <swe:DataRecord>" <<endl;
	for(int i = 1; i <= bandNum; i++)
	{
		double dfMin=0.0, dfMax=0.0, dfMean=0.0, dfStdDev=0.0;
		GDALRasterBandH	hBand = GDALGetRasterBand(absDS->GetGDALDataset(), i);
		GDALGetRasterStatistics( hBand, true, true, &dfMin, &dfMax, &dfMean, &dfStdDev );
	outStream << "	        <swe:field name=\"" << StrTrims(absDS->GetDatasetName(), "\"") + "_field_" + convertToString(i) << "\">" <<endl;
	outStream << "	          <swe:Quantity definition=\"http://www.opengis.net/def/property/OGC/0/" << absDS->GetFieldQuantityDef() << "\">" <<endl;
	outStream << "            <swe:description>" << absDS->GetDataTypeName() + ", the number " << convertToString(i) << " filed of " <<absDS->GetDatasetName() << "</swe:description>" <<endl;
	outStream << "	          <swe:nilValues>"<< absDS->GetMissingValue()<<"</swe:nilValues>" <<endl;
	outStream << "	          <swe:constraint>" <<endl;
	outStream << "	            <swe:AllowedValues>" <<endl;
	outStream << "	              <swe:min>" << dfMin << "</swe:min>" <<endl;
	outStream << "	              <swe:max>" << dfMax << "</swe:max>" <<endl;
	outStream << "	            </swe:AllowedValues>" <<endl;
	outStream << "	          </swe:constraint>" <<endl;
	outStream << "	          </swe:Quantity>" <<endl;
	outStream << "	        </swe:field>" <<endl;
	}
	outStream << "	      </swe:DataRecord>" <<endl;
	outStream << "	    </gmlcov:rangeType>" <<endl;

	//Create metadata part
	outStream << "	    <gmlcov:metadata>" <<endl;
	if(absDS->GetMetaDataList().size() > 1)
		outStream << "	    " << absDS->GetMetaDataList().at(1) <<endl;
	outStream << "	    </gmlcov:metadata>" <<endl;
	outStream << "    </wcs:CoverageDescription>" << endl;

	WCSTDestroyDataset(absDS);
}

/************************************************************************/
/*               CreateOneDatasetSeriesDescription()                    */
/************************************************************************/

/**
 * \brief Create the coverage description contents for the DatasetSeries object.
 *
 * This method is used to create the coverage description XMl elements for
 * specified DatasetSeries object, The generated XML element will be appended to
 * the output stream.
 *
 * @param outStream Stream object used to generate response.
 *
 * @param dsSeriesObj DatasetSeries object which includes some attribute information about this datasetSeries.
 */

void WCS_DescribeCoverage::CreateOneDatasetSeriesDescription(ostringstream& outStream, DatasetSeriesObject& dsSeriesObj)
{
	outStream << "  <wcseo:DatasetSeriesDescriptions>" <<endl;
	outStream << "    <wcseo:DatasetSeriesDescription gml:id=\"" << dsSeriesObj.m_covName << "\">" <<endl;
	outStream << "      <gml:boundedBy>" <<endl;
	outStream << "        <gml:Envelope axisLabels=\"lon lat\" srsDimension=\"2\" srsName=\"http://www.opengis.net/def/crs/EPSG/0/4326\" uomLabels=\"deg deg\">" <<endl;
	outStream << "          <gml:lowerCorner>" << dsSeriesObj.m_minx << " " << dsSeriesObj.m_miny << "</gml:lowerCorner>" <<endl;
	outStream << "          <gml:upperCorner>" << dsSeriesObj.m_maxx << " " << dsSeriesObj.m_maxy << "</gml:upperCorner>" <<endl;
	outStream << "        </gml:Envelope>" <<endl;
	outStream << "      </gml:boundedBy>" <<endl;
	outStream << "      <wcseo:DatasetSeriesId>" << dsSeriesObj.m_covName << "</wcseo:DatasetSeriesId>" <<endl;
	outStream << "      <gml:TimePeriod gml:id=\"" << dsSeriesObj.m_covName << "_timeperiod\">" <<endl;
	outStream << "        <gml:beginPosition>" << dsSeriesObj.m_beginTime << "</gml:beginPosition>" <<endl;
	outStream << "        <gml:endPosition>" << dsSeriesObj.m_endTime << "</gml:endPosition>" <<endl;
	outStream << "      </gml:TimePeriod>" <<endl;
	outStream << "    </wcseo:DatasetSeriesDescription>" <<endl;
	outStream << "  </wcseo:DatasetSeriesDescriptions>" <<endl;
}

/************************************************************************/
/*               CreateDescribeCoverageXMLTree()                        */
/************************************************************************/

/**
 * \brief Create the whole response for DescribeCoverage or
 * DescribeEOCoverageSet request.
 *
 * This method is used to create the whole response for DescribeCoverage
 * or DescribeEOCoverageSet request.
 *
 * @param outStream Stream object used to generate response.
 *
 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr WCS_DescribeCoverage::CreateDescribeCoverageXMLTree(ostringstream& outStream)
{
	if(mb_DescribeEOCoverage)
		CreateDescribeEOCoverageSetXMLHead(outStream);
	else
		CreateDescribeCoverageXMLHead(outStream);

	for(unsigned int i = 0; i < mv_CovIDs.size(); i++)
	{
		string curID = mv_CovIDs.at(i);

		if(mb_DescribeEOCoverage) //The request equals to DescribeEOCoverageset
		{
			DatasetSeriesObject dssObj = InitializeDatasetSeriesByID(curID);
			if(dssObj.m_covName == "")
			{
				SetWCS_ErrorLocator("CreateDescribeCoverageXMLTree");
				WCS_Error(CE_Failure, OGC_WCS_NoApplicableCode,
						"No such coverage.");
				return CE_Failure;
			}
			else
			{
				CreateOneDatasetSeriesDescription(outStream, dssObj);
				vector<DatasetObject> datasetVec = QueryFromDatasetSeries(dssObj);
				outStream << "  <wcs:CoverageDescriptions>" <<endl;
				for(unsigned int j = 0; j < datasetVec.size(); j++)
					CreateOneCoverageDescription(outStream, datasetVec.at(j));
				outStream << "  </wcs:CoverageDescriptions>" <<endl;
			}
		}

		else //The request equals to DescribeCoverage
		{
			//DatasetObject dsObj = InitializeDatasetByID(curID);
			//  |
			// \ /

			DatasetObject dsObj;
			dsObj.m_covName = curID;
			dsObj.m_covGDALID = curID;

			if(EQUAL(dsObj.m_covName.c_str(), ""))
			{
				SetWCS_ErrorLocator("CreateDescribeCoverageXMLTree");
				WCS_Error(CE_Failure, OGC_WCS_NoApplicableCode,
						"No such coverage.");
				return CE_Failure;
			}else
				CreateOneCoverageDescription(outStream, dsObj);
		}
	}//end for each request coverage id

	if(mb_DescribeEOCoverage)
		outStream << "</wcseo:EOCoverageSetDescription>" <<endl;
	else
		outStream << "</wcs:CoverageDescriptions>" <<endl;

	return CE_None;
}

/************************************************************************/
/*           CreateDescibeCoverageXMLByCoverageID()                     */
/************************************************************************/

/**
 * \brief Create the whole response for DescribeCoverage request.
 *
 * This method is used to create the whole response for DescribeCoverage request
 * based on coverage identifier.
 *
 * @param coverageID coverage identifier.
 *
 * @return DescribeCoverage response for specific coverage identifier.
 */
string WCS_DescribeCoverage::CreateDescibeCoverageXMLByCoverageID(string coverageID)
{
	ostringstream osstrm;
	CreateDescribeCoverageXMLHead(osstrm);
	DatasetObject dsObj;
	dsObj.m_covName = coverageID;
	dsObj.m_covGDALID = coverageID;

	if(EQUAL(dsObj.m_covName.c_str(), ""))
		return NULL;
	else
		CreateOneCoverageDescription(osstrm, dsObj);
	osstrm << "</wcs:CoverageDescriptions>" <<endl;

	return osstrm.str();
}

/************************************************************************/
/*                           WCST_Respond()                             */
/************************************************************************/

/**
 * \brief The enter point for WCS_DescribeCoverage class (Command line environment).
 *
 * This method is the enter point for WCS_DescribeCoverage class, which
 * is used under command line environment. In order to debug WCS and use
 * WCS as a command line, the user could issue a request under command line.
 * The response information will be stored in the specified file.
 *
 * @param sOutFileName The path of the response file.
 */

void WCS_DescribeCoverage::WCST_Respond(string& sOutFileName)
{
	if (sOutFileName.empty() || EQUAL(sOutFileName.c_str(), ""))
		sOutFileName = MakeTempFile(mp_Conf->Get_TEMPORARY_OUTPUT_DIRECTORY(), "", ".xml");

	ostringstream osstrm;
	CreateDescribeCoverageXMLTree(osstrm);

	ofstream outStream(sOutFileName.c_str());
	outStream << osstrm.str() << endl;
	outStream.close();

	return;
}

/************************************************************************/
/*                           WCST_Respond()                             */
/************************************************************************/

/**
 * \brief The enter point for WCS_DescribeCoverage class (CGI environment).
 *
 * This method is the enter point for WCS_DescribeCoverage class, which
 * is used under CGI environment. The response information will be displayed
 * in the browser.
 */

void WCS_DescribeCoverage::WCST_Respond()
{
	ostringstream osstrm;

	osstrm <<"Content-Type: text/xml"<<endl<<endl;
	osstrm << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"<< endl;

	if(CE_None != CreateDescribeCoverageXMLTree(osstrm))
	{
		SendHttpHead();
		cout << GetWCS_ErrorMsg()<< endl;
		return;
	}

	cout << osstrm.str()<<endl;

	return;
}
