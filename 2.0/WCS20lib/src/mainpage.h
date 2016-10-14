/*
 * $Id: mainpage.h 2011-07-19 16:24:00Z $
 *
 * Project:  The Open Geospatial Consortium (OGC) Web Coverage Service (WCS)
 * 			 for Earth Observation: Open Source Reference Implementation
 * Purpose:	 Main page for doxygen.
 * Author:   Yuanzheng Shao, yshao3@gmu.edu
 *
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
 */

/*! \mainpage GMU Earth Observation WCS v2.0 Documentation
 *
 * \section intro Introduction
 * This manual documents the C++ APIs for GMU EO-WCS.\n
 * EO-WCS is an open source reference implementation for the Open Geospatial Consortium (OGC) Web Coverage Service (WCS) - 8.\n
 * ( For more information about EO-WCS, check the website: http://geobrain.laits.gmu.edu/wcseodemo.html)\n
 *
 *
 * \section license License
 * This software is released under the MIT license, anybody can use or modify the package, even for commercial applications.\n
 * The only restriction is to retain the copyright in the sources or the binaries documentation.\n
 * Neither the author, nor the university accept any responsibility for any kind of error or data loss which may occur during usage.
 *
 * \section standards Followed Standards
 *  - OGC WCS 2.0 Interface Standard - Core (OGC 09-110r3)
 *  - OGC WCS 2.0 Interface Standard - KVP Protocol binding Extension (OGC 09-147r1)
 *  - OGC WCS 2.0 Interface Standard - XML POST Protocol binding Extension (OGC 09-148r1)
 *  - OGC WCS 2.0 Application Profile - Earth Observation (OGC 10-140)
 *
 * \section reqlibs Required libraries
 *  - GDAL library at server (http://www.gdal.org)
 *  - HDF4 library at server (http://www.hdfgroup.org)
 *  - HDF5 library at server (http://www.hdfgroup.org)
 *  - HDF-EOS library at server (http://www.hdfeos.org/)
 *
 *  We tested this package with a Linux Fedora 12 and Mac OS 15 as the servers and clients.
 *
 * \section supportformats Supported Data Type
 *  - HDF-EOS2 Grid, including MOD13C1, MOD09GQ, MYD15A2
 *  - HDF-EOS2 Swath, including MOD05_L2
 *  - HDF-EOS5 Grid, including Aura OMSO2G
 *  - HDF-EOS5 Swath, including Aura OMDOAO3
 *  - HDF4, Tropical Rainfall Measuring Mission (TRMM)
 *  - NetCDF, NOAA GOES Imager data
 *
 * \section compilenotes Compiling Notes
 * GDAL library is needed before compiling WCS package.\n
 * Be sure that the support to HDF4 library should be enabled in GDAL library when compiling GDAL from source code.\n
 * HDF-EOS library is also required to enable the HDF-EOS export format.
 *
 * \section sourceCode Source Code
 * GMU WCS source code can be downloaded from http://geobrain.laits.gmu.edu/ows8/code/gmu_eowcs_v0.1.tar.gz
 *
 * \author Yuanzheng Shao <yshao3@gmu.edu>, Liping Di <ldi@gmu.edu>
 */
