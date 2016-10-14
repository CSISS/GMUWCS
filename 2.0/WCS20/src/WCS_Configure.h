/******************************************************************************
 * $Id: WCS_Configure.h 2011-07-19 16:24:00Z $
 *
 * Project:  The Open Geospatial Consortium (OGC) Web Coverage Service (WCS)
 * 			 for Earth Observation: Open Source Reference Implementation
 * Purpose:  WCS_Configure class definition, provide get methods to acquire
 * 			 WCS-realted parameters in configuration file
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

#ifndef WCS_CONFIGURE_H_
#define WCS_CONFIGURE_H_

#include <memory>
#include "wcsUtil.h"

using namespace std;

/* ******************************************************************** */
/*                             WCS_Configure                            */
/* ******************************************************************** */

//! Configuration class provides fetch methods to get information from WCS configuration file.

class WCS_Configure
{
	auto_ptr<CFGReader> map_Config;
	string ms_ConfigureFile;

public:
	WCS_Configure();
	WCS_Configure(const string &conf);
	virtual ~WCS_Configure();

	string Get_CAPABILITIES_HEAD_FILE_PATH();
	string Get_CAPABILITIES_SEVICEIDENTIFICATION_FILE_PATH();
	string Get_CAPABILITIES_SEVICEPROVIDER_FILE_PATH();
	string Get_CAPABILITIES_OPERATIONSMETADA_FILE_PATH();
	string Get_CAPABILITIES_CONTENTS_FILE_PATH();

	string Get_SERVICE_ACCESS_URL();
	string Get_WCS_SERVICE_DATA_DIRECTORY();
	string Get_DATASET_CONFIGRATION_FILE_PATH();
	string Get_STITCHED_MOSAIC_CONFIGRATION_FILE_PATH();
	string Get_DATASET_SERIES_CONFIGRATION_FILE_PATH();
	string Get_TRANSACTION_DATA_DIRECTORY();
	string Get_TEMPORARY_OUTPUT_DIRECTORY();
	string Get_OUTPUT_PREFIX_URL();
	string Get_CAPABILITIES_FILE_PATH();
	string Get_WCS_LOGFILE_PATH();
	string Get_GDAL_WARP_PATH();
	string Get_GDAL_TRANSLATE_PATH();
	string Get_KAKADU_COMPRESS_PATH();
	string Get_ISO_19115_METADATA_TEMPLATE_PATH();

	string GetConfigureFileName();
};

#endif /* WCS_CONFIGURE_H_ */
