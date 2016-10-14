/******************************************************************************
 * $Id: HE5_GRID_Dataset.cpp 2011-07-29 16:24:00Z $
 *
 * Project:  The Open Geospatial Consortium (OGC) Web Coverage Service (WCS)
 * 			 for Earth Observation: Open Source Reference Implementation
 * Purpose:  HE2_GRID_Dataset implementation for HDF-EOS Grid format
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

#include "HE5_GRID_Dataset.h"

/* -------------------------------------------------------------------- */
/* SR-ORG:6974, MODIS Sinusoidal projection                             */
/* Refer: http://spatialreference.org/ref/sr-org/6974/                  */
/* -------------------------------------------------------------------- */

string HE5_GRID_Dataset::MODIS_Sinusoidal_WKT =
	"PROJCS[\"MODIS Sinusoidal\", \
    GEOGCS[\"WGS 84\", \
        DATUM[\"WGS_1984\", \
            SPHEROID[\"WGS 84\",6378137,298.257223563, \
                AUTHORITY[\"EPSG\",\"7030\"]], \
            AUTHORITY[\"EPSG\",\"6326\"]], \
        PRIMEM[\"Greenwich\",0, \
            AUTHORITY[\"EPSG\",\"8901\"]], \
        UNIT[\"degree\",0.01745329251994328, \
            AUTHORITY[\"EPSG\",\"9122\"]], \
        AUTHORITY[\"EPSG\",\"4326\"]], \
    PROJECTION[\"Sinusoidal\"], \
    PARAMETER[\"false_easting\",0.0], \
    PARAMETER[\"false_northing\",0.0], \
    PARAMETER[\"central_meridian\",0.0], \
    PARAMETER[\"semi_major\",6371007.181], \
    PARAMETER[\"semi_minor\",6371007.181], \
    UNIT[\"m\",1.0], \
    AUTHORITY[\"SR-ORG\",\"6974\"]]";

string HE5_GRID_Dataset::CEA_CRS_WKT =
	"PROJCS[\"NSIDC EASE-Grid Global\", \
		GEOGCS[\"Unspecified datum based upon the International 1924 Authalic Sphere\", \
			DATUM[\"Not_specified_based_on_International_1924_Authalic_Sphere\", \
				SPHEROID[\"International 1924 Authalic Sphere\",6371228,0, \
					AUTHORITY[\"EPSG\",\"7057\"]], \
				AUTHORITY[\"EPSG\",\"6053\"]], \
			PRIMEM[\"Greenwich\",0, \
				AUTHORITY[\"EPSG\",\"8901\"]], \
			UNIT[\"degree\",0.01745329251994328, \
				AUTHORITY[\"EPSG\",\"9122\"]], \
			AUTHORITY[\"EPSG\",\"4053\"]], \
		UNIT[\"metre\",1, \
			AUTHORITY[\"EPSG\",\"9001\"]], \
		PROJECTION[\"Cylindrical_Equal_Area\"], \
		PARAMETER[\"standard_parallel_1\",30], \
		PARAMETER[\"central_meridian\",0], \
		PARAMETER[\"false_easting\",0], \
		PARAMETER[\"false_northing\",0], \
		AUTHORITY[\"EPSG\",\"3410\"], \
		AXIS[\"X\",EAST], \
		AXIS[\"Y\",NORTH]]";

HE5_GRID_Dataset::HE5_GRID_Dataset()
{

}

/************************************************************************/
/*                            HE5_GRID_Dataset()                        */
/************************************************************************/

/**
 * \brief Create an HDF-EOS5 Grid dataset object.
 *
 * This is the accepted method of creating a HE5_GRID_Dataset dataset and
 * allocating all resources associated with it.
 *
 * @param id The coverage identifier.
 *
 * @param rBandList The field list selected for this coverage. For HDF-EOS
 * Grid data, only one field in each coverage.
 *
 * @return A HE5_GRID_Dataset object.
 */

HE5_GRID_Dataset::HE5_GRID_Dataset(const string& id, vector<int> &rBandList) :
	AbstractDataset(id, rBandList)
{
	md_MissingValue = -9999;
}

/************************************************************************/
/*                           ~HE5_GRID_Dataset()                        */
/************************************************************************/

/**
 * \brief Destroy an open HE5_GRID_Dataset object.
 *
 * This is the accepted method of closing a HE5_GRID_Dataset dataset and
 * deallocating all resources associated with it.
 */

HE5_GRID_Dataset::~HE5_GRID_Dataset()
{

}

/************************************************************************/
/*                           InitialDataset()                           */
/************************************************************************/

/**
 * \brief Initialize the HDF-EOS5 Grid dataset .

 * This method is the implementation for initializing a HDF-EOS5 Grid dataset.
 * Within this method, SetNativeCRS(), SetGeoTransform() and SetGDALDataset()
 * will be called to initialize an HDF-EOS Grid dataset.
 * * The coverage type of HDF-EOS Swath data is set to "RectifiedDataset".
 *
 * @param isSimple The WCS request type.  When user executing a DescribeCoverage
 * request, isSimple is set to 1, and for GetCoverage, is set to 0.
 *
 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr HE5_GRID_Dataset::InitialDataset(const int isSimple)
{
	ms_CoverageSubType = "RectifiedDataset";

	string tmpStr;
	vector<string> strSet;
	unsigned int n = CsvburstCpp(ms_CoverageID, strSet, ':');

	if (n != 3)
	{
		SetWCS_ErrorLocator("HE5_GRID_Dataset::initialDataset()");
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
		SetWCS_ErrorLocator("HE5_GRID_Dataset::initialDataset()");
		WCS_Error(CE_Failure, OGC_WCS_NoApplicableCode,
				"Failed to open file \"%s\".", ms_SrcFilename.c_str());
		return CE_Failure;
	}

	////fetch data format
	ms_NativeFormat = GDALGetDriverShortName(pSrc->GetDriver());
	if (!EQUAL(ms_NativeFormat.c_str(),"HDF5Image"))
	{
		GDALClose(pSrc);
		SetWCS_ErrorLocator("HE5_GRID_Dataset::initialDataset()");
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
		SetWCS_ErrorLocator("HE5_GRID_Dataset::initialDataset()");
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
/*                           SetGeoTransform()                          */
/************************************************************************/

/**
 * \brief Set the affine GeoTransform matrix for a HDF-EOS5 Grid coverage.
 *
 * The method will set a GeoTransform matrix for a HDF-EOS5 Grid coverage
 * by parsing and analyzing the metadata of HDF-EOS5 Grid data granule.
 *
 * The CRS for the bounding box is EPSG:4326.
 *
 * @return CE_None on success or CE_Failure on failure.
 */

CPLErr HE5_GRID_Dataset::SetGeoTransform()
{
	char **papszMetadata = maptrDS->GetMetadata("");
	double resX,resY;
	double mdSrcGeoMinX, mdSrcGeoMaxX, mdSrcGeoMinY, mdSrcGeoMaxY;
    if( NULL!= CSLFetchNameValue( papszMetadata, "OMI Total Column Amount SO2:Projection" ) &&
    	NULL!= CSLFetchNameValue( papszMetadata, "OMI Total Column Amount SO2:GridSpacing" ) &&
    	NULL!= CSLFetchNameValue( papszMetadata, "OMI Total Column Amount SO2:GridSpan" ) )
    {
    	string projType = CSLFetchNameValue(papszMetadata, "OMI Total Column Amount SO2:Projection");
    	string gridSpace = StrTrims(CSLFetchNameValue(papszMetadata, "OMI Total Column Amount SO2:GridSpacing"), ")");
    	string gridSpan = StrTrims(CSLFetchNameValue(papszMetadata, "OMI Total Column Amount SO2:GridSpan"), ")");
    	gridSpan = StrTrims(gridSpan, "(");
    	gridSpace = StrTrims(gridSpace, "(");

    	vector<string> strSet;
    	CsvburstCpp(gridSpan, strSet, ',');

    	convertFromString(mdSrcGeoMinX, strSet[0]);
    	convertFromString(mdSrcGeoMaxX, strSet[1]);
    	convertFromString(mdSrcGeoMinY, strSet[2]);
    	convertFromString(mdSrcGeoMaxY, strSet[3]);

		if(mdSrcGeoMinX > mdSrcGeoMaxX)//Cross IDL
			Exchange(mdSrcGeoMinX, mdSrcGeoMaxX);

		strSet.clear();
		CsvburstCpp(gridSpace, strSet, ',');

		convertFromString(resX, strSet[0]);
		convertFromString(resY, strSet[0]);

		md_Geotransform[0] = mdSrcGeoMinX;
		md_Geotransform[1] = resX;
		md_Geotransform[2] = 0;
		md_Geotransform[3] = mdSrcGeoMaxY;
		md_Geotransform[4] = 0;
		md_Geotransform[5] = -resY;

		mb_GeoTransformSet = TRUE;
    }
    else if(CE_None != AbstractDataset::SetGeoTransform())
		return CE_Failure;

    return CE_None;
}

/************************************************************************/
/*                           SetGDALDataset()                           */
/************************************************************************/

/**
 * \brief Set the GDALDataset object to HDF-EOS5 Grid dataset.
 *
 * This method is used to set the HDF-EOS5 Grid dataset based on
 * GDAL RasterIO related functions.
 *
 * @param isSimple the WCS request type.  When user executing a DescribeCoverage
 * request, isSimple is set to 1, and for GetCoverage, is set to 0.
 *
 * @return CE_None on success or CE_Failure on failure.
 */


CPLErr HE5_GRID_Dataset::SetGDALDataset(const int isSimple)
{
	int nXSize = maptrDS->GetRasterXSize();
	int nYSize = maptrDS->GetRasterYSize();

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
			for(int i = 1; i <= maptrDS.get()->GetRasterCount(); i++)
			{
				maptrDS.get()->GetRasterBand(i)->RasterIO(GF_Read, 0, 0, nXSize, nYSize, pData, nXSize, nYSize, eDT, 0, 0);
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

CPLErr HE5_GRID_Dataset::SetMetaDataList(GDALDataset* hSrcDS)
{
	mv_MeteDataList.push_back("Product_Description=The data was created by WCS from HDF-EOS5 GRID data.");

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
/*                            SetNativeCRS()                            */
/************************************************************************/

/**
 * \brief Set the Native CRS for a HDF-EOS5 Grid dataset.
 *
 * The method will set the CRS for a HDF-EOS5 Grid dataset as an native
 * CRS.
 *
 * @return CE_None on success or CE_Failure on failure.
 */


CPLErr HE5_GRID_Dataset::SetNativeCRS()
{
	if (CE_None == AbstractDataset::SetNativeCRS())
	{
		long iProjCode = 0, iZoneCode = 0, iSphereCode = 0;
		double *adfProjParms = NULL;

		if (OGRERR_NONE != mo_NativeCRS.exportToUSGS(&iProjCode, &iZoneCode, &adfProjParms, &iSphereCode))
		{
			SetWCS_ErrorLocator("HE5_GRID_Dataset::setNativeCRS()");
			WCS_Error(CE_Failure, OGC_WCS_NoApplicableCode,
					"Failed to export CRS to USGS.");
			return CE_Failure;
		}
		CPLFree(adfProjParms);

		/************************************************************************/
		/*  GCTP projection codes.                                              */
		/*	Refer ogr_srs_usgs.cpp for details									*/
		/************************************************************************/
		switch (iProjCode)
		{
			case 99:// Integerized Sinusoidal Grid
			case 31:// Integerized Sinusoidal Grid (the same as 99)
			case 16:// Sinusiodal
			{
				char *tmp = (char*) MODIS_Sinusoidal_WKT.c_str();
				mo_NativeCRS.importFromWkt(&tmp);
				break;
			}
			case 97:// Cylindrical Equal Area (Grid corners set in meters for EASE grid)
			case 98:// Cylindrical Equal Area (Grid corners set in DMS degs for EASE grid)
			{
				char *tmp = (char*) CEA_CRS_WKT.c_str();
				mo_NativeCRS.importFromWkt(&tmp);
				break;
			}
			case 0:// Geographic
			{
				mo_NativeCRS.SetWellKnownGeogCS("WGS84");
				break;
			}
			default:
				break;
		}

		char *wkt = NULL;
		mo_NativeCRS.exportToWkt(&wkt);
		maptrDS->SetProjection(wkt);
		OGRFree(wkt);
	}
	else
	{
		mo_NativeCRS.SetLocalCS("OGC:ImageCRS");
	}

	return CE_None;
}

