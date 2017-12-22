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

"""
This module adds the method toACE to the classes in the fudge.gnd.productData.distributions.angularEnergy module.
"""

from xData import standards as standardsModule
from xData import axes as axesModule
from xData import XYs as XYsModule
from xData import multiD_XYs as multiD_XYsModule

from fudge.gnd.productData.distributions import angularEnergy as angularEnergyModule

class angularFor_angularEnergy( multiD_XYsModule.XYs2d ) :

    def __init__( self, angularEnergy ) :

        self.__productFrame = angularEnergy.productFrame
        axes = axesModule.defaultAxes( 3 )
        axes2d[2] = angularEnergy.axes[2]
        axes2d[1] = angularEnergy.axes[1]
        axes2d[0] = axesModule.axis( "P(mu|energy_in)", 0, "" )
        axes1d = XYsModule.XYs1d.defaultAxes( )
        axes1d[0] = axes2d[0]
        axes1d[1] = axes2d[1]
        multiD_XYsModule.XYs2d.__init__( self, axes = axes2d )

        for P_EpGivenMu in angularEnergy :
            P_mu = [ [ xys.value, xys.integrate( ) ] for xys in P_EpGivenMu ]
            self.append( XYsModule.XYs1d( data = P_mu, axes = axes1d, accuracy = 1e-3, value = P_EpGivenMu.value ) )

    @property
    def productFrame( self ) :

        return( self.__productFrame )

    def isIsotropic( self ) :

        return( False )

def toACE( self, label, offset, weight, **kwargs ) :

    header = [ 0, 67, offset + len( weight ) + 4 ] + weight
    e_inFactor, e_outFactor = self.domainUnitConversionFactor( 'MeV' ), self.axes[2].unitConversionFactor( 'MeV' )

    INTE = -1
    interpolation = self.interpolation
    if( interpolation == standardsModule.interpolation.flatToken ) :
        INTE = 1
    elif( interpolation == standardsModule.interpolation.linearToken ) :
        INTE = 2
    if( INTE == -1 ) : raise Exception( 'Interpolation "%s" not supported for incident energy' % interpolation )

    INTMU = -1
    interpolation = self[0].interpolation
    if( interpolation == standardsModule.interpolation.flatToken ) :
        INTMU = 1
    elif( interpolation == standardsModule.interpolation.linlinToken ) :
        INTMU = 2
    if( INTMU == -1 ) : raise Exception( 'Interpolation "%s" not supported for outgoing energy' % interpolation )

    INTEP = -1
    interpolation = self[0][0].interpolation
    if( interpolation == standardsModule.interpolation.flatToken ) :
        INTEP = 1
    elif( interpolation == standardsModule.interpolation.linearToken ) :
        INTEP = 2
    if( INTEP == -1 ) : raise Exception( 'Interpolation "%s" not supported for outgoing energy' % interpolation )

    NE, e_ins, Ls, MuData = len( self ), [], [], []
    offset += len( header ) + 3 + 1 + 2 * NE + 1        # header length plus NR, NE, Es, Ls, (1-based).
    for w_xys in self :
        e_ins.append( w_xys.value * e_inFactor )
        Ls.append( offset + len( MuData ) )

        NMU, XMU, LMU, EpPData = len( w_xys ), [], [], []
        offset_LC = Ls[-1] + 1 + 2 * NMU
        for i1, _xys in enumerate( w_xys ) :
            XMU.append( _xys.value )
            LMU.append( offset_LC + len( EpPData ) )

            xys = _xys.normalize( )
            cdfOfEps = xys.runningIntegral( )
            cdfOfEps[-1] = 1.
            Eps, pdfOfEps = [], []
            for Ep1, pdf1 in xys :
                Eps.append( Ep1 * e_outFactor )
                pdfOfEps.append( pdf1 / e_outFactor )
            EpPData += [ INTEP, len( Eps ) ] + Eps + pdfOfEps + cdfOfEps
        MuData += [ INTMU, NMU ] + XMU + LMU + EpPData

    return( header + [ 1, NE, INTE, NE ] + e_ins + Ls + MuData )

angularEnergyModule.XYs3d.toACE = toACE
