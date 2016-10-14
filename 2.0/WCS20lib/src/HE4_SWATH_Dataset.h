/******************************************************************************
 * $Id: HE4_SWATH_Dataset.h 2011-07-19 16:24:00Z $
 *
 * Project:  The Open Geospatial Consortium (OGC) Web Coverage Service (WCS)
 * 			 for Earth Observation: Open Source Reference Implementation
 * Purpose:  HE4_SWATH_Dataset class definition
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

#ifndef HE4_SWATH_DATASET_H_
#define HE4_SWATH_DATASET_H_

#include <string>
#include "AbstractDataset.h"

using namespace std;

/************************************************************************/
/* ==================================================================== */
/*                             HE4_SWATH_Dataset                        */
/* ==================================================================== */
/************************************************************************/

//! HE4_SWATH_Dataset is a subclass of AbstractDataset, used to process HDF-EOS2 Swath coverage.

/**
 * \class HE4_SWATH_Dataset "HE4_SWATH_Dataset.h"
 *
 * The Swath concept for HDF-EOS is based on a typical satellite swath,
 * where an instrument takes a series of scans perpendicular to the
 * ground track of the satellite as it moves along that ground track.
 *
 * The HDF-EOS2 data view of a swath is one where the data is ordered by
 * time or a time-like variable (e.g., scan line counter). The data
 * stored for every time entry can consist of time, geolocation (latitude,
 * longitude), scalar values, 1D arrays of values (scan lines or profiles),
 * or 2D arrays of values (multiple channel scan lines).
 *
 * HE4_SWATH_Dataset is a subclass of AbstractDataset, which is used to
 * process MODIS Swath products collection, such as MOD021KM, MYD02HKM and
 * MOD02QKM.
 */

class HE4_SWATH_Dataset : public AbstractDataset
{
protected:
	int mi_RectifiedImageXSize;
	int mi_RectifiedImageYSize;

public:
	HE4_SWATH_Dataset();
	virtual CPLErr SetGeoTransform();
	virtual CPLErr SetMetaDataList(GDALDataset* );
	virtual CPLErr SetNativeCRS();
	virtual CPLErr SetGDALDataset(const int isSimple=0);
	virtual CPLErr InitialDataset(const int isSimple=0);

public:
	HE4_SWATH_Dataset(const string& id, vector<int> &rBandList);
	virtual ~HE4_SWATH_Dataset();

	virtual int GetImageXSize()
	{
		return mi_RectifiedImageXSize;
	}

	virtual int GetImageYSize()
	{
		return mi_RectifiedImageYSize;
	}
};

#endif /*HE4_SWATH_DATASET_H_*/
