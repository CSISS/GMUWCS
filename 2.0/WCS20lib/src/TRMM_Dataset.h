/******************************************************************************
 * $Id: TRMM_Dataset.h 2011-07-19 16:24:00Z $
 *
 * Project:  The Open Geospatial Consortium (OGC) Web Coverage Service (WCS)
 * 			 for Earth Observation: Open Source Reference Implementation
 * Purpose:  TRMM_Dataset class definition
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

#ifndef TRMM_DATASET_H_
#define TRMM_DATASET_H_

#include <string>
#include "AbstractDataset.h"
#include "wcsUtil.h"

using namespace std;

/************************************************************************/
/* ==================================================================== */
/*                             TRMM_Dataset                             */
/* ==================================================================== */
/************************************************************************/

//! TRMM_Dataset is a subclass of AbstractDataset, used to process TRMM coverage.

/**
 * \class TRMM_Dataset "TRMM_Dataset.h"
 *
 * The Tropical Rainfall Measuring Mission (TRMM) is a joint space mission
 * between NASA and the Japan Aerospace Exploration Agency (JAXA) designed
 * to monitor and study tropical rainfall. The term refers to both the mission
 * itself and the satellite that the mission uses to collect data. TRMM is
 * part of NASA's Mission to Planet Earth, a long-term, coordinated research
 * effort to study the Earth as a global system. The satellite was launched
 * on November 27, 1997 from the Tanegashima Space Center in Tanegashima, Japan.
 *
 * TRMM_Dataset is a subclass of AbstractDataset, which is used to
 * process TRMM products.
 */

class TRMM_Dataset : public AbstractDataset
{
public:
	int m_bDaily;

public:
	TRMM_Dataset();

	virtual ~TRMM_Dataset();
	virtual CPLErr SetMetaDataList(GDALDataset* );
	virtual CPLErr SetNativeCRS();
	virtual CPLErr SetGeoTransform();
	virtual CPLErr SetGDALDataset(const int isSimple=0);
	virtual CPLErr InitialDataset(const int isSimple=0);

public:
	TRMM_Dataset(const string& id, vector<int> &rBandList);
};

#endif /* TRMM_DATASET_H_ */
