/******************************************************************************
 * $Id: WCS_GetCapabilities.h 2011-07-19 16:24:00Z $
 *
 * Project:  The Open Geospatial Consortium (OGC) Web Coverage Service (WCS)
 * 			 for Earth Observation: Open Source Reference Implementation
 * Purpose:  WCS_GetCapabilities class definition
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

#ifndef WCS_GETCAPABILITIES_H_
#define WCS_GETCAPABILITIES_H_

#include "WCS_T.h"

/* ******************************************************************** */
/*                          WCS_DescribeCoverage                        */
/* ******************************************************************** */

//! This class is used to handle GetCapabilities request.

class WCS_GetCapabilities: public WCS_T
{
protected:
	vector<string> 	mv_sections;

private:
	void CreateCapabilitiesXMLTree(ostringstream& ostream);
	void CreateCapabilitiesXMLHead(ostringstream& ostream);
	void WriteServiceIdentification(ostringstream& ostream);
	void WriteServiceProvider(ostringstream& ostream);
	void WriteOperationsMetadata(ostringstream& ostream);
	void WriteContents(ostringstream& ostream);

public:
	WCS_GetCapabilities();
	WCS_GetCapabilities(const string& conf);
	virtual ~WCS_GetCapabilities();

	virtual CPLErr GetReqMessageFromXMLTree(CPLXMLNode *xmlRoot);
	virtual CPLErr GetReqMessageFromURLString(const string&);

	virtual void WCST_Respond(string& sOutFileName);
	virtual void WCST_Respond();
};

#endif /* WCS_GETCAPABILITIES_H_ */
