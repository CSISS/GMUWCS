/******************************************************************************
 * $Id: WCS_GetCapabilities.cpp 2011-07-19 16:24:00Z $
 *
 * Project:  The Open Geospatial Consortium (OGC) Web Coverage Service (WCS)
 * 			 for Earth Observation: Open Source Reference Implementation
 * Purpose:  WCS_GetCapabilities class implementation
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

#include "WCS_GetCapabilities.h"

/************************************************************************/
/* ==================================================================== */
/*                        WCS_DescribeCoverage                          */
/* ==================================================================== */
/************************************************************************/

/**
 * \class WCS_GetCapabilities "WCS_GetCapabilities.h"
 *
 * This class is used to handle GetCapabilities request. Several functions
 * for parsing request parameters, generating the response XML document,
 * are provided.
 */

WCS_GetCapabilities::WCS_GetCapabilities()
{
}

/************************************************************************/
/*                        WCS_GetCapabilities()                         */
/************************************************************************/

/**
 * \brief Constructor of a WCS_GetCapabilities object.
 *
 * This is the accepted method of creating an WCS_GetCapabilities object.
 *
 * @param conf String of the full path of the configuration file.
 */

WCS_GetCapabilities::WCS_GetCapabilities(const string& conf) : WCS_T(conf)
{
	mv_sections.clear();
	mb_SoapRequest = 0;
}

WCS_GetCapabilities::~WCS_GetCapabilities()
{

}

/************************************************************************/
/*                      GetReqMessageFromXMLTree()                      */
/************************************************************************/

/**
 * \brief Fetch the request GetCapabilities parameters from XML tree object.
 *
 * This method is used to fetch the GetCapabilities parameters from an
 * XML object (HTTP POST method).
 *
 *Sample request XML document
 * @code
<?xml version="1.0" encoding="UTF-8"?>
<GetCapabilities
  xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xmlns:ows="http://www.opengis.net/ows/1.1"
  xmlns:wcs="http://www.opengis.net/wcs/2.0"
  xsi:schemaLocation=
    "http://schemas.opengis.net/wcs/2.0 ../wcsAll.xsd"
  service="WCS">
  <ows:AcceptVersions>
    <ows:Version>2.0.0</ows:Version>
  </ows:AcceptVersions>
</GetCapabilities>
 * @endcode
 *
 * @param xmlRoot XML Node object created by GDAL library.
 *
 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr WCS_GetCapabilities::GetReqMessageFromXMLTree(CPLXMLNode *xmlRoot)
{
	if (!xmlRoot)
	{
		SetWCS_ErrorLocator( "WCS_GetCapabilities::GetReqMessageFromXMLTree()");
		WCS_Error(CE_Failure, OGC_WCS_MissingParameterValue, "XMl Root Node Equal NULL.");
		return CE_Failure;
	}

	mv_sections = WCSTGetXMLValueList(xmlRoot, "Section");

	return CE_None;
}

/************************************************************************/
/*                    GetReqMessageFromURLString()                      */
/************************************************************************/

/**
 * \brief Fetch the request parameters from URL string.
 *
 * This method is used to fetch the GetCapabilities parameters from an URL
 * string (HTTP GET method).
 *
 * @param urlStr String of the GetCapabilities request.
 *
 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr WCS_GetCapabilities::GetReqMessageFromURLString(const string& urlStr)
{
	if (urlStr.empty() || urlStr == "")
	{
		SetWCS_ErrorLocator( "WCS_GetCapabilities::GetReqMessageFromURLString()");
		WCS_Error(CE_Failure, OGC_WCS_MissingParameterValue, "No Requested Content.");
		return CE_Failure;
	}

	vector<string> strVec;
	KVPsReader kvps(urlStr, '&');
	string tmpStr = kvps.getValue("Section", "");

	if (tmpStr != "")
	{
		unsigned int nDesCov = CsvburstCpp(tmpStr, strVec, ',');
		for (unsigned int i = 0; i < nDesCov; ++i)
		{
			mv_sections.push_back(StrTrim(strVec[i]));
		}
	}

	return CE_None;
}

/************************************************************************/
/*                   CreateCapabilitiesXMLTree()                        */
/************************************************************************/

/**
 * \brief Create the whole response for GetCapabilities request.
 *
 * This method is used to create the whole response for GetCapabilities request.
 *
 * @param outStream Stream object used to generate response.
 */

void WCS_GetCapabilities::CreateCapabilitiesXMLTree(ostringstream& ostream)
{
	CreateCapabilitiesXMLHead(ostream);
	if (mv_sections.empty())
	{
		WriteServiceIdentification(ostream);
		WriteServiceProvider(ostream);
		WriteOperationsMetadata(ostream);
		ostream <<"  <wcs:ServiceMetadata version=\"1.0.0\"/>" << endl; //Add on 08/02, for TEAM Engine testing - p01 
		WriteContents(ostream);
	}
	else
	{
		for (unsigned int i = 0; i < mv_sections.size(); i++)
		{
			if (Find_Compare_SubStr(mv_sections[i], "ServiceIdentification"))
			{
				WriteServiceIdentification(ostream);
			}
			else if (Find_Compare_SubStr(mv_sections[i], "ServiceProvider"))
			{
				WriteServiceProvider(ostream);
			}
			else if (Find_Compare_SubStr(mv_sections[i], "OperationsMetadata"))
			{
				WriteOperationsMetadata(ostream);
			}
			else if (Find_Compare_SubStr(mv_sections[i], "Contents"))
			{
				WriteContents(ostream);
			}
		}
	}
	ostream << "</wcs:Capabilities>" << endl;

	return ;
}

/************************************************************************/
/*                     CreateCapabilitiesXMLHead()                      */
/************************************************************************/

/**
 * \brief Create the XML head element for GetCapabilities response.
 *
 * This method is used to create the XML head element for GetCapabilities
 * response and append the response to the output stream. In this implementation
 * of WCS, the XML head elements are read from configuration file.
 *
 * @param outStream Stream object used to generate GetCapbilities response.
 */

void WCS_GetCapabilities::CreateCapabilitiesXMLHead(ostringstream& ostream)
{
	ifstream ifile(mp_Conf->Get_CAPABILITIES_HEAD_FILE_PATH().c_str());
	if(ifile)
	{
		ostream << ifile.rdbuf();
	}
	ifile.close();

}

/************************************************************************/
/*                     WriteServiceIdentification()                     */
/************************************************************************/

/**
 * \brief Create the Service Identifier element for GetCapabilities response.
 *
 * This method is used to create the Service Identifier element for GetCapabilities
 * response and append the response to the output stream. In this implementation
 * of WCS, the Service Identifier elements are read from configuration file.
 *
 * @param outStream Stream object used to generate GetCapbilities response.
 */

void WCS_GetCapabilities::WriteServiceIdentification(ostringstream& ostream)
{
	ifstream ifile(mp_Conf->Get_CAPABILITIES_SEVICEIDENTIFICATION_FILE_PATH().c_str());
	if(ifile)
	{
		ostream << ifile.rdbuf();
	}
	ifile.close();
}

/************************************************************************/
/*                         WriteServiceProvider()                       */
/************************************************************************/

/**
 * \brief Create the Service Provider element for GetCapabilities response.
 *
 * This method is used to create the Service Provider element for GetCapabilities
 * response and append the response to the output stream. In this implementation
 * of WCS, the Service Provider elements are read from configuration file.
 *
 * @param outStream Stream object used to generate GetCapbilities response.
 */

void WCS_GetCapabilities::WriteServiceProvider(ostringstream& ostream)
{
	ifstream ifile(mp_Conf->Get_CAPABILITIES_SEVICEPROVIDER_FILE_PATH().c_str());
	if(ifile)
	{
		ostream << ifile.rdbuf();
	}
	ifile.close();
}

/************************************************************************/
/*                         WriteServiceProvider()                       */
/************************************************************************/

/**
 * \brief Create the Operations Metadata element for GetCapabilities response.
 *
 * This method is used to create the Operations Metadata element for GetCapabilities
 * response and append the response to the output stream. In this implementation
 * of WCS, the Operations Metadata elements are read from configuration file.
 *
 * @param outStream Stream object used to generate GetCapbilities response.
 */

void WCS_GetCapabilities::WriteOperationsMetadata(ostringstream& ostream)
{
	ifstream ifile(mp_Conf->Get_CAPABILITIES_OPERATIONSMETADA_FILE_PATH().c_str());
	if(ifile)
	{
		ostream << ifile.rdbuf();
	}
	ifile.close();
}

/************************************************************************/
/*                         WriteServiceProvider()                       */
/************************************************************************/

/**
 * \brief Create the Contents element for GetCapabilities response.
 *
 * This method is used to create the Contents element for GetCapabilities
 * response and append the response to the output stream. In this implementation
 * of WCS, the Contents elements are read from configuration file.
 *
 * @param outStream Stream object used to generate GetCapbilities response.
 */

void WCS_GetCapabilities::WriteContents(ostringstream& ostream)
{
	ifstream ifile(mp_Conf->Get_CAPABILITIES_CONTENTS_FILE_PATH().c_str());
	if(ifile)
	{
		ostream << ifile.rdbuf();
		ifile.close();
	}
	else
	{
		InitializeConfigurationFiles();

		ostream << "  <wcs:Contents>" << endl;

		if(!mv_datasetCoverage.empty())
		{
			for (unsigned int i = 0; i < mv_datasetCoverage.size(); i++)
			{
				string covName = mv_datasetCoverage.at(i).m_covName;
				string covType = (Find_Compare_SubStr(covName, "MOD05_L2") || Find_Compare_SubStr(covName, "OMI-Aura_L2-") || Find_Compare_SubStr(covName, "goes")) ? "ReferenceableDataset" : "RectifiedDataset";
				ostream << "    <wcs:CoverageSummary>" << endl;
				ostream << "      <wcs:CoverageId>" << covName << "</wcs:CoverageId>" << endl;
				ostream << "      <wcs:CoverageSubtype>"<< covType <<"</wcs:CoverageSubtype>" << endl;
				ostream << "    </wcs:CoverageSummary>" << endl;
			}
		}

		if(!mv_stitchedMosaicCoverage.empty())
		{
			for (unsigned int i = 0; i < mv_stitchedMosaicCoverage.size(); i++)
			{
				string covName = mv_stitchedMosaicCoverage.at(i).m_covName;
				ostream << "    <wcs:CoverageSummary>" << endl;
				ostream << "      <wcs:CoverageId>"<< covName << "</wcs:CoverageId>" << endl;
				ostream << "      <wcs:CoverageSubtype>RectifiedStitchedMosaic</wcs:CoverageSubtype>" << endl;
				ostream << "    </wcs:CoverageSummary>" << endl;
			}
		}

		if(!mv_datasetSeriesCoverage.empty())
		{
			for (unsigned int i = 0; i < mv_datasetSeriesCoverage.size(); i++)
			{
				string covName = mv_datasetSeriesCoverage.at(i).m_covName;

				ostream << "    <wcseo:DatasetSeriesSummary>" << endl;
				ostream << "      <wcseo:DatasetSeriesId>" << covName << "</wcseo:DatasetSeriesId>" << endl;
				ostream << "      <ows:WGS84BoundingBox>" << endl;
				ostream << "        <ows:LowerCorner>" << mv_datasetSeriesCoverage.at(i).m_minx <<" "<< mv_datasetSeriesCoverage.at(i).m_miny << "</ows:LowerCorner>" << endl;
				ostream << "        <ows:UpperCorner>" << mv_datasetSeriesCoverage.at(i).m_maxx <<" "<< mv_datasetSeriesCoverage.at(i).m_maxy << "</ows:UpperCorner>" << endl;
				ostream << "      </ows:WGS84BoundingBox>" << endl;
				ostream << "      <gml:TimePeriod gml:id=\"tp_" << covName << "\">" << endl;
				ostream << "        <gml:beginPosition>" << mv_datasetSeriesCoverage.at(i).m_beginTime << "</gml:beginPosition>" << endl;
				ostream << "        <gml:endPosition>" << mv_datasetSeriesCoverage.at(i).m_endTime << "</gml:endPosition>" << endl;
				ostream << "      </gml:TimePeriod>" << endl;
				ostream << "    </wcseo:DatasetSeriesSummary>" << endl;
			}
		}

		if(!mv_CovIDs.empty())
		{
			for (unsigned int i = 0; i < mv_CovIDs.size(); i++)
			{
				string covName = mv_CovIDs.at(i);
				string covType = (Find_Compare_SubStr(covName, "MOD05_L2") || Find_Compare_SubStr(covName, "OMI-Aura_L2-") || Find_Compare_SubStr(covName, "goes")) ? "ReferenceableDataset" : "RectifiedDataset";
				ostream << "    <wcs:CoverageSummary>" << endl;
				ostream << "      <wcs:CoverageId>" << covName << "</wcs:CoverageId>" << endl;
				ostream << "      <wcs:CoverageSubtype>"<< covType <<"</wcs:CoverageSubtype>" << endl;
				ostream << "    </wcs:CoverageSummary>" << endl;
			}
		}

		ostream << "  </wcs:Contents>" << endl;
	}
}

/************************************************************************/
/*                           WCST_Respond()                             */
/************************************************************************/

/**
 * \brief The enter point for WCS_GetCapabilities class (Command line environment).
 *
 * This method is the enter point for WCS_GetCapabilities class, which
 * is used under command line environment. In order to debug WCS and use
 * WCS as a command line, the user could issue a request under command line.
 * The response information will be stored in the specified file.
 *
 * @param sOutFileName The path of the response file.
  */

void WCS_GetCapabilities::WCST_Respond(string& sOutFileName)
{
	if (sOutFileName.empty() || EQUAL(sOutFileName.c_str(), ""))
		sOutFileName = MakeTempFile(mp_Conf->Get_TEMPORARY_OUTPUT_DIRECTORY(), "", ".xml");

	ostringstream osstrm;

	CreateCapabilitiesXMLTree(osstrm);

	ofstream outStream(sOutFileName.c_str());
	outStream << osstrm.str() << endl;
	outStream.close();

	return;
}

/************************************************************************/
/*                           WCST_Respond()                             */
/************************************************************************/

/**
 * \brief The enter point for WCS_GetCapabilities class (CGI environment).
 *
 * This method is the enter point for WCS_GetCapabilities class, which
 * is used under CGI environment. The response information will be displayed
 * in the browser.
 */

void WCS_GetCapabilities::WCST_Respond()
{
	ostringstream osstrm;

	osstrm <<"Content-Type: text/xml"<<endl<<endl;
	//osstrm << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"<< endl;
	osstrm << "<?xml version=\"1.0\"?>"<< endl;

	CreateCapabilitiesXMLTree(osstrm);

	cout << osstrm.str()<<endl;

	return;
}
