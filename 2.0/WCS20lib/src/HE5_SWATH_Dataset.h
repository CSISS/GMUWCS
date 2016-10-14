/******************************************************************************
 * $Id: HE5_SWATH_Dataset.h 2011-07-29 16:24:00Z $
 *
 * Project:  The Open Geospatial Consortium (OGC) Web Coverage Service (WCS)
 * 			 for Earth Observation: Open Source Reference Implementation
 * Purpose:  HE5_SWATH_Dataset class definition
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

#ifndef HE5_SWATH_DATASET_H_
#define HE5_SWATH_DATASET_H_

#include <string>
#include "AbstractDataset.h"

using namespace std;

/************************************************************************/
/* ==================================================================== */
/*                             HE5_SWATH_Dataset                        */
/* ==================================================================== */
/************************************************************************/

//! HE5_SWATH_Dataset is a subclass of AbstractDataset, used to process HDF-EOS5 Swath coverage.

/**
 * \class HE5_SWATH_Dataset "HE5_SWATH_Dataset.h"
 *
 * HDF-EOS is a software library designed to support NASA Earth Observing
 * System (EOS) science data. HDF is the Hierarchical Data Format developed
 * by the National Center for Supercomputing Applications. Specific data
 * structures which are containers for science data are: Grid, Point, Zonal
 * Average and Swath. These data structures are constructed from standard
 * HDF data objects, using EOS conventions, through the use of a software
 * library. A key feature of HDF-EOS is a standard prescription for
 * associating geolocation data with science data through internal
 * structural metadata. The relationship between geolocation and science
 * data is transparent to the end-user. Instrument and data type-independent
 * services, such as subsetting by geolocation, can be applied to files
 * across a wide variety of data products through the same library interface.
 *
 * Aura data products are mostly in the new HDF-EOS5 file format (level 2
 * and 3), however there are some products which are not (level 1 OMI
 * is in the old HDF-EOS2.x, and level 1 MLS is in plain HDF5).
 *
 * HE5_SWATH_Dataset is a subclass of AbstractDataset, which is used to
 * process Aura Swath products collection, such as OMI OMDOAO3 data.
 *
 * Sample data could be downloaded from the following link:
 * <a href="ftp://aurapar2u.ecs.nasa.gov/data/s4pa///Aura_OMI_Level2/OMDOAO3.003//2011/211/OMI-Aura_L2-OMDOAO3_2011m0730t0925-o37442_v003-2011m0730t151910.he5">
 * OMI-Aura_L2-OMDOAO3_2011m0730t0925-o37442_v003-2011m0730t151910.he5</a>
 */

class HE5_SWATH_Dataset : public AbstractDataset{
protected:
	int mi_RectifiedImageXSize;
	int mi_RectifiedImageYSize;

public:
	HE5_SWATH_Dataset();
	virtual CPLErr SetGeoTransform();
	virtual CPLErr SetMetaDataList(GDALDataset* );
	virtual CPLErr SetNativeCRS();
	virtual CPLErr SetGDALDataset(const int isSimple=0);
	virtual CPLErr InitialDataset(const int isSimple=0);

public:
	HE5_SWATH_Dataset(const string& id, vector<int> &rBandList);
	virtual ~HE5_SWATH_Dataset();

	virtual int GetImageXSize()
	{
		return mi_RectifiedImageXSize;
	}

	virtual int GetImageYSize()
	{
		return mi_RectifiedImageYSize;
	}
};

#endif /* HE5_SWATH_DATASET_H_ */
