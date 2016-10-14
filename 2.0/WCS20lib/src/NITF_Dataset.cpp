/******************************************************************************
 * $Id: NITF_Dataset.cpp 2011-07-19 16:24:00Z $
 *
 * Project:  The Open Geospatial Consortium (OGC) Web Coverage Service (WCS)
 * 			 for Earth Observation: Open Source Reference Implementation
 * Purpose:  NITF_Dataset implementation for NITF data
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

#include <iomanip>
#include "NITF_Dataset.h"

NITF_Dataset::NITF_Dataset() {
}

/************************************************************************/
/*                           ~NITF_Dataset()                            */
/************************************************************************/

/**
 * \brief Destroy an open NITF_Dataset object.
 *
 * This is the accepted method of closing a NITF_Dataset dataset and
 * deallocating all resources associated with it.
 */

NITF_Dataset::~NITF_Dataset() {
}

/************************************************************************/
/*                            NITF_Dataset()                            */
/************************************************************************/

/**
 * \brief Create an NITF_Dataset object.
 *
 * This is the accepted method of creating a NITF_Dataset object and
 * allocating all resources associated with it.
 *
 * @param id The coverage identifier.
 *
 * @param rBandList The field list selected for this coverage. For NITF
 * daily data, the user could specify multiple days range in request.
 * Each day is seemed as one field.
 *
 * @return A NITF_Dataset object.
 */

NITF_Dataset::NITF_Dataset(const string& id, vector<int> &rBandList) :
	AbstractDataset(id, rBandList)
{
	md_MissingValue = 0;
}

/************************************************************************/
/*                           InitialDataset()                           */
/************************************************************************/

/**
 * \brief Initialize the NITF dataset .

 * This method is the implementation for initializing a NITF dataset.
 * Within this method, SetNativeCRS(), SetGeoTransform() and SetGDALDataset()
 * will be called to initialize an NITF dataset.
 *
 * @param isSimple the WCS request type.  When user executing a DescribeCoverage
 * request, isSimple is set to 1, and for GetCoverage, is set to 0.
 *
 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr NITF_Dataset::InitialDataset(const int isSimple)
{
	ms_CoverageSubType = "RectifiedDataset";

	vector<string> strSet;
	unsigned int n = CsvburstCpp(ms_CoverageID, strSet, ':');

	if (n != 3)
	{
		SetWCS_ErrorLocator("NITF_Dataset::initialDataset()");
		WCS_Error(CE_Failure, OGC_WCS_NoSuchCoverage, "Coverage ID Error.");
		return CE_Failure;
	}

	ms_SrcFilename = StrTrims(strSet[1], "\"");
	ms_DataTypeName = "NITF";
	ms_DatasetName = strSet[2];

	GDALDataset* pSrc = (GDALDataset*) GDALOpen(ms_SrcFilename.c_str(), GA_ReadOnly);
	if (pSrc == NULL)
	{
		SetWCS_ErrorLocator("NITF_Dataset::initialDataset()");
		WCS_Error(CE_Failure, OGC_WCS_NoApplicableCode, "Failed to open file \"%s\".", ms_SrcFilename.c_str());
		return CE_Failure;
	}

	////fetch data format
	ms_NativeFormat = GDALGetDriverShortName(pSrc->GetDriver());

	//set meta data list
	SetMetaDataList(pSrc);

	//fetch raster band count
	unsigned int nBandCount = pSrc->GetRasterCount();
	if (nBandCount < 1)
	{
		GDALClose(pSrc);
		SetWCS_ErrorLocator("HE4_GRID_Dataset::initialDataset()");
		WCS_Error(CE_Failure, OGC_WCS_NoApplicableCode, "Failed to get raster band for coverage.");
		return CE_Failure;
	}

	maptrDS.reset(pSrc);

	if (CE_None != SetNativeCRS() ||
		CE_None != SetGeoTransform() ||
		CE_None != SetGDALDataset(isSimple))
	{
		GDALClose(maptrDS.release());
		return CE_Failure;
	}

	return CE_None;
}

/************************************************************************/
/*                            SetNativeCRS()                            */
/************************************************************************/

/**
 * \brief Set the Native CRS for a NITF dataset.
 *
 * The method will set the CRS for a NITF dataset as an native CRS.
 *
 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr NITF_Dataset::SetNativeCRS()
{
	if(CE_None==AbstractDataset::SetNativeCRS())
		return CE_None;
	return CE_Failure;
}

/************************************************************************/
/*                           SetGeoTransform()                          */
/************************************************************************/

/**
 * \brief Set the affine GeoTransform matrix for a NITF coverage.
 *
 * The method will set a GeoTransform matrix for a NITF coverage
 * by the pre-knowledge of its extent and size.
 *
 * The CRS for the bounding box is EPSG:4326.
 *
 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr NITF_Dataset::SetGeoTransform()
{
	if (CE_None == AbstractDataset::SetGeoTransform())
		return CE_None;
	return CE_Failure;
}

/************************************************************************/
/*                           SetGDALDataset()                           */
/************************************************************************/

/**
 * \brief Set the GDALDataset object to NITF dataset.
 *
 * This method is used to set the NITF dataset based on GDAL
 * class VRTDataset.
 *
 * @param isSimple the WCS request type.  When user executing a DescribeCoverage
 * request, isSimple is set to 1, and for GetCoverage, is set to 0.
 *
 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr NITF_Dataset::SetGDALDataset(const int isSimple)
{
	return CE_None;
}

/************************************************************************/
/*                        SetMetaDataList()                             */
/************************************************************************/

/**
 * \brief Set the metadata list for this coverage.
 *
 * The method will set the metadata list for the coverage based on its
 * corresponding GDALDataset object.
 *
 * @param hSrc the GDALDataset object corresponding to coverage.
 *
 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr NITF_Dataset::SetMetaDataList(GDALDataset* hSrcDS)
{
	mv_MeteDataList.push_back("Product_Description=The data was created by WCS from NITF data.");

	string nullstr = "";
	string header = "<headers>";
	string subheader = "<subheaders>";

	char **papszMetadataHeader = hSrcDS->GetMetadata();
	int metadataCountHeader = CSLCount((char**) papszMetadataHeader);
	for (int i = 0; i < metadataCountHeader; ++i)
	{
		string curMetadata = papszMetadataHeader[i];//NITF_ABPP=08
		vector<string> strVecTemp;
		CsvburstCpp(curMetadata, strVecTemp, '=');
		char* key = (char*)StrReplace(strVecTemp.at(0), "NITF_", "").c_str();
		char* value = (char*)strVecTemp.at(1).c_str();

		if(EQUAL(key, "FHDR") || EQUAL(key, "FVER") || EQUAL(key, "CLEVEL") || EQUAL(key, "STYPE") || EQUAL(key, "OSTAID") || EQUAL(key, "FDT") ||
				EQUAL(key, "FTITLE") || EQUAL(key, "FSCLAS") || EQUAL(key, "FSCLSY") || EQUAL(key, "FSCODE") || EQUAL(key, "FSCTLH") || EQUAL(key, "FSREL") ||
				EQUAL(key, "FSDCTP") || EQUAL(key, "FSDCDT") || EQUAL(key, "FSDCXM") || EQUAL(key, "FSDG") || EQUAL(key, "FSDGDT") || EQUAL(key, "FSCLTX") ||
				EQUAL(key, "FSCATP") || EQUAL(key, "FSCAUT") || EQUAL(key, "FSCRSN") || EQUAL(key, "FSSRDT") || EQUAL(key, "FSCTLN") || EQUAL(key, "FSCOP") ||
				EQUAL(key, "FSCPYS") || EQUAL(key, "ENCRYP") || EQUAL(key, "FBKGC") || EQUAL(key, "ONAME") || EQUAL(key, "OPHONE") || EQUAL(key, "FL") ||
				EQUAL(key, "HL") || EQUAL(key, "NUMI") || EQUAL(key, "LISHn") || EQUAL(key, "LIn") || EQUAL(key, "NUMS") || EQUAL(key, "LSSHn") ||
				EQUAL(key, "LSn") || EQUAL(key, "NUMX") || EQUAL(key, "NUMT") || EQUAL(key, "LTSHn") || EQUAL(key, "LTn") || EQUAL(key, "NUMDES") ||
				EQUAL(key, "LDSHn") || EQUAL(key, "LDn") || EQUAL(key, "NUMRES") || EQUAL(key, "LRESHn") || EQUAL(key, "LREn") || EQUAL(key, "UDHDL") ||
				EQUAL(key, "UDHOFL") || EQUAL(key, "UDHD") || EQUAL(key, "XHDL") || EQUAL(key, "XHDLOFL") || EQUAL(key, "XHD") )
		{
			if(EQUAL(key, "FBKGC")) 
			{
				//0,127,0
				vector<string> colorVec;
				int colorunit = CsvburstCpp(value, colorVec, ',');
				if(colorVec.size() == 3) 
				{
					stringstream ss;
					ss << std::setfill ('0') << std::setw(2) << std::hex << uppercase << atoi((char*)colorVec[0].c_str()) 
					<< std::setfill ('0') << std::setw(2) << std::hex << uppercase << atoi((char*)colorVec[1].c_str()) 
					<< std::setfill ('0') << std::setw(2) << std::hex << uppercase << atoi((char*)colorVec[2].c_str());
					header += nullstr + "<field name=\"" + key + "\" value=\"" + ss.str() + "\" />";
				}
				else
					header += nullstr + "<field name=\"" + key + "\" value=\"" + value + "\" />";
			}
			else
				header += nullstr + "<field name=\"" + key + "\" value=\"" + value + "\" />";
		}
		else
		{
			subheader += nullstr + "<field name=\"" + key + "\" value=\"" + value + "\" />";
		}

	}

	header += "</headers>";
	subheader += "</subheaders>";


	char **papszMetadata = hSrcDS->GetMetadata("xml:TRE");
	int metadataCount = CSLCount((char**) papszMetadata);
	for (int i = 0; i < metadataCount; ++i)
	{
		string all = header + subheader + papszMetadata[i];
		mv_MeteDataList.push_back(all);
	}

	return CE_None;
}

