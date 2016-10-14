/******************************************************************************
 * $Id: wcst.cpp 2011-07-19 16:24:00Z $
 *
 * Project:  The Open Geospatial Consortium (OGC) Web Coverage Service (WCS)
 * 			 for Earth Observation: Open Source Reference Implementation
 * Purpose:  Main function for WCS
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

#include <iostream>
#include <time.h>
#include "wcsUtil.h"
#include "WCS_T.h"
#include "wcs_error.h"

using namespace std;

static void Usage()
{
	cout << "Usage: [--help]" << endl;
	cout << "       [-of xml_filename] [output_filename]" << endl;
	cout << "       [-os get_method_string] [output_filename]" << endl;
	cout << "Please access http://geobrain.laits.gmu.edu/wcseodemo.html for details." << endl;
}

int main(int argc, char **argv)
{
	CPLErrorReset();
	CPLSetErrorHandler(WCS_ErrorHandler);

	string inputStr;
	string outFileName;
	string confNm = argv[0] + string(".conf");

	GDALAllRegister();
	if (argc > 1 && argc <= 4)
	{
		try
		{
			int i=1;
			if(argc>2)
			{
				inputStr = argv[i+1];
	    		if(argc>3)
	    			outFileName= argv[i+2];
			}
			if (EQUAL(argv[1],"-of"))
			{
				if(EQUAL(inputStr.c_str(), ""))
				{
					cout << "Please Assign The Request XML File Path."<<endl;
					Usage();
					return -1;
				}

				clock_t start, finish;
				double duration;
				start = clock();

				WCS_T* wcst = WCSTOpenFromXMLFile(inputStr, confNm);
				if (!wcst)
				{
					cout << GetWCS_ErrorMsg() << endl;
					return -1;
				}
				wcst->WCST_Respond(outFileName);

				finish = clock();
				duration = (double)(finish-start)/CLOCKS_PER_SEC;
				cout<<"Execute time is: "<<duration<<" second."<<endl;

				cout << "Output File name = " << outFileName << endl;
				WCSTClose(wcst);
				cout << "Succeed" << endl;
			}
			else if (EQUAL(argv[1],"-os"))
			{
				if(EQUAL(inputStr.c_str(), ""))
				{
					cout << "Please Assign The Request String."<<endl;
					cout << "GetCoverage Operation Request String Example:" << endl;
					Usage();
					return -1;
				}

				clock_t start, finish;
				double duration;
				start = clock();

				WCS_T* wcst = WCSTOpenFromURLString(inputStr, confNm);
				if (!wcst)
				{
					cout << GetWCS_ErrorMsg() << endl;
					return -1;
				}
				wcst->WCST_Respond(outFileName);

				finish = clock();
				duration = (double)(finish-start)/CLOCKS_PER_SEC;
				cout<<"Execute time is: "<<duration<<" second."<<endl;

				cout << "Output File name = " << outFileName << endl;
				WCSTClose(wcst);
				cout << "Succeed" << endl;
			}
			else if (EQUAL(argv[1],"--help") || EQUAL(argv[1],"-h"))
			{
				Usage();
			}
			else
			{
				cout << "argv[] error." << endl;
				Usage();
			}
		} catch (...)
		{
			cout << GetWCS_ErrorMsg() << endl;
			return -1;
		}
	}
	else if (argc == 1)//CGI execuate
	{
		try
		{
			WCSCGI cgi;
			if (CE_None != cgi.Run() || UN_KNOWN == cgi.GetCGImethod())
			{
				cout<<"Content-Type: text/xml"<<endl<<endl;
				cout << GetWCS_ErrorMsg() << endl;
				return -1;
			}
			if (HTTP_GET == cgi.GetCGImethod())
			{
				WCS_T* wcst = WCSTOpenFromURLString(cgi.GetRqstContent(),confNm);
				if (!wcst)
				{;
					cout<<"Content-Type: text/xml"<<endl<<endl;
					cout << GetWCS_ErrorMsg() << endl;
					return -1;
				}
				wcst->WCST_Respond();
				WCSTClose(wcst);
			}
			else if (HTTP_XML_POST == cgi.GetCGImethod())
			{
				WCS_T* wcst = WCSTOpenFromXMLString(cgi.GetRqstContent(), confNm);
				if (!wcst)
				{
					cout<<"Content-Type: text/xml"<<endl<<endl;
					cout << GetWCS_ErrorMsg()<< endl;
					return -1;
				}
				wcst->WCST_Respond();
				WCSTClose(wcst);
			}
		} catch (...)
		{
			cout<<"Content-Type: text/xml"<<endl<<endl;
			cout << GetWCS_ErrorMsg() << endl;
			return -1;
		}
	}
	else
		Usage();

	GDALDestroyDriverManager();
	return 0;
}

