/******************************************************************************
 * $Id: TRMM_Dataset.cpp 2011-07-19 16:24:00Z $
 *
 * Project:  The Open Geospatial Consortium (OGC) Web Coverage Service (WCS)
 * 			 for Earth Observation: Open Source Reference Implementation
 * Purpose:  TRMM_Dataset implementation for TRMM data
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

#include "TRMM_Dataset.h"

TRMM_Dataset::TRMM_Dataset() {
}

/************************************************************************/
/*                           ~TRMM_Dataset()                            */
/************************************************************************/

/**
 * \brief Destroy an open TRMM_Dataset object.
 *
 * This is the accepted method of closing a TRMM_Dataset dataset and
 * deallocating all resources associated with it.
 */

TRMM_Dataset::~TRMM_Dataset() {
}

/************************************************************************/
/*                            TRMM_Dataset()                            */
/************************************************************************/

/**
 * \brief Create an TRMM_Dataset object.
 *
 * This is the accepted method of creating a TRMM_Dataset object and
 * allocating all resources associated with it.
 *
 * @param id The coverage identifier.
 *
 * @param rBandList The field list selected for this coverage. For TRMM
 * daily data, the user could specify multiple days range in request.
 * Each day is seemed as one field.
 *
 * @return A TRMM_Dataset object.
 */

TRMM_Dataset::TRMM_Dataset(const string& id, vector<int> &rBandList) :
	AbstractDataset(id, rBandList)
{
	md_MissingValue = -9999.9;
}

/************************************************************************/
/*                           InitialDataset()                           */
/************************************************************************/

/**
 * \brief Initialize the TRMM dataset .

 * This method is the implementation for initializing a TRMM dataset.
 * Within this method, SetNativeCRS(), SetGeoTransform() and SetGDALDataset()
 * will be called to initialize an TRMM dataset.
 *
 * @param isSimple the WCS request type.  When user executing a DescribeCoverage
 * request, isSimple is set to 1, and for GetCoverage, is set to 0.
 *
 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr TRMM_Dataset::InitialDataset(const int isSimple)
{
	ms_CoverageSubType = "RectifiedDataset";

	vector<string> strSet;
	unsigned int n = CsvburstCpp(ms_CoverageID, strSet, ':');

	if (n != 3)
	{
		SetWCS_ErrorLocator("TRMM_Dataset::initialDataset()");
		WCS_Error(CE_Failure, OGC_WCS_NoSuchCoverage, "Coverage ID Error.");
		return CE_Failure;
	}

	ms_SrcFilename = strSet[1];
	ms_DataTypeName = "TRMM";
	ms_DatasetName = strSet[2];

	m_bDaily = EQUAL(ms_DatasetName.c_str(), "Daily") ? TRUE : FALSE;

	GDALDataset* pSrc = (GDALDataset*) GDALOpen(ms_SrcFilename.c_str(), GA_ReadOnly);
	if (pSrc == NULL)
	{
		SetWCS_ErrorLocator("TRMM_Dataset::initialDataset()");
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
 * \brief Set the Native CRS for a TRMM dataset.
 *
 * The method will set the CRS for a TRMM dataset as an native CRS.
 *
 * The native CRS for TRMM is assigned to EPSG:4326.
 *
 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr TRMM_Dataset::SetNativeCRS()
{
	mo_NativeCRS.SetWellKnownGeogCS("WGS84");
	return CE_None;
}

/************************************************************************/
/*                           SetGeoTransform()                          */
/************************************************************************/

/**
 * \brief Set the affine GeoTransform matrix for a TRMM coverage.
 *
 * The method will set a GeoTransform matrix for a TRMM coverage
 * by the pre-knowledge of its extent and size.
 *
 * The CRS for the bounding box is EPSG:4326.
 *
 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr TRMM_Dataset::SetGeoTransform()
{
	md_Geotransform[0] = -180;
	md_Geotransform[1] = 0.25;
	md_Geotransform[2] = 0;
	md_Geotransform[3] = 50;
	md_Geotransform[4] = 0;
	md_Geotransform[5] = -0.25;

	mb_GeoTransformSet = TRUE;

    return CE_None;
}

/************************************************************************/
/*                           SetGDALDataset()                           */
/************************************************************************/

/**
 * \brief Set the GDALDataset object to TRMM dataset.
 *
 * This method is used to set the TRMM dataset based on GDAL
 * class VRTDataset.
 *
 * @param isSimple the WCS request type.  When user executing a DescribeCoverage
 * request, isSimple is set to 1, and for GetCoverage, is set to 0.
 *
 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr TRMM_Dataset::SetGDALDataset(const int isSimple)
{
	int nXSize = 1440;
	int nYSize = 400;
	int mBands = mv_BandList.size();
	int sBands = maptrDS.get()->GetRasterCount();
	int tBands = (mBands == 0) ? sBands : mBands;
	if(mBands == 0)
	{
		for(int i = 1; i <= sBands; i++)
			mv_BandList.push_back(i);
	}

	GDALDataType eDT = maptrDS.get()->GetRasterBand(1)->GetRasterDataType();

	int bufSize = nXSize * nYSize * GDALGetDataTypeSize(eDT) / 8;
	char *pData = (char *) CPLMalloc(bufSize);

	char *psGeoSRS = NULL;
	mo_NativeCRS.exportToWkt(&psGeoSRS);

	GDALDriverH hDriver = GDALGetDriverByName("MEM");
	GDALDataset* hSubDS = (GDALDataset*)GDALCreate( hDriver, "", nXSize, nYSize, tBands, eDT, NULL );
	hSubDS->SetProjection(psGeoSRS);
	hSubDS->SetGeoTransform(md_Geotransform);

	if(!isSimple)
	{
		if(mBands == 0)
		{
			for(int i = 1; i <= sBands; i++)
			{
				int curBand = mv_BandList.at(i-1);
				maptrDS.get()->GetRasterBand(curBand)->RasterIO(GF_Read, 0, 0, nXSize, nYSize, pData, nXSize, nYSize, eDT, 0, 0);
				hSubDS->GetRasterBand(i)->SetNoDataValue(md_MissingValue);
				hSubDS->GetRasterBand(i)->RasterIO(GF_Write, 0, 0, nXSize, nYSize, pData, nXSize, nYSize, eDT, 0, 0);
			}
		}
		else
		{
			for(int i = 1; i <= mBands; i++)
			{
				int curBand = mv_BandList.at(i-1);
				maptrDS.get()->GetRasterBand(curBand)->RasterIO(GF_Read, 0, 0, nXSize, nYSize, pData, nXSize, nYSize, eDT, 0, 0);
				hSubDS->GetRasterBand(i)->SetNoDataValue(md_MissingValue);
				hSubDS->GetRasterBand(i)->RasterIO(GF_Write, 0, 0, nXSize, nYSize, pData, nXSize, nYSize, eDT, 0, 0);
			}
		}
	}

	VSIFree((char*) pData);
	VSIFree((char*) psGeoSRS);

	GDALClose(maptrDS.release());
	maptrDS.reset(hSubDS);

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

CPLErr TRMM_Dataset::SetMetaDataList(GDALDataset* hSrcDS)
{
	mv_MeteDataList.push_back("Product_Description=The data was created by GMU WCS from TRMM daily data.");
	mv_MeteDataList.push_back("unit=mm");//could be hard code
	mv_MeteDataList.push_back("FillValue=-9999.9");//could be hard code
	ms_FieldQuantityDef = "mm";
	ms_AllowRanges = "0 5000";
	ms_CoveragePlatform = "TRMM";
	ms_CoverageInstrument = "Precipitation Radar";
	ms_CoverageSensor = "Precipitation Radar";

	return CE_None;
}
