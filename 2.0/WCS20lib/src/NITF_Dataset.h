/******************************************************************************
 * $Id: NITF_Dataset.h 2011-07-19 16:24:00Z $
 *
 * Project:  The Open Geospatial Consortium (OGC) Web Coverage Service (WCS)
 * 			 for Earth Observation: Open Source Reference Implementation
 * Purpose:  NITF_Dataset class definition
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

#ifndef NITF_DATASET_H_
#define NITF_DATASET_H_

#include <string>
#include "AbstractDataset.h"
#include "wcsUtil.h"

using namespace std;

/************************************************************************/
/* ==================================================================== */
/*                             NITF_Dataset                             */
/* ==================================================================== */
/************************************************************************/

//! NITF_Dataset is a subclass of AbstractDataset, used to process NITF coverage.

/**
 * \class NITF_Dataset "NITF_Dataset.h"
 *
 * The National Imagery Transmission Format Standard (NITFS) is a U.S. Department of Defense (DoD)
 * and Federal Intelligence Community (IC) suite of standards for the exchange, storage, and transmission
 * of digital-imagery products and image-related products.
 *
 * DoD policy is that other image formats can be used internally within a single system; however, NITFS
 * is the default format for interchange between systems. NITFS provides a package containing information
 * about the image, the image itself, and optional overlay graphics. (i.e. a "package" containing an
 * image(s), subimages, symbols, labels, and text as well as other information related to the image(s))
 * NITFS supports the dissemination of secondary digital imagery from overhead collection platforms.
 *
 * NITF_Dataset is a subclass of AbstractDataset, which is used to
 * process NITF products.
 */

class NITF_Dataset : public AbstractDataset
{
public:
	NITF_Dataset();

	virtual ~NITF_Dataset();
	virtual CPLErr SetMetaDataList(GDALDataset* );
	virtual CPLErr SetNativeCRS();
	virtual CPLErr SetGeoTransform();
	virtual CPLErr SetGDALDataset(const int isSimple=0);
	virtual CPLErr InitialDataset(const int isSimple=0);

public:
	NITF_Dataset(const string& id, vector<int> &rBandList);
};

#endif /* NITF_DATASET_H_ */
