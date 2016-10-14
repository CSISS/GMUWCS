/******************************************************************************
 * $Id: WCS_Configure.cpp 2011-07-19 16:24:00Z $
 *
 * Project:  The Open Geospatial Consortium (OGC) Web Coverage Service (WCS)
 * 			 for Earth Observation: Open Source Reference Implementation
 * Purpose:  WCS_Configure class implementation
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

#include "WCS_Configure.h"

/************************************************************************/
/* ==================================================================== */
/*                           WCS_Configure                              */
/* ==================================================================== */
/************************************************************************/

/**
 * \class WCS_Configure "WCS_Configure.h"
 *
 * A configuration file contains WCS-related parameters, which is placed
 * under the same directory with WCS executable. The configuration file
 * also has the same name with WCS executable but plus an suffix "conf".
 *
 * This configuration file is required for WCS service.
 */

/************************************************************************/
/*                            WCS_Configure()                           */
/************************************************************************/

/**
 * \brief Constructor of an WCS_Configure object.
 *
 * Constructor of an WCS_Configure object.
 */

WCS_Configure::WCS_Configure()
{

}

WCS_Configure::~WCS_Configure()
{

}

/************************************************************************/
/*                            WCS_Configure()                           */
/************************************************************************/

/**
 * \brief Constructor of an WCS_Configure object.
 *
 * This is the accepted method of creating an configuration object and
 * based this object to read the configuration information.
 *
 * @param conf String of the full path of the configuration file.
 *
 */

WCS_Configure::WCS_Configure(const string &conf) :
	map_Config(new CFGReader(conf)),ms_ConfigureFile(conf)
{

}

/************************************************************************/
/*                            GetConfigureFileName()                    */
/************************************************************************/

/**
 * \brief Fetch the full path of the configuration file.
 *
 * This method will return the full path of the configuration file.
 *
 * @return String of the full path of the configuration file.
 */

string WCS_Configure::GetConfigureFileName()
{
	return ms_ConfigureFile;
}

/************************************************************************/
/*                   Get_CAPABILITIES_HEAD_FILE_PATH()                  */
/************************************************************************/

/**
 * \brief Fetch the XML head element of the capabilities document.
 *
 * This method will return the XML head element of the capabilities document.
 *
 * @return String of the head element of the capabilities document.
 */

string WCS_Configure::Get_CAPABILITIES_HEAD_FILE_PATH()
{
	return map_Config->getValue("CAPABILITIES_HEAD_FILE_PATH", "");
}

/************************************************************************/
/*          Get_CAPABILITIES_SEVICEIDENTIFICATION_FILE_PATH()           */
/************************************************************************/

/**
 * \brief Fetch the ServiceIdentification part of the capabilities document.
 *
 * This method will return the ServiceIdentification part of the
 * capabilities document.
 *
 * @return String of the ServiceIdentification part of the capabilities document.
 */

string WCS_Configure::Get_CAPABILITIES_SEVICEIDENTIFICATION_FILE_PATH()
{
	return map_Config->getValue("CAPABILITIES_SEVICEIDENTIFICATION_FILE_PATH", "");
}

/************************************************************************/
/*          Get_CAPABILITIES_SEVICEPROVIDER_FILE_PATH()                 */
/************************************************************************/

/**
 * \brief Fetch the ServiceProvider part of the capabilities document.
 *
 * This method will return the ServiceProvider part of the
 * capabilities document.
 *
 * @return String of the ServiceProvider part of the capabilities document.
 */

string WCS_Configure::Get_CAPABILITIES_SEVICEPROVIDER_FILE_PATH()
{
	return map_Config->getValue("CAPABILITIES_SEVICEPROVIDER_FILE_PATH", "");
}

/************************************************************************/
/*          Get_CAPABILITIES_OPERATIONSMETADA_FILE_PATH()               */
/************************************************************************/

/**
 * \brief Fetch the OperationsMetadata part of the capabilities document.
 *
 * This method will return the OperationsMetadata part of the
 * capabilities document.
 *
 * @return String of the OperationsMetadata part of the capabilities document.
 */

string WCS_Configure::Get_CAPABILITIES_OPERATIONSMETADA_FILE_PATH()
{
	return map_Config->getValue("CAPABILITIES_OPERATIONSMETADA_FILE_PATH", "");
}

/************************************************************************/
/*                Get_CAPABILITIES_CONTENTS_FILE_PATH()                 */
/************************************************************************/

/**
 * \brief Fetch the Contents part of the capabilities document.
 *
 * This method will return the Contents part of the
 * capabilities document.
 *
 * @return String of the Contents part of the capabilities document.
 */

string WCS_Configure::Get_CAPABILITIES_CONTENTS_FILE_PATH()
{
	return map_Config->getValue("CAPABILITIES_CONTENTS_FILE_PATH", "");
}

/************************************************************************/
/*                        Get_SERVICE_ACCESS_URL()                      */
/************************************************************************/

/**
 * \brief Fetch the access URL of this WCS.
 *
 * This method will return the access URL of this WCS.
 *
 * @return String of the access URL of this WCS.
 */

string WCS_Configure::Get_SERVICE_ACCESS_URL()
{
	return map_Config->getValue("SERVICE_ACCESS_URL", "");
}

/************************************************************************/
/*                    Get_WCS_SERVICE_DATA_DIRECTORY()                  */
/************************************************************************/

/**
 * \brief Fetch the directory of the data-to-be-served.
 *
 * This method will return the directory of the data-to-be-served.
 *
 * @return String of the directory of the data-to-be-served.
 */

string WCS_Configure::Get_WCS_SERVICE_DATA_DIRECTORY()
{
	return map_Config->getValue("WCS_SERVICE_DATA_DIRECTORY", "");
}

/************************************************************************/
/*                  Get_DATASET_CONFIGRATION_FILE_PATH()                */
/************************************************************************/

/**
 * \brief Fetch the configuration XML file path of dataset coverage.
 *
 * To improve the performance of WCS and make it easy to add the support
 * to new datasets, some specified information of each dataset/granule
 * will be extracted and build a XML file based on those information,
 * such as coverage identifier in GDAL, bounding box, temporal range,
 * coverage name to-be-displayed to user.
 *
 * On dataset's configuration file could include many dataset, and also the values
 * of 'DATASET_CONFIGRATION_FILE_PATH' could be specified by multiple dataset's
 * configuration files, separated by ',' or ';'.
 *
 * Sample:
 * @code
 * <Datasets>
 *   <Dataset>
 *     <name>MOD05_L2.A2008205.0255.005.2008206170326.hdf:Water_Vapor_Infrared</name>
 *	   <path>/home/yshao/data/</path>
 *	   <coverageID>HDF4_EOS:EOS_SWATH:"/home/yshao/data/MOD05_L2.A2008205.0255.005.2008206170326.hdf":mod05:Water_Vapor_Infrared</coverageID>
 *	   <west>-93.8909</west>
 *	   <east>-59.1097</east>
 *	   <south>32.2516</south>
 * 	   <north>53.7898</north>
 *	   <beginTime>2008-07-23T02:55:00Z</beginTime>
 *	   <endTime>2008-07-23T03:00:00Z</endTime>
 *     </Dataset>
 *   <Dataset>
 *     ...
 *   </Dataset>
 * </Datasets>
 * @endcode
 *
 * @return String of the paths of the dataset's configuration file.
 */

string WCS_Configure::Get_DATASET_CONFIGRATION_FILE_PATH()
{
	return map_Config->getValue("DATASET_CONFIGRATION_FILE_PATH", "");
}

/************************************************************************/
/*                  Get_DATASET_CONFIGRATION_FILE_PATH()                */
/************************************************************************/

/**
 * \brief Fetch the configuration XML file path of stitched mosaic coverage.
 *
 * To improve the performance of WCS and make it easy to add the support
 * to new mosaic coverage, some specified information of each dataset/granule
 * within the mosaic coverage will be extracted and build a XML file
 * based on those information.
 *
 * @todo Currently (as of 07/22) the GMU WCS did not serve the data in
 * the form of mosaic coverage, some demos will be added based on new
 * requirement.
 *
 * @return String of the paths of the mosaic coverage's configuration file.
 */

string WCS_Configure::Get_STITCHED_MOSAIC_CONFIGRATION_FILE_PATH()
{
	return map_Config->getValue("STITCHED_MOSAIC_CONFIGRATION_FILE_PATH", "");
}

/************************************************************************/
/*                Get_DATASET_SERIES_CONFIGRATION_FILE_PATH()           */
/************************************************************************/

/**
 * \brief Fetch the configuration XML file path of datasetSeries coverage.
 *
 * To improve the performance of WCS and make it easy to add the support
 * to new datasetSeries coverage, some specified information of each
 * dataset/granule within the dataset series will be extracted and build
 * a XML file based on those information.
 *
 * Sample:
 * @code
 <WCSEODatasetSeries>
 <DatasetSeries>
 <DatasetSeriesId>MODIS_Vegetation_Indices_NDVI_16-Day_L3_Global_0.05Deg_Year_2010</DatasetSeriesId>
 <CollectionName>MOD13C1</CollectionName>
 <CoverageName>CMG 0.05 Deg 16 days NDVI</CoverageName>
 <Datasets>
 <Dataset>
 <name>MOD13C1.A2010289.005.2010310073051.hdf</name>
 <path>/home/yshao/data/MOD13C1.A2010289.005.2010310073051.hdf</path>
 <coverageID>HDF4_EOS:EOS_GRID:"/home/yshao/data/MOD13C1.A2010289.005.2010310073051.hdf":MODIS_Grid_16Day_VI_CMG:CMG 0.05 Deg 16 days NDVI</coverageID>
 <west>-180</west>
 <east>180</east>
 <south>-90</south>
 <north>90</north>
 <beginTime>2010-07-28T00:00:00Z</beginTime>
 <endTime>2010-08-12T23:59:59Z</endTime>
 </Dataset>
 <Dataset>
 ...
 </Dataset>
 </Datasets>
 </DatasetSeries>
 <DatasetSeries>
 ...
 </DatasetSeries>
 </WCSEODatasetSeries>
 * @endcode
 *
 * @return String of the paths of the dataset series' configuration file.
 */

string WCS_Configure::Get_DATASET_SERIES_CONFIGRATION_FILE_PATH()
{
	return map_Config->getValue("DATASET_SERIES_CONFIGRATION_FILE_PATH", "");
}

/************************************************************************/
/*                    Get_TRANSACTION_DATA_DIRECTORY()                  */
/************************************************************************/

/**
 * \brief Fetch the directory for placing the data will be transacted.
 *
 * This method will return the directory for placing the data will be
 * transacted.
 *
 * @todo Currently (as of 07/22) the transaction interface of GMU WCS
 * have not been tested.
 *
 * @return String of the directory of the data-to-be-served.
 */

string WCS_Configure::Get_TRANSACTION_DATA_DIRECTORY()
{
	return map_Config->getValue("TRANSACTION_DATA_DIRECTORY", "");
}

/************************************************************************/
/*                    Get_TEMPORARY_OUTPUT_DIRECTORY()                  */
/************************************************************************/

/**
 * \brief Fetch the directory for placing temporary and output files.
 *
 * This method will return the directory for placing temporary and output files.
 *
 * @return String of the directory for placing temporary and output files.
 */

string WCS_Configure::Get_TEMPORARY_OUTPUT_DIRECTORY()
{
	return map_Config->getValue("TEMPORARY_OUTPUT_DIRECTORY", "");
}

/************************************************************************/
/*                        Get_OUTPUT_PREFIX_URL()                       */
/************************************************************************/

/**
 * \brief Fetch the URL prefix corresponding to the temporary directory.
 *
 * This method will return the URL prefix corresponding to the temporary directory.
 *
 * @return String of the URL prefix corresponding to the temporary directory.
 */

string WCS_Configure::Get_OUTPUT_PREFIX_URL()
{
	return map_Config->getValue("OUTPUT_PREFIX_URL", "");
}

/************************************************************************/
/*                        Get_CAPABILITIES_FILE_PATH()                  */
/************************************************************************/

/**
 * \brief Fetch the path of the capabilities file path.
 *
 * This method will return the path of the capabilities file path. If
 * this value is specified, WCS will import the file specified and display
 * the XML contents to user.
 *
 * @return String of the path of the capabilities file path.
 */


string WCS_Configure::Get_CAPABILITIES_FILE_PATH()
{
	return map_Config->getValue("CAPABILITIES_FILE_PATH", "");
}

/************************************************************************/
/*                        Get_WCS_LOGFILE_PATH()                        */
/************************************************************************/

/**
 * \brief Fetch the path of the WCS log file path.
 *
 * This method will return the path of the WCS log file path.
 *
 * @return String of the path of the WCS log file path.
 */

string WCS_Configure::Get_WCS_LOGFILE_PATH()
{
	return map_Config->getValue("WCS_LOGFILE_PATH", "");
}

/************************************************************************/
/*                Get_ISO_19115_METADATA_TEMPLATE_PATH()                */
/************************************************************************/

/**
 * \brief Fetch the path of the ISO 19115 template file.
 *
 * This method will return the path of the ISO 19115 template file.
 *
 * @return String of the path of the ISO 19115 template file.
 */

string WCS_Configure::Get_ISO_19115_METADATA_TEMPLATE_PATH()
{
	return map_Config->getValue("ISO_19115_METADATA_TEMPLATE_PATH", "");
}

/************************************************************************/
/*                        Get_GDAL_WARP_PATH()                          */
/************************************************************************/

/**
 * \brief Fetch the path of gdalwap command line.
 *
 * This method will return the path of gdalwap command line
 *
 * @return String of the path of gdalwap command line
 */

string WCS_Configure::Get_GDAL_WARP_PATH()
{
	return map_Config->getValue("GDAL_WARP_PATH", "");
}

/************************************************************************/
/*                    Get_GDAL_TRANSLATE_PATH()                         */
/************************************************************************/

/**
 * \brief Fetch the path of gdal_translate command line.
 *
 * This method will return the path of gdal_translate command line
 *
 * @return String of the path of gdal_translate command line
 */

string WCS_Configure::Get_GDAL_TRANSLATE_PATH()
{
	return map_Config->getValue("GDAL_TRANSLATE_PATH", "");
}

/************************************************************************/
/*                   Get_KAKADU_COMPRESS_PATH()                         */
/************************************************************************/

/**
 * \brief Fetch the path of kdu_compress command line.
 *
 * This method will return the path of kakadu compress command line,
 * which is used to convert GeoTIFF format to JPEG2000, then delivered the
 * data with JPIP protocol.
 *
 * @return String of the path of kdu_compress command line
 */

string WCS_Configure::Get_KAKADU_COMPRESS_PATH()
{
	return map_Config->getValue("KAKADU_COMPRESS_PATH", "");
}
