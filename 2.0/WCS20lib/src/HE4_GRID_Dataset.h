/******************************************************************************
 * $Id: HE4_GRID_Dataset.h 2011-07-19 16:24:00Z $
 *
 * Project:  The Open Geospatial Consortium (OGC) Web Coverage Service (WCS)
 * 			 for Earth Observation: Open Source Reference Implementation
 * Purpose:  HE4_GRID_Dataset class definition
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

#ifndef HE4_GRID_DATASET_H_
#define HE4_GRID_DATASET_H_

#include <string>
#include "AbstractDataset.h"
#include "wcsUtil.h"

using namespace std;

/************************************************************************/
/* ==================================================================== */
/*                             HE4_GRID_Dataset                         */
/* ==================================================================== */
/************************************************************************/

//! HE4_GRID_Dataset is a subclass of AbstractDataset, used to process HDF-EOS2 Grid coverage.

/**
 * \class HE4_GRID_Dataset "HE4_GRID_Dataset.h"
 *
 * To better serve a broader spectrum within the NASA/ECHO with
 * needs for geolocated data, a new format or convention, HDF4-EOS was
 * developed. HDF-EOS2 supports three geospatial data types: grid, point,
 * and swath.
 *
 * For grid structures, the HDF-EOS library uses the U.S. Geological
 * Survey (USGS) General Cartographic Transformation Package (GCTP)
 * conventions for storing projection information.
 *
 * HE4_GRID_Dataset is a subclass of AbstractDataset, which is used to
 * process MODIS Grid products collection, such as MOD09GQ, MOD13C1 and
 * MOD15A2.
 */

class HE4_GRID_Dataset : public AbstractDataset
{
public:
	static string MODIS_Sinusoidal_WKT;
	static string CEA_CRS_WKT;

	HE4_GRID_Dataset();
	virtual CPLErr SetMetaDataList(GDALDataset* );
	virtual CPLErr SetNativeCRS();
	virtual CPLErr SetGeoTransform();
	virtual CPLErr SetGDALDataset(const int isSimple=0);

public:
	HE4_GRID_Dataset(const string& id, vector<int> &rBandList);
	virtual ~HE4_GRID_Dataset();
	virtual CPLErr InitialDataset(const int isSimple=0);

};

#endif /*HE4_GRID_Dataset_H_*/
