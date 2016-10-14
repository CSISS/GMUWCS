/******************************************************************************
 * $Id: WCS_T.cpp 2011-07-19 16:24:00Z $
 *
 * Project:  The Open Geospatial Consortium (OGC) Web Coverage Service (WCS)
 * 			 for Earth Observation: Open Source Reference Implementation
 * Purpose:  WCS_T class implementation
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

#include "WCS_T.h"
#include "WCS_GetCapabilities.h"
#include "WCS_DescribeCoverage.h"
#include "WCS_GetCoverage.h"
#include "wcstdsinc.h"


/************************************************************************/
/* ==================================================================== */
/*                                   WCS_T                              */
/* ==================================================================== */
/************************************************************************/

/**
 * \class WCS_T "WCS_T.h"
 *
 * This class is the upper class for handling WCS request.
 */

WCS_T::WCS_T()
{
}

/************************************************************************/
/*                                   WCS_T()                            */
/************************************************************************/

/**
 * \brief Constructor of a WCS_T object.
 *
 * This is the accepted method of creating an WCS_T object. The path
 * for served dataset, stitched mosaic and datasetSeries will be assigned
 * to member variables through parsing configuration file.
 */

WCS_T::WCS_T(const string& conf):mp_Conf(new WCS_Configure(conf))
{
	mv_datasetCoverage.clear();
	mv_stitchedMosaicCoverage.clear();
	mv_datasetSeriesCoverage.clear();
	mv_CovIDs.clear();

	ms_datasetConfPath = mp_Conf->Get_DATASET_CONFIGRATION_FILE_PATH();
	ms_stitchedMosaicConfPath = mp_Conf->Get_STITCHED_MOSAIC_CONFIGRATION_FILE_PATH();
	ms_datasetSeriesConfPath = mp_Conf->Get_DATASET_SERIES_CONFIGRATION_FILE_PATH();
	ms_dataDirectoryPath = mp_Conf->Get_WCS_SERVICE_DATA_DIRECTORY();

	ifstream ifile(mp_Conf->Get_ISO_19115_METADATA_TEMPLATE_PATH().c_str());
	ostringstream out;
	out << ifile.rdbuf();
	ms_iso19115Contents = out.str();
	ifile.close();
}

WCS_T::~WCS_T()
{
	delete mp_Conf;
}

/************************************************************************/
/*                     InitializeConfigurationFiles()                   */
/************************************************************************/

/**
 * \brief Initialize the configuration files and load the contents to memory.
 *
 * This method is used to parse configuration files for dataset/mosaci/datasetSeries
 * coverage, and assign those information to corresponding array.
 *
 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr WCS_T::InitializeConfigurationFiles()
{
	vector<int> oBandList;
	vector<CPLXMLNode *> tmpCovNodes;

	if(!EQUAL(ms_datasetConfPath.c_str(), ""))
	{
		vector<string> dsSet;
		int n = CsvburstCpp(ms_datasetConfPath, dsSet, ',');

		for(int m = 0; m < n; m++)
		{
			string curDSPath = dsSet[m];
			CPLXMLNode* datasetXMLNode  = CPLParseXMLFile((char*)curDSPath.c_str());
			if(NULL == datasetXMLNode)
			{
				SetWCS_ErrorLocator( "WCS_GetCapabilities::initializeConfigurationFiles()");
				WCS_Error(CE_Failure, OGC_WCS_MissingParameterValue, "No Requested Content.");
				return CE_Failure;
			}

			tmpCovNodes = WCSTGetXMLNodeList(datasetXMLNode, "Dataset");

			for (unsigned int i = 0; i < tmpCovNodes.size(); i++)
			{
				DatasetObject curDO;
				curDO.m_covName = CPLGetXMLValue(tmpCovNodes[i], "name", "");
				curDO.m_covPath = CPLGetXMLValue(tmpCovNodes[i], "path", "");
				curDO.m_covGDALID = CPLGetXMLValue(tmpCovNodes[i], "coverageID", "");

				convertFromString(curDO.m_minx, CPLGetXMLValue(tmpCovNodes[i], "west", ""));
				convertFromString(curDO.m_maxx, CPLGetXMLValue(tmpCovNodes[i], "east", ""));
				convertFromString(curDO.m_miny, CPLGetXMLValue(tmpCovNodes[i], "south", ""));
				convertFromString(curDO.m_maxy, CPLGetXMLValue(tmpCovNodes[i], "north", ""));

				curDO.m_beginTime = CPLGetXMLValue(tmpCovNodes[i], "beginTime", "");
				curDO.m_endTime = CPLGetXMLValue(tmpCovNodes[i], "endTime", "");

				mv_datasetCoverage.push_back(curDO);
			}
			tmpCovNodes.clear();
		}
	}


	if(!EQUAL(ms_stitchedMosaicConfPath.c_str(), ""))
	{
		CPLXMLNode* datasetXMLNode  = CPLParseXMLFile((char*)ms_stitchedMosaicConfPath.c_str());
		if(NULL == datasetXMLNode)
		{
			SetWCS_ErrorLocator( "WCS_GetCapabilities::initializeConfigurationFiles()");
			WCS_Error(CE_Failure, OGC_WCS_MissingParameterValue, "No Requested Content.");
			return CE_Failure;
		}

		tmpCovNodes = WCSTGetXMLNodeList(datasetXMLNode, "StitchedMosaic");

		for (unsigned int i = 0; i < tmpCovNodes.size(); i++)
		{
			StitchedMosaicObject curSMO;
			curSMO.m_covName = CPLGetXMLValue(tmpCovNodes[i], "CoverageID", "");

			vector<CPLXMLNode *> tmpDatasetNodes;
			tmpDatasetNodes = WCSTGetXMLNodeList(tmpCovNodes[i], "Datasets.Dataset");
			for (unsigned int j = 0; j < tmpDatasetNodes.size(); j++)
			{
				DatasetObject curDO;
				curDO.m_covName = CPLGetXMLValue(tmpDatasetNodes[j], "name", "");
				curDO.m_covPath = CPLGetXMLValue(tmpDatasetNodes[j], "path", "");
				curSMO.mv_dataset.push_back(curDO);
			}
			mv_stitchedMosaicCoverage.push_back(curSMO);
		}
		tmpCovNodes.clear();
	}


	if(!EQUAL(ms_datasetSeriesConfPath.c_str(), ""))
	{
		vector<string> dssSet;
		int n = CsvburstCpp(ms_datasetSeriesConfPath, dssSet, ',');

		for(int m = 0; m < n; m++)
		{
			string curDSSPath = dssSet[m];
			CPLXMLNode* datasetXMLNode  = CPLParseXMLFile((char*)curDSSPath.c_str());
			if(NULL == datasetXMLNode)
			{
				SetWCS_ErrorLocator( "WCS_GetCapabilities::initializeConfigurationFiles()");
				WCS_Error(CE_Failure, OGC_WCS_MissingParameterValue, "No Requested Content.");
				return CE_Failure;
			}

			tmpCovNodes = WCSTGetXMLNodeList(datasetXMLNode, "DatasetSeries");

			for(unsigned int i = 0; i < tmpCovNodes.size(); i++)
			{
				DatasetSeriesObject curDSO;
				curDSO.m_covName = CPLGetXMLValue(tmpCovNodes[i], "DatasetSeriesId", "");
				curDSO.m_covType = CPLGetXMLValue(tmpCovNodes[i], "CoverageName", "");

				vector<CPLXMLNode *> tmpDatasetNodes;
				tmpDatasetNodes = WCSTGetXMLNodeList(tmpCovNodes[i], "Datasets.Dataset");
				for (unsigned int j = 0; j < tmpDatasetNodes.size(); j++)
				{
					DatasetObject curDO;
					curDO.m_covName = CPLGetXMLValue(tmpDatasetNodes[j], "name", "");
					curDO.m_covPath = CPLGetXMLValue(tmpDatasetNodes[j], "path", "");
					curDO.m_covGDALID = CPLGetXMLValue(tmpDatasetNodes[j], "coverageID", "");

					convertFromString(curDO.m_minx, CPLGetXMLValue(tmpDatasetNodes[j], "west", ""));
					convertFromString(curDO.m_maxx, CPLGetXMLValue(tmpDatasetNodes[j], "east", ""));
					convertFromString(curDO.m_miny, CPLGetXMLValue(tmpDatasetNodes[j], "south", ""));
					convertFromString(curDO.m_maxy, CPLGetXMLValue(tmpDatasetNodes[j], "north", ""));

					if(j == 0)
					{
						curDSO.m_minx = curDO.m_minx;
						curDSO.m_maxx = curDO.m_maxx;
						curDSO.m_miny = curDO.m_miny;
						curDSO.m_maxy = curDO.m_maxy;
					}else
					{
						curDSO.m_minx = MIN(curDO.m_minx, curDSO.m_minx);
						curDSO.m_maxx = MAX(curDO.m_maxx, curDSO.m_maxx);
						curDSO.m_miny = MIN(curDO.m_miny, curDSO.m_miny);
						curDSO.m_maxy = MAX(curDO.m_maxy, curDSO.m_maxy);
					}

					curDO.m_beginTime = CPLGetXMLValue(tmpDatasetNodes[j], "beginTime", "");
					curDO.m_endTime = CPLGetXMLValue(tmpDatasetNodes[j], "endTime", "");
					if(EQUAL(curDSO.m_beginTime.c_str(), ""))
						curDSO.m_beginTime = curDO.m_beginTime;
					if(EQUAL(curDSO.m_endTime.c_str(), ""))
						curDSO.m_endTime = curDO.m_endTime;
					curDSO.m_beginTime = CompareDateTime_GreaterThan(curDSO.m_beginTime, curDO.m_beginTime) >= 0 ? curDO.m_beginTime : curDSO.m_beginTime;
					curDSO.m_endTime = CompareDateTime_GreaterThan(curDSO.m_endTime, curDO.m_endTime) <= 0 ? curDO.m_endTime : curDSO.m_endTime;

					curDSO.mv_dataset.push_back(curDO);
				}//end loop for each data-set

				vector<CPLXMLNode *> tmpStitchNodes;
				tmpStitchNodes = WCSTGetXMLNodeList(tmpCovNodes[i], "StitchedMosaic");
				for (unsigned int j = 0; j < tmpStitchNodes.size(); j++)
				{
					StitchedMosaicObject curSMO;
					curSMO.m_covName = CPLGetXMLValue(tmpStitchNodes[j], "CoverageID", "");

					vector<CPLXMLNode *> tmpDatasetNodes;
					tmpDatasetNodes = WCSTGetXMLNodeList(tmpStitchNodes[j], "Datasets.Dataset");
					for (unsigned int k = 0; k < tmpDatasetNodes.size(); k++)
					{
						DatasetObject curDO;
						curDO.m_covName = CPLGetXMLValue(tmpDatasetNodes[k], "name", "");
						curDO.m_covPath = CPLGetXMLValue(tmpDatasetNodes[k], "path", "");
						curSMO.mv_dataset.push_back(curDO);
					}

					curDSO.mv_stitchedMosaic.push_back(curSMO);
				}
				mv_datasetSeriesCoverage.push_back(curDSO);
			}//end for loop for each data-set series file path

			tmpCovNodes.clear();
		}//end for loop for multiple data-set series file path
	}//User specified data-set series file path


	if(!EQUAL(ms_dataDirectoryPath.c_str(), ""))
	{
		vector<string> datasetList;
		vector<string> CovIDList;
		vector<string> tmplist;
		int n = CsvburstCpp(ms_dataDirectoryPath, datasetList, ',');
		for(int i = 0; i < n; i++)
		{
			if (CE_None != GetFileNameList((char*) datasetList[i].c_str(), tmplist))
			{
				return CE_Failure;
			}

			CovIDList = WCSTRegisterCoverageID(tmplist);

			for (unsigned int j = 0; j < CovIDList.size(); j++)
			{
				mv_CovIDs.push_back(CovIDList[j]);
			}
			tmplist.clear();
		}

	}

	return CE_None;
}

vector<string> WCSTRegisterCoverageID(vector<string>& fileList)
{
	string sName;
	vector<string> covIDs;

	for (unsigned int i = 0; i < fileList.size(); ++i)
	{
		GDALDataset* hSrcDS = (GDALDataset*) GDALOpen((char*) fileList[i].c_str(), GA_ReadOnly);
		if(hSrcDS == NULL)
		{
			continue;
		}

		const char* pchrNativeFormat = GDALGetDriverShortName(hSrcDS->GetDriver());

		if (EQUAL(pchrNativeFormat,"GTIFF"))
		{
			sName = "GEOTIFF:\"" + fileList[i] + "\":Band";
			covIDs.push_back(sName);

		}
		else if (EQUAL(pchrNativeFormat,"NITF"))
		{
			sName = "NITF:\"" + fileList[i] + "\":Data";
			covIDs.push_back(sName);
		}

		GDALClose(hSrcDS);
	}
	return covIDs;
}

/************************************************************************/
/*                     InitializeDatasetSeriesByID()                    */
/************************************************************************/

/**
 * \brief Initialize the dataset series by specify the coverage identifier.
 *
 * This method is used to initialize the dataset series by specify the
 * coverage identifier.
 *
 * @param sCovID Coverage indeifier.
 *
 * @return The corresponding DatasetSeries object.
 */

DatasetSeriesObject WCS_T::InitializeDatasetSeriesByID(string& sCovID)
{
	vector<int> oBandList;
	DatasetSeriesObject dssObj;

	if(!EQUAL(ms_datasetSeriesConfPath.c_str(), ""))
	{
		vector<string> dssSet;
		int n = CsvburstCpp(ms_datasetSeriesConfPath, dssSet, ',');

		for(int m = 0; m < n; m++)
		{
			string curDSSPath = dssSet[m];
			CPLXMLNode* datasetXMLNode  = CPLParseXMLFile((char*)curDSSPath.c_str());
			if(NULL == datasetXMLNode)
				return dssObj;

			vector<CPLXMLNode *> tmpCovNodes = WCSTGetXMLNodeList(datasetXMLNode, "DatasetSeries");

			for(unsigned int i = 0; i < tmpCovNodes.size(); i++)
			{
				DatasetSeriesObject curDSO;
				curDSO.m_covName = CPLGetXMLValue(tmpCovNodes[i], "DatasetSeriesId", "");
				curDSO.m_covType = CPLGetXMLValue(tmpCovNodes[i], "CoverageName", "");

				if(EQUAL(curDSO.m_covName.c_str(), sCovID.c_str()))//Found it!!!
				{
					vector<CPLXMLNode *> tmpDatasetNodes;
					tmpDatasetNodes = WCSTGetXMLNodeList(tmpCovNodes[i], "Datasets.Dataset");
					for (unsigned int j = 0; j < tmpDatasetNodes.size(); j++)
					{
						DatasetObject curDO;
						curDO.m_covName = CPLGetXMLValue(tmpDatasetNodes[j], "name", "");
						curDO.m_covPath = CPLGetXMLValue(tmpDatasetNodes[j], "path", "");
						curDO.m_covGDALID = CPLGetXMLValue(tmpDatasetNodes[j], "coverageID", "");

						convertFromString(curDO.m_minx, CPLGetXMLValue(tmpDatasetNodes[j], "west", ""));
						convertFromString(curDO.m_maxx, CPLGetXMLValue(tmpDatasetNodes[j], "east", ""));
						convertFromString(curDO.m_miny, CPLGetXMLValue(tmpDatasetNodes[j], "south", ""));
						convertFromString(curDO.m_maxy, CPLGetXMLValue(tmpDatasetNodes[j], "north", ""));

						curDO.m_beginTime = CPLGetXMLValue(tmpDatasetNodes[j], "beginTime", "");
						curDO.m_endTime = CPLGetXMLValue(tmpDatasetNodes[j], "endTime", "");

						if(j == 0)
						{
							curDSO.m_minx = curDO.m_minx;
							curDSO.m_maxx = curDO.m_maxx;
							curDSO.m_miny = curDO.m_miny;
							curDSO.m_maxy = curDO.m_maxy;
							curDSO.m_beginTime = curDO.m_beginTime;
							curDSO.m_endTime = curDO.m_endTime;
						}else
						{
							curDSO.m_minx = MIN(curDO.m_minx, curDSO.m_minx);
							curDSO.m_maxx = MAX(curDO.m_maxx, curDSO.m_maxx);
							curDSO.m_miny = MIN(curDO.m_miny, curDSO.m_miny);
							curDSO.m_maxy = MAX(curDO.m_maxy, curDSO.m_maxy);
							curDSO.m_beginTime = CompareDateTime_GreaterThan(curDSO.m_beginTime, curDO.m_beginTime) >= 0 ? curDO.m_beginTime : curDSO.m_beginTime;
							curDSO.m_endTime = CompareDateTime_GreaterThan(curDSO.m_endTime, curDO.m_endTime) <= 0 ? curDO.m_endTime : curDSO.m_endTime;
						}
						curDSO.mv_dataset.push_back(curDO);
					}
					tmpDatasetNodes.clear();

					dssObj = curDSO;
					break;
				}
			}
			tmpCovNodes.clear();
		}
	}

	return dssObj;
}

/************************************************************************/
/*                        InitializeDatasetByID()                       */
/************************************************************************/

/**
 * \brief Initialize the dataset by specify the coverage identifier.
 *
 * This method is used to initialize the dataset by specify the
 * coverage identifier.
 *
 * @param sCovID Coverage indeifier.
 *
 * @return The corresponding Dataset object.
 */

DatasetObject WCS_T::InitializeDatasetByID(string& sCovID)
{
	vector<int> oBandList;
	DatasetObject dsObj;

	if(!EQUAL(ms_datasetConfPath.c_str(), ""))
	{
		vector<string> dsSet;
		int n = CsvburstCpp(ms_datasetConfPath, dsSet, ',');
		for(int m = 0; m < n; m++)
		{
			string curDSPath = dsSet[m];
			CPLXMLNode* datasetXMLNode  = CPLParseXMLFile((char*)curDSPath.c_str());
			vector<CPLXMLNode *> tmpDatasetNodes = WCSTGetXMLNodeList(datasetXMLNode, "Dataset");
			for(unsigned int j = 0; j < tmpDatasetNodes.size(); j++)
			{
				DatasetObject curDO;
				curDO.m_covName = CPLGetXMLValue(tmpDatasetNodes[j], "name", "");
				curDO.m_covPath = CPLGetXMLValue(tmpDatasetNodes[j], "path", "");

				if(EQUAL(curDO.m_covName.c_str(), sCovID.c_str()))//Found it!!!
				{
					curDO.m_covGDALID = CPLGetXMLValue(tmpDatasetNodes[j], "coverageID", "");

					convertFromString(curDO.m_minx, CPLGetXMLValue(tmpDatasetNodes[j], "west", ""));
					convertFromString(curDO.m_maxx, CPLGetXMLValue(tmpDatasetNodes[j], "east", ""));
					convertFromString(curDO.m_miny, CPLGetXMLValue(tmpDatasetNodes[j], "south", ""));
					convertFromString(curDO.m_maxy, CPLGetXMLValue(tmpDatasetNodes[j], "north", ""));

					curDO.m_beginTime = CPLGetXMLValue(tmpDatasetNodes[j], "beginTime", "");
					curDO.m_endTime = CPLGetXMLValue(tmpDatasetNodes[j], "endTime", "");

					return curDO;
				}//found the dataset
			}
		}
	}

	if(!EQUAL(ms_datasetSeriesConfPath.c_str(), ""))
	{
		vector<string> dssSet;
		int n = CsvburstCpp(ms_datasetSeriesConfPath, dssSet, ',');

		for(int m = 0; m < n; m++)
		{
			string curDSSPath = dssSet[m];
			CPLXMLNode* datasetXMLNode  = CPLParseXMLFile((char*)curDSSPath.c_str());
			if(NULL == datasetXMLNode)
				return dsObj;

			vector<CPLXMLNode *> tmpCovNodes = WCSTGetXMLNodeList(datasetXMLNode, "DatasetSeries");

			for(unsigned int i = 0; i < tmpCovNodes.size(); i++)
			{
				DatasetSeriesObject curDSO;
				curDSO.m_covName = CPLGetXMLValue(tmpCovNodes[i], "DatasetSeriesId", "");
				curDSO.m_covType = CPLGetXMLValue(tmpCovNodes[i], "CoverageName", "");

				vector<CPLXMLNode *> tmpDatasetNodes;
				tmpDatasetNodes = WCSTGetXMLNodeList(tmpCovNodes[i], "Datasets.Dataset");
				for (unsigned int j = 0; j < tmpDatasetNodes.size(); j++)
				{
					DatasetObject curDO;
					curDO.m_covName = CPLGetXMLValue(tmpDatasetNodes[j], "name", "");
					curDO.m_covPath = CPLGetXMLValue(tmpDatasetNodes[j], "path", "");

					if(EQUAL(curDO.m_covName.c_str(), sCovID.c_str()))//Found it!!!
					{
						curDO.m_covGDALID = CPLGetXMLValue(tmpDatasetNodes[j], "coverageID", "");

						convertFromString(curDO.m_minx, CPLGetXMLValue(tmpDatasetNodes[j], "west", ""));
						convertFromString(curDO.m_maxx, CPLGetXMLValue(tmpDatasetNodes[j], "east", ""));
						convertFromString(curDO.m_miny, CPLGetXMLValue(tmpDatasetNodes[j], "south", ""));
						convertFromString(curDO.m_maxy, CPLGetXMLValue(tmpDatasetNodes[j], "north", ""));

						curDO.m_beginTime = CPLGetXMLValue(tmpDatasetNodes[j], "beginTime", "");
						curDO.m_endTime = CPLGetXMLValue(tmpDatasetNodes[j], "endTime", "");

						return curDO;
					}//found the dataset
				}//end for each dataset in one dataset series
			}//end for each dataset series
		}//end for each dataset series configuration file
	}//end for looking from dataset series set

	return dsObj;
}

/************************************************************************/
/*                              SendHttpHead()                          */
/************************************************************************/

/**
 * \brief Sent the HTTP head type according to request type.
 *
 * This method is used to send the HTTP head type according to request type.
 * As to SOAP request:
 * @code
 * Content-Type: application/xml+soap
 * @endcode
 *
 * As to POST request:
 * @code
 * Content-Type: text/xml
 * @endcode
 */

void WCS_T::SendHttpHead()
{
	if (mb_SoapRequest)
		cout<<"Content-Type: application/xml+soap"<<endl<<endl;
	else
		cout<<"Content-Type: text/xml"<<endl<<endl;
}

/************************************************************************/
/*                        WCSTOpenFromXMLString()                       */
/************************************************************************/

/**
 * \brief Initialize the WCS_T object from XML string.
 *
 * This method is used to initialize the WCS_T object from XML string.
 *
 * @param xmlStr The string contains XML contents from HTTP POST method.
 *
 * @param cnfNm The path of WCS configuration file.
 *
 * @return The WCS_T object, has been Initialized by concert sub class.
 */

WCS_T* WCSTOpenFromXMLString(const string& xmlStr, const string& cnfNm)
{
    WCS_T* wcst;

    string::size_type beginIdx, endIdx;
    beginIdx = xmlStr.find("<?");
    endIdx = xmlStr.rfind(">");

    if (beginIdx == string::npos && endIdx == string::npos)
    {
        wcst = WCSTOpenFromURLString(xmlStr, cnfNm);
    }
    else
    {
        CPLXMLNode *xmlRoot = CPLParseXMLString(xmlStr.c_str());
        if (xmlRoot == NULL)
        	return NULL;
        wcst = WCSTCreateFromXMLTree(xmlRoot, cnfNm);

        CPLDestroyXMLNode(xmlRoot);
    }

    return wcst;
}

/************************************************************************/
/*                        WCSTOpenFromXMLFile()                         */
/************************************************************************/

/**
 * \brief Initialize the WCS_T object from XML file path.
 *
 * This method is used to initialize the WCS_T object from XML file path.
 *
 * @param xmlStr The path of XML file.
 *
 * @param cnfNm The path of WCS configuration file.
 *
 * @return The WCS_T object, has been Initialized by concert sub class.
 */

WCS_T* WCSTOpenFromXMLFile(const string& fileName, const string& cnfNm)
{
	CPLXMLNode *xmlRoot = CPLParseXMLFile(fileName.c_str());
	if (xmlRoot == NULL)
	{
		SetWCS_ErrorLocator("WCSTOpenFromXMLFile()");
		WCS_Error(CE_Failure, OGC_WCS_MissingParameterValue,
				"Failed to Create XML Tree from file \"%s\".", fileName.c_str());
		return NULL;
	}

	WCS_T* wcst = WCSTCreateFromXMLTree(xmlRoot, cnfNm);
	if (wcst == NULL)
	{
		CPLDestroyXMLNode(xmlRoot);
		return NULL;
	}
	CPLDestroyXMLNode(xmlRoot);

	return wcst;
}

/************************************************************************/
/*                       WCSTOpenFromURLString()                        */
/************************************************************************/

/**
 * \brief Initialize the WCS_T object from URL string.
 *
 * This method is used to initialize the WCS_T object from URL string.
 *
 * @param urlStr The URL string.
 *
 * @param cnfNm The path of WCS configuration file.
 *
 * @return The WCS_T object, has been Initialized by concert sub class.
 */

WCS_T* WCSTOpenFromURLString(string urlStr, const string& cnfNm)
{
	WCST_SetSoapMsgTrns(FALSE);

	WCS_T* wcst = NULL;

	string::size_type pos = urlStr.find_first_of("?");
	if(pos!=string::npos)
		urlStr=urlStr.substr(pos+1);

	KVPsReader rqstParas(urlStr, '&');
	string sVersion = rqstParas.getValue("Version", "");
	string sRequest = rqstParas.getValue("Request", "");
	string sService = rqstParas.getValue("Service", "");

	if (!EQUAL(sService.c_str(),"WCS"))
	{
		SetWCS_ErrorLocator("Service");
		WCS_Error(CE_Failure, OGC_WCS_MissingParameterValue,
				"The \"service\" name should be fixed to \"WCS\".");
		return NULL;
	}

	if (sRequest.empty() || sRequest == "")
	{
		SetWCS_ErrorLocator("Request");
		WCS_Error(CE_Failure, OGC_WCS_MissingParameterValue,
				"Failed to get \"request\" Parameter.");
		return NULL;
	}

	if (! (EQUAL(sVersion.c_str(),"2.0") || EQUAL(sVersion.c_str(),"2.0.0") ))
	{
		SetWCS_ErrorLocator("Version");
		WCS_Error(CE_Failure, OGC_WCS_MissingParameterValue,
				"Requested protocol version should be fixed to \"2.0\".");
		return NULL;
	}

	if (EQUAL(sRequest.c_str(),"GETCAPABILITIES"))
	{
		wcst = new WCS_GetCapabilities(cnfNm);
		if (CE_None != wcst->GetReqMessageFromURLString(urlStr))
		{
			delete wcst;
			return NULL;
		}
	}
	else if (EQUAL(sRequest.c_str(),"DESCRIBECOVERAGE") || EQUAL(sRequest.c_str(),"DESCRIBEEOCOVERAGESET"))
	{
		wcst = new WCS_DescribeCoverage(cnfNm);
		if (CE_None != wcst->GetReqMessageFromURLString(urlStr))
		{
			delete wcst;
			return NULL;
		}
	}
	else if (EQUAL(sRequest.c_str(),"GETCOVERAGE"))
	{
		wcst = new WCS_GetCoverage(cnfNm);
		if (CE_None != wcst->GetReqMessageFromURLString(urlStr))
		{
			delete wcst;
			return NULL;
		}
	}
	else
	{
		SetWCS_ErrorLocator("WCS-GET-KVP");
		WCS_Error(CE_Failure, OGC_WCS_InvalidParameterValue,
				"Passed KVPs could not be parsed based on [OGC 09-147r1] extension.");
		return NULL;
	}

	return wcst;
}

/************************************************************************/
/*                       WCSTCreateFromXMLTree()                        */
/************************************************************************/

/**
 * \brief Initialize the WCS_T object from CPLXMLNode object.
 *
 * This method is used to initialize the WCS_T object from CPLXMLNode object.
 *
 * @param xmlRoot The CPLXMLNode object corresponding to XML file.
 *
 * @param cnfNm The path of WCS configuration file.
 *
 * @return The WCS_T object, has been implemented by its subclass.
 */

WCS_T* WCSTCreateFromXMLTree(CPLXMLNode *xmlRoot, const string& cnfNm)
{
	if (!xmlRoot)
	{
		SetWCS_ErrorLocator("WCSTCreateFromXMLTree()");
		WCS_Error(CE_Failure, OGC_WCS_MissingParameterValue,
				"No XML Root Node");
		return NULL;
	}

	WCS_T* wcst;
	CPLStripXMLNamespace(xmlRoot, NULL, TRUE);

	const char* pChrVersion = NULL;
	const char* pChrService = NULL;
	string pChrRequest = "";
	CPLXMLNode *xmlRqstRoot = NULL;

	xmlRqstRoot = CPLGetXMLNode(xmlRoot, "=Envelop.Body");
	if (xmlRqstRoot)
	{
		xmlRoot = xmlRqstRoot;
		WCST_SetSoapMsgTrns(TRUE);
	}

	if ((xmlRqstRoot = CPLSearchXMLNode(xmlRoot, "=DescribeCoverage")))
	{
		pChrVersion = CPLGetXMLValue(xmlRqstRoot, "Version", "2.0");
		pChrService = CPLGetXMLValue(xmlRqstRoot, "Service", "");
		pChrRequest = "DescribeCoverage";
	}
	else if ((xmlRqstRoot = CPLSearchXMLNode(xmlRoot, "=GetCapabilities")))
	{
		pChrVersion = CPLGetXMLValue(xmlRqstRoot, "Version", "2.0");
		pChrService = CPLGetXMLValue(xmlRqstRoot, "Service", "");
		pChrRequest = "GetCapabilities";
	}
	else if ((xmlRqstRoot = CPLSearchXMLNode(xmlRoot, "=GetCoverage")))
	{
		pChrVersion = CPLGetXMLValue(xmlRqstRoot, "Version", "2.0");
		pChrService = CPLGetXMLValue(xmlRqstRoot, "Service", "");
		pChrRequest = "GetCoverage";
	}
	else if ((xmlRqstRoot = CPLSearchXMLNode(xmlRoot, "=Transaction")))
	{
		pChrVersion = CPLGetXMLValue(xmlRqstRoot, "Version", "1.1.0");//???
		pChrService = CPLGetXMLValue(xmlRqstRoot, "Service", "");
		pChrRequest = "Transaction";
	}
	else
	{
		SetWCS_ErrorLocator("WCSTCreateFromXMLTree()");
		WCS_Error(CE_Failure, OGC_WCS_InvalidParameterValue, "Invalid Request.");
		return NULL;
	}

	if (!EQUAL(pChrService,"WCS"))
	{
		SetWCS_ErrorLocator("Service");
		WCS_Error(CE_Failure, OGC_WCS_MissingParameterValue,
				"The \"service\" name should be fixed to \"WCS\".");
		return NULL;
	}

	if (pChrRequest.empty() || pChrRequest == "")
	{
		SetWCS_ErrorLocator("Request");
		WCS_Error(CE_Failure, OGC_WCS_MissingParameterValue,
				"Failed to get \"request\" Parameter.");
		return NULL;
	}

	if (EQUAL(pChrRequest.c_str(),"GETCAPABILITIES"))
	{
		wcst = new WCS_GetCapabilities(cnfNm);
		if (CE_None != wcst->GetReqMessageFromXMLTree(xmlRqstRoot))
		{
			delete wcst;
			return NULL;
		}
	}
	else if (EQUAL(pChrRequest.c_str(),"DESCRIBECOVERAGE"))
	{
		wcst = new WCS_DescribeCoverage(cnfNm);
		if (CE_None != wcst->GetReqMessageFromXMLTree(xmlRqstRoot))
		{
			delete wcst;
			return NULL;
		}
	}
	else if (EQUAL(pChrRequest.c_str(),"GETCOVERAGE"))
	{
		wcst = new WCS_GetCoverage(cnfNm);
		if (CE_None != wcst->GetReqMessageFromXMLTree(xmlRqstRoot))
		{
			delete wcst;
			return NULL;
		}
	}
	else
	{
		SetWCS_ErrorLocator("WCS-POST-XML");
		WCS_Error(CE_Failure, OGC_WCS_InvalidParameterValue,
				"Posted XML could not be parsed based on [OGC 09-148r1] extension.");
		return NULL;
	}

	return wcst;
}

/************************************************************************/
/*                        WCSTGetXMLValueList()                         */
/************************************************************************/

/**
 * \brief Fetch the value list from CPLXMLNode object.
 *
 * This method is used to fetch the value list from CPLXMLNode object by
 * specifying the element path.
 *
 * @param psRoot The CPLXMLNode object corresponding to XML file.
 *
 * @param pszPath The path of element needs to be fetched.
 *
 * @return The value array.
 */

vector<string> WCSTGetXMLValueList(CPLXMLNode *psRoot, const char* pszPath)
{
	vector<string> valueList;
	CPLXMLNode *psTarget;

	if (pszPath == NULL || *pszPath == '\0')
		psTarget = psRoot;
	else
		psTarget = CPLGetXMLNode(psRoot, pszPath);

	if (psTarget == NULL)
		return valueList;

	if (psTarget->eType == CXT_Element)
	{
		while (psTarget)
		{
			if (psTarget->psChild != NULL && psTarget->psChild->eType == CXT_Text && psTarget->psChild->psNext == NULL)
				valueList.push_back(StrTrim(psTarget->psChild->pszValue));

			psTarget = psTarget->psNext;
		}
	}

	return valueList;
}

/************************************************************************/
/*                        WCSTGetXMLNodeList()                          */
/************************************************************************/

/**
 * \brief Fetch the CPLXMLNode array from the root CPLXMLNode.
 *
 * This method is used to fetch the CPLXMLNode array from the root CPLXMLNode.
 *
 * @param psRoot The CPLXMLNode object corresponding to XML file.
 *
 * @param pszPath The path of element needs to be fetched.
 *
 * @return The CPLXMLNode array.
 */

vector<CPLXMLNode*> WCSTGetXMLNodeList(CPLXMLNode *psRoot, const char* pszPath)
{
	vector<CPLXMLNode*> nodeList;

	if (NULL == psRoot)
		return nodeList;

	const char* psTargetEleName = NULL;

	CPLXMLNode *psTarget = CPLGetXMLNode(psRoot, pszPath);
	if (psTarget && psTarget->eType == CXT_Element)
	{
		psTargetEleName = psTarget->pszValue;
	}
	else
		return nodeList;

	while (psTarget)
	{
		if (psTarget->eType != CXT_Text && EQUAL(psTarget->pszValue,psTargetEleName))
		{
			nodeList.push_back(psTarget);
		}
		psTarget = psTarget->psNext;
	}

	return nodeList;
}

/************************************************************************/
/*                          WCSTCreateDataset()                         */
/************************************************************************/

/**
 * \brief Create a AbstractDataset object by specifying coverage identifier and band list.
 *
 * This method is used to create a AbstractDataset object by specifying
 * coverage identifier and band list.
 *
 * @param covID The coverage identifier.
 *
 * @param oBandList The field list selected for this coverage.
 *
 * @param isSimple the WCS request type.  When user executing a DescribeCoverage
 * request, isSimple is set to 1, and for GetCoverage, is set to 0.
 *
 * @return The AbstractDataset object, have been implemented by it subclass.
 */

AbstractDataset* WCSTCreateDataset(const string& covID, vector<int>& oBandList, const int isSample)
{
	if (EQUAL(covID.c_str(), ""))
	{
		SetWCS_ErrorLocator("WCSTCreateDataset");
		WCS_Error(CE_Failure, OGC_WCS_NoApplicableCode,
				"Coverage identifier is null.");
		return NULL;
	}

	AbstractDataset* absDS = NULL;

	if (EQUALN(covID.c_str(),"HDF4_EOS:EOS_SWATH:",19))
	{
		absDS = new HE4_SWATH_Dataset(covID, oBandList);
	}
	else if (EQUALN(covID.c_str(),"HDF4_EOS:EOS_GRID:",18))
	{
		absDS = new HE4_GRID_Dataset(covID, oBandList);
	}
	else if (EQUALN(covID.c_str(),"TRMM:",5))
	{
		absDS = new TRMM_Dataset(covID, oBandList);
	}
	else if (EQUALN(covID.c_str(),"HDF5:",5) && Find_Compare_SubStr(covID, "HDFEOS/GRIDS"))
	{
		absDS = new HE5_GRID_Dataset(covID, oBandList);
	}
	else if (EQUALN(covID.c_str(),"HDF5:",5) && Find_Compare_SubStr(covID, "HDFEOS/SWATHS"))
	{
		absDS = new HE5_SWATH_Dataset(covID, oBandList);
	}
	else if (EQUALN(covID.c_str(),"GOES:NETCDF",11))
	{
		absDS = new NC_GOES_Dataset(covID, oBandList);
	}
	else if (EQUALN(covID.c_str(),"NITF",4))
	{
		absDS = new NITF_Dataset(covID, oBandList);
	}
	else
	{
		SetWCS_ErrorLocator("WCSTCreateDataset()");
		WCS_Error(CE_Failure, OGC_WCS_NoApplicableCode,
				"The data type of coverage is not supported in GMU EO WCS.");
		return NULL;
	}

	if (NULL == absDS)
	{
		SetWCS_ErrorLocator("WCSTCreateDataset()");
		WCS_Error(CE_Failure, OGC_WCS_NoApplicableCode,
				"Failed to create abstract data-set for coverage.");
		return NULL;
	}

	if (CE_None != absDS->InitialDataset(isSample))
	{
		WCSTDestroyDataset(absDS);
		return NULL;
	}

	return absDS;
}

/************************************************************************/
/*                          WCSTDestroyDataset()                        */
/************************************************************************/

/**
 * \brief Close an AbstractDataset object.
 *
 * This method is used to close an AbstractDataset object.
 *
 * @param absDS The AbstractDataset object.
 */

void WCSTDestroyDataset(AbstractDataset* absDS)
{
	if (absDS)
		delete absDS;

	return;
}

/************************************************************************/
/*                             WCSTClose()                              */
/************************************************************************/

/**
 * \brief Close the WCS_T object.
 *
 * This method is used to close WCS_T object.
 *
 * @param pwcst WCS_T object.
 */

void WCSTClose(WCS_T* pwcst)
{
	if(pwcst != NULL)
		delete pwcst;
	pwcst = NULL;
}
