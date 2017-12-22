#! /usr/bin/env python

# <<BEGIN-copyright>>
# Copyright (c) 2016, Lawrence Livermore National Security, LLC.
# Produced at the Lawrence Livermore National Laboratory.
# Written by the LLNL Nuclear Data and Theory group
#         (email: mattoon1@llnl.gov)
# LLNL-CODE-683960.
# All rights reserved.
# 
# This file is part of the FUDGE package (For Updating Data and 
#         Generating Evaluations)
# 
# When citing FUDGE, please use the following reference:
#   C.M. Mattoon, B.R. Beck, N.R. Patel, N.C. Summers, G.W. Hedstrom, D.A. Brown, "Generalized Nuclear Data: A New Structure (with Supporting Infrastructure) for Handling Nuclear Data", Nuclear Data Sheets, Volume 113, Issue 12, December 2012, Pages 3145-3171, ISSN 0090-3752, http://dx.doi.org/10. 1016/j.nds.2012.11.008
# 
# 
#     Please also read this link - Our Notice and Modified BSD License
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the disclaimer below.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the disclaimer (as noted below) in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of LLNS/LLNL nor the names of its contributors may be used
#       to endorse or promote products derived from this software without specific
#       prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL LAWRENCE LIVERMORE NATIONAL SECURITY, LLC,
# THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# 
# Additional BSD Notice
# 
# 1. This notice is required to be provided under our contract with the U.S.
# Department of Energy (DOE). This work was produced at Lawrence Livermore
# National Laboratory under Contract No. DE-AC52-07NA27344 with the DOE.
# 
# 2. Neither the United States Government nor Lawrence Livermore National Security,
# LLC nor any of their employees, makes any warranty, express or implied, or assumes
# any liability or responsibility for the accuracy, completeness, or usefulness of any
# information, apparatus, product, or process disclosed, or represents that its use
# would not infringe privately-owned rights.
# 
# 3. Also, reference herein to any specific commercial products, process, or services
# by trade name, trademark, manufacturer or otherwise does not necessarily constitute
# or imply its endorsement, recommendation, or favoring by the United States Government
# or Lawrence Livermore National Security, LLC. The views and opinions of authors expressed
# herein do not necessarily state or reflect those of the United States Government or
# Lawrence Livermore National Security, LLC, and shall not be used for advertising or
# product endorsement purposes.
# 
# <<END-copyright>>

import sys, os, traceback
binDir = os.path.dirname( os.path.abspath( __file__ ) )
sys.path.insert(0, os.path.dirname( binDir ) )

from fudge.legacy.converting import endfFileToGND 

def process_args():
    # see http://docs.python.org/lib/optparse-tutorial.html
    import argparse
    parser = argparse.ArgumentParser(description="translates an ENDF file to the new GND format")
    parser.add_argument("inFile", help="input file")
    parser.add_argument("-v", action="store_true", dest="verbose", help="enable verbose output")
    parser.add_argument("-q", action="store_false", dest="verbose", help="disable verbose output")
    parser.add_argument("--skipBadData", action="store_true", default=False, help="skip bad data, rather than throw an exception, when reading an ENDF file")
    parser.add_argument("--skipCovariances", action="store_true", default=False, help="skip the covariance, if present")
    parser.add_argument("--verboseWarnings", action="store_true", default=False, help="print verbose warnings")
    parser.add_argument("--printBadNK14", action="store_true", default=False, help="print bad NK's if found")
    parser.add_argument("--continuumSpectraFix", action="store_true", default=False, help="fix continuous spectra on read, if foobar")
    parser.add_argument("--ignoreBadDate", action="store_true", default=False, help="ignore malformed ENDF dates")
    parser.add_argument("--ignoreMF10Fission", action="store_true", default=False, help="ignore fission in MF=10 (for IAEA W evaluation)")
    parser.add_argument("--traceback", action="store_true", default=False, help="print traceback on exception")
    return parser.parse_args()


args = process_args()

inFile = args.inFile
outFile = inFile+'.gnd.xml'
outCovFile = inFile+'.gndCov.xml'

try:
    results = endfFileToGND.endfFileToGND( inFile, toStdOut = args.verbose,
                                           skipBadData = args.skipBadData, doCovariances = not (args.skipCovariances),
                                           verboseWarnings = args.verboseWarnings,
                                           printBadNK14 = args.printBadNK14, continuumSpectraFix = args.continuumSpectraFix,
                                           ignoreBadDate = args.ignoreBadDate, ignoreMF10Fission = args.ignoreMF10Fission)
    e = results.get('element',None)
    x = results.get('reactionSuite',None)
    c = results.get('covarianceSuite',None)
    p = results.get('PoPs',None)
except Exception as err:
    sys.stderr.write( 'WARNING: ENDF READ HALTED BECAUSE '+str(err)+'\n' )
    if args.traceback:
        exc_type, exc_value, exc_traceback = sys.exc_info()
        traceback.print_tb(exc_traceback)
    exit()
        
f = open( outFile, 'w' )
try:
    for thing in [x,e,p]:
        if thing is not None:
            f.write( '\n'.join( thing.toXMLList(  ) ) )
except Exception as err:
    sys.stderr.write( 'WARNING: MAIN ENDF WRITE HALTED BECAUSE '+str(err)+'\n' )
    if args.traceback:
        exc_type, exc_value, exc_traceback = sys.exc_info()
        traceback.print_tb(exc_traceback)
    exit()
f.close( )
if c:
    f = open( outCovFile, 'w' )
    try:
        f.write( '\n'.join( c.toXMLList(  ) ) )
    except Exception as err:
        sys.stderr.write( 'WARNING: COVARIANCE ENDF WRITE HALTED BECAUSE '+str(err)+'\n' )
        if args.traceback:
            exc_type, exc_value, exc_traceback = sys.exc_info()
            traceback.print_tb(exc_traceback)
        exit()
    f.close()
