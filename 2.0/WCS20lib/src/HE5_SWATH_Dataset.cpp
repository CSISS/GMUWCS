/******************************************************************************
 * $Id: HE5_SWATH_Dataset.cpp 2011-07-19 16:24:00Z $
 *
 * Project:  The Open Geospatial Consortium (OGC) Web Coverage Service (WCS)
 * 			 for Earth Observation: Open Source Reference Implementation
 * Purpose:  HE5_SWATH_Dataset implementation for HDF-EOS Swath format
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

#include "HE5_SWATH_Dataset.h"

HE5_SWATH_Dataset::HE5_SWATH_Dataset()
{
}

/************************************************************************/
/*                            HE5_SWATH_Dataset()                       */
/************************************************************************/

/**
 * \brief Create an HDF-EOS Swath dataset object.
 *
 * This is the accepted method of creating a HE5_SWATH_Dataset dataset and
 * allocating all resources associated with it.
 *
 * @param id The coverage identifier.
 *
 * @param rBandList The field list selected for this coverage. For HDF-EOS5
 * Swath data, the user may choose multiple range and execute range subset
 * operation.
 *
 * @return A HE5_SWATH_Dataset object.
 */

HE5_SWATH_Dataset::HE5_SWATH_Dataset(const string& id, vector<int> &rBandList) :
	AbstractDataset(id, rBandList)
{
	md_MissingValue = -9999;
	mb_GeoTransformSet = FALSE;
}

/************************************************************************/
/*                           ~HE5_SWATH_Dataset()                       */
/************************************************************************/

/**
 * \brief Destroy an open HE5_SWATH_Dataset object.
 *
 * This is the accepted method of closing a HE5_SWATH_Dataset dataset and
 * deallocating all resources associated with it.
 */

HE5_SWATH_Dataset::~HE5_SWATH_Dataset()
{
}

/************************************************************************/
/*                           InitialDataset()                           */
/************************************************************************/

/**
 * \brief Initialize the HDF-EOS Swath dataset .

 * This method is the implementation for initializing a HDF-EOS Swath dataset.
 * Within this method, SetNativeCRS(), SetGeoTransform() and SetGDALDataset()
 * will be called to initialize an HDF-EOS Swath dataset.
 * The coverage type of HDF-EOS Swath data is set to "ReferenceableDataset".
 *
 * @param isSimple the WCS request type.  When user executing a DescribeCoverage
 * request, isSimple is set to 1, and for GetCoverage, is set to 0.
 *
 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr HE5_SWATH_Dataset::InitialDataset(const int isSimple)
{
	ms_CoverageSubType = "ReferenceableDataset";

	string tmpStr;
	vector<string> strSet;
	unsigned int n = CsvburstCpp(ms_CoverageID, strSet, ':');

	if (n != 3)
	{
		SetWCS_ErrorLocator("HE5_SWATH_Dataset::initialDataset()");
		WCS_Error(CE_Failure, OGC_WCS_NoSuchCoverage, "Coverage ID Error.");
		return CE_Failure;
	}

	ms_SrcFilename = StrTrims(strSet[1], " \'\"");
	tmpStr = StrTrims(strSet[2], "//");
	strSet.clear();

	n = CsvburstCpp(tmpStr, strSet, '/');
	ms_DataTypeName = strSet[2];
	ms_DatasetName = strSet[4];
	ms_SrcFilename = StrTrims(ms_SrcFilename, " \'\"");
	ms_CoverageID = StrReplace(ms_CoverageID, "\'", "\"");

	if (!isSimple)
		CPLSetConfigOption("GEOL_AS_GCPS", "FULL");
	else
		CPLSetConfigOption("GEOL_AS_GCPS", "PARTIAL");

	GDALDataset* pSrc = (GDALDataset*) GDALOpen(ms_CoverageID.c_str(), GA_ReadOnly);
	if (pSrc == NULL)
	{
		SetWCS_ErrorLocator("HE5_SWATH_Dataset::initialDataset()");
		WCS_Error(CE_Failure, OGC_WCS_NoApplicableCode,
				"Failed to open file \"%s\".", ms_SrcFilename.c_str());
		return CE_Failure;
	}

	////fetch data format
	ms_NativeFormat = GDALGetDriverShortName(pSrc->GetDriver());
	if (!EQUAL(ms_NativeFormat.c_str(),"HDF5Image"))
	{
		GDALClose(pSrc);
		SetWCS_ErrorLocator("HE5_SWATH_Dataset::initialDataset()");
		WCS_Error(CE_Failure, OGC_WCS_NoApplicableCode, "Failed to get data format");
		return CE_Failure;
	}

	//set meta data list
	SetMetaDataList(pSrc);

	//fetch raster band count
	unsigned int nBandCount = pSrc->GetRasterCount();
	if (nBandCount < 1)
	{
		GDALClose(pSrc);
		SetWCS_ErrorLocator("HE5_SWATH_Dataset::initialDataset()");
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
 * \brief Set the Native CRS for a HDF-EOS Swath dataset.
 *
 * The method will set the CRS for a HDF-EOS5 Swath dataset as an native
 * CRS. Since the coordinates recorded in HDF-EOS Swath granule belong
 * to geographic CRS, EPSG:4326 CRS are assigned to HDF-EOS Swath dataset.
 *
 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr HE5_SWATH_Dataset::SetNativeCRS()
{
	if (CE_None == AbstractDataset::SetNativeCRS())
		return CE_None;

	char *psTargetSRS = (char*) maptrDS->GetGCPProjection();
	if (psTargetSRS && *psTargetSRS != '\0')
	{
		if (OGRERR_NONE != mo_NativeCRS.importFromWkt(&psTargetSRS))
			mo_NativeCRS.SetWellKnownGeogCS("WGS84");
	}
	else if (maptrDS->GetGCPCount() > 0)
	{
		mo_NativeCRS.SetWellKnownGeogCS("WGS84");
	}
	else
	{
		SetWCS_ErrorLocator("HE5_SWATH_Dataset::setNativeCRS()");
		WCS_Error(CE_Failure, OGC_WCS_NoApplicableCode, "Failed to generate CRS for coverage.");
		return CE_Failure;
	}

	return CE_None;
}


/************************************************************************/
/*                           SetGeoTransform()                          */
/************************************************************************/

/**
 * \brief Set the affine GeoTransform matrix for a HDF-EOS5 Swath coverage.
 *
 * The method will set a GeoTransform matrix for a HDF-EOS5 Swath coverage
 * by parsing and analyzing the metadata of HDF-EOS5 Swath data granule. If
 * granule metadata do not include bounding box extent, GDAL_GCP will be used
 * to deduce the bounding box and set the GeoTransform accordingly.
 *
 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr HE5_SWATH_Dataset::SetGeoTransform()
{
	if (CE_None == AbstractDataset::SetGeoTransform())
		return CE_None;

	char **papszMetadata = maptrDS->GetMetadata("");
	double resX, resY, resMIN;
	double mdSrcGeoMinX, mdSrcGeoMaxX, mdSrcGeoMinY, mdSrcGeoMaxY;

    if( NULL != CSLFetchNameValue( papszMetadata, "WESTBOUNDINGCOORDINATE" ) &&
    	NULL != CSLFetchNameValue( papszMetadata, "EASTBOUNDINGCOORDINATE" ) &&
    	NULL != CSLFetchNameValue( papszMetadata, "SOUTHBOUNDINGCOORDINATE" ) &&
    	NULL != CSLFetchNameValue( papszMetadata, "NORTHBOUNDINGCOORDINATE" ) )
    {
		mdSrcGeoMinX =atof( CSLFetchNameValue( papszMetadata, "WESTBOUNDINGCOORDINATE" ) );
		mdSrcGeoMaxX =atof( CSLFetchNameValue( papszMetadata, "EASTBOUNDINGCOORDINATE" ) );
		mdSrcGeoMinY =atof( CSLFetchNameValue( papszMetadata, "SOUTHBOUNDINGCOORDINATE" ) );
		mdSrcGeoMaxY =atof( CSLFetchNameValue( papszMetadata, "NORTHBOUNDINGCOORDINATE" ) );

		if(mdSrcGeoMinX > mdSrcGeoMaxX)//Cover IDL
			Exchange(mdSrcGeoMinX, mdSrcGeoMaxX);

		resX = (mdSrcGeoMaxX-mdSrcGeoMinX) / (maptrDS->GetRasterXSize());
		resY = (mdSrcGeoMaxY-mdSrcGeoMinY) / (maptrDS->GetRasterYSize());

		resMIN = MIN(resX, resY);

		md_Geotransform[0] = mdSrcGeoMinX;
		md_Geotransform[1] = resMIN;
		md_Geotransform[2] = 0;
		md_Geotransform[3] = mdSrcGeoMaxY;
		md_Geotransform[4] = 0;
		md_Geotransform[5] = -resMIN;

		mi_RectifiedImageXSize = fabs(mdSrcGeoMaxX - mdSrcGeoMinX) / resMIN + 1;
		mi_RectifiedImageYSize = fabs(mdSrcGeoMaxY - mdSrcGeoMinY) / resMIN + 1;

		mb_GeoTransformSet = TRUE;
    }
    else //If failed to get bounding box from meta-data, then using GCPs
    {
    	int nGCPs = maptrDS->GetGCPCount();

    	OGRSpatialReference oGCPsSRS;
    	char *psTargetSRS = (char*) maptrDS->GetGCPProjection();
    	if (psTargetSRS && *psTargetSRS != '\0')
    	{
    		if (OGRERR_NONE != oGCPsSRS.importFromWkt(&psTargetSRS))
    			oGCPsSRS = mo_NativeCRS;
    	}
    	else if (nGCPs > 0)
    	{
    		oGCPsSRS = mo_NativeCRS;
    	}

    	const GDAL_GCP* pGCPList = maptrDS->GetGCPs();
    	if (nGCPs < 2 || NULL == pGCPList)
    	{
    		md_Geotransform[0] = 0;
    		md_Geotransform[1] = 1;
    		md_Geotransform[2] = 0;
    		md_Geotransform[3] = 0;
    		md_Geotransform[4] = 0;
    		md_Geotransform[5] = 1;

    		mb_GeoTransformSet = FALSE;
    		return CE_None;
    	}

    	My2DPoint lowLeft;
    	My2DPoint upRight;

    	GetCornerPoints(pGCPList, nGCPs, lowLeft, upRight);

    	if (CE_None != bBox_transFormmate(oGCPsSRS, mo_NativeCRS, lowLeft, upRight))
    		return CE_Failure;

    	resX = fabs(upRight.mi_X - lowLeft.mi_X) / (maptrDS->GetRasterXSize());
    	resY = fabs(upRight.mi_Y - lowLeft.mi_Y) / (maptrDS->GetRasterYSize());

    	resX = MIN(resX,resY);
    	resY = MIN(resX,resY);

    	md_Geotransform[0] = lowLeft.mi_X;
    	md_Geotransform[1] = resX;
    	md_Geotransform[2] = 0;
    	md_Geotransform[3] = upRight.mi_Y;
    	md_Geotransform[4] = 0;
    	md_Geotransform[5] = -resY;

    	mi_RectifiedImageXSize = fabs(upRight.mi_X - lowLeft.mi_X) / resX + 1;
    	mi_RectifiedImageYSize = fabs(upRight.mi_Y - lowLeft.mi_Y) / resY + 1;

    	mb_GeoTransformSet = TRUE;
    }

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

CPLErr HE5_SWATH_Dataset::SetMetaDataList(GDALDataset* hSrcDS)
{
	mv_MeteDataList.push_back("Product_Description=The data was created by WCS from HDF-EOS5 SWATH data.");

	char **papszMetadata = hSrcDS->GetMetadata("");
	int metadataCount = CSLCount((char**) papszMetadata);
	for (int i = 0; i < metadataCount; ++i)
	{
		mv_MeteDataList.push_back(papszMetadata[i]);
		KVP kvpStr1(papszMetadata[i]);
		if(EQUAL(kvpStr1.name.c_str(), "FILE_ATTRIBUTES:InstrumentName"))
			ms_CoverageInstrument = kvpStr1.value;
	}

	GDALRasterBand* pRB = hSrcDS->GetRasterBand(1);
	char **pRBMetadata = pRB->GetMetadata("");
	int metadataCount1 = CSLCount((char**) pRBMetadata);
	for (int i = 0; i < metadataCount1; ++i)
	{
		mv_MeteDataList.push_back(pRBMetadata[i]);
		KVP kvpStr2(pRBMetadata[i]);
		if(Find_Compare_SubStr(kvpStr2.name, "_FillValue"))
			convertFromString(md_MissingValue, kvpStr2.value);
		else if(Find_Compare_SubStr(kvpStr2.name, "Units"))
			ms_FieldQuantityDef = kvpStr2.value;
		else if(Find_Compare_SubStr(kvpStr2.name, "ValidRange"))
			ms_AllowRanges = kvpStr2.value;
	}

	ms_CoveragePlatform = "Aura";
	ms_CoverageSensor = "CCD Ultra Violet";

	return CE_None;
}

/************************************************************************/
/*                           SetGDALDataset()                           */
/************************************************************************/

/**
 * \brief Set the GDALDataset object to HDF-EOS5 Swath dataset.
 *
 * This method is used to set the HDF-EOS5 Swath dataset.
 *
 * @param isSimple the WCS request type.  When user executing a DescribeCoverage
 * request, isSimple is set to 1, and for GetCoverage, is set to 0.
 *
 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr HE5_SWATH_Dataset::SetGDALDataset(const int isSimple)
{
/*	int nXSize = mi_RectifiedImageXSize;
	int nYSize = mi_RectifiedImageYSize;

	int mBands = mv_BandList.size();
	int sBands = maptrDS.get()->GetRasterCount();
	int tBands = (mBands == 0) ? sBands : mBands;
	if(mBands == 0)
	{
		for(int i = 1; i <= sBands; i++)
			mv_BandList.push_back(i);
	}

	GDALDataType eDT = maptrDS.get()->GetRasterBand(1)->GetRasterDataType();
	char *psGeoSRS = NULL;
	mo_NativeCRS.exportToWkt(&psGeoSRS);

	GDALDriverH hDriver = GDALGetDriverByName("MEM");
	GDALDataset* hSubDS = (GDALDataset*)GDALCreate( hDriver, "", nXSize, nYSize, tBands, eDT, NULL );
	hSubDS->SetProjection(psGeoSRS);
	hSubDS->SetGeoTransform(md_Geotransform);

	if(!isSimple)
	{
		if (CE_None != GDALReprojectImage(maptrDS.get(), NULL, hSubDS, psGeoSRS, GRA_NearestNeighbour, 0, 0.125, NULL, NULL, NULL))
		{
			GDALClose(hSubDS);
			SetWCS_ErrorLocator("HE5_SWATH_Dataset::SetGDALDataset()");
			WCS_Error(CE_Failure, OGC_WCS_NoApplicableCode,
					"Failed to re-project the coverage from original GCPs projection to affine geo-transform projection.");
			return CE_Failure;
		}
	}

	VSIFree((char*) psGeoSRS);

	GDALClose(maptrDS.release());
	maptrDS.reset(hSubDS);
*/
	return CE_None;
}
