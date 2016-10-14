/******************************************************************************
 * $Id: HE5_GRID_Dataset.h 2011-07-29 16:24:00Z $
 *
 * Project:  The Open Geospatial Consortium (OGC) Web Coverage Service (WCS)
 * 			 for Earth Observation: Open Source Reference Implementation
 * Purpose:  HE5_GRID_Dataset class definition
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

#ifndef HE5_GRID_DATASET_H_
#define HE5_GRID_DATASET_H_

#include <string>
#include "AbstractDataset.h"
#include "wcsUtil.h"

using namespace std;

/************************************************************************/
/* ==================================================================== */
/*                             HE5_GRID_Dataset                         */
/* ==================================================================== */
/************************************************************************/

//! HE5_GRID_Dataset is a subclass of AbstractDataset, used to process HDF-EOS5 Grid coverage.

/**
 * \class HE5_GRID_Dataset "HE5_GRID_Dataset.h"
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
 * HE5_GRID_Dataset is a subclass of AbstractDataset, which is used to
 * process Aura Grid products collection, such as OMI OMSO2G data.
 *
 * Sample data could be downloaded from the following link:
 * <a href="ftp://acdisc.gsfc.nasa.gov/data/s4pa///Aura_OMI_Level2G/OMSO2G.003//2011/OMI-Aura_L2G-OMSO2G_2011m0728_v003-2011m0729t081026.he5">
 * OMI-Aura_L2G-OMSO2G_2011m0728_v003-2011m0729t081026.he5</a>
 */

class HE5_GRID_Dataset : public AbstractDataset
{
public:
	static string MODIS_Sinusoidal_WKT;
	static string CEA_CRS_WKT;

	HE5_GRID_Dataset();
	HE5_GRID_Dataset(const string& id, vector<int> &rBandList);
	virtual ~HE5_GRID_Dataset();

public:
	virtual CPLErr SetMetaDataList(GDALDataset* );
	virtual CPLErr SetNativeCRS();
	virtual CPLErr SetGeoTransform();
	virtual CPLErr SetGDALDataset(const int isSimple=0);
	virtual CPLErr InitialDataset(const int isSimple=0);
};

#endif /* HE5_GRID_DATASET_H_ */
