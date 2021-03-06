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
This module contains the nucleus classes.
"""

continuumID = 1000000
sumID = continuumID + 1

from .. import misc as miscModule

from ..quantities import nuclearEnergyLevel as nuclearEnergyLevelModule

from . import particle as particleModule

class alias( particleModule.alias ) :

    moniker = 'nucleusAlias'

    @property
    def chemicalElement( self ) :

        return( self.__particle.chemicalElement )

    @property
    def Z( self ) :

        return( self.__particle.Z )

    @property
    def A( self ) :

        return( self.__particle.A )

    @property
    def index( self ) :

        return( self.__particle.index )

    @property
    def intIndex( self ) :

        return( self.__particle.intIndex )

    @property
    def energy( self ) :

        return( self.__particle.energy )

class particle( particleModule.particle ) :

    moniker = 'nucleus'
    alias = alias

    def __init__( self, id, index ) :

        from ..groups import chemicalElement as chemicalElementModule
        from ..groups import isotope as isotopeModule

        particleModule.particle.__init__( self, id )

        isNucleus, chemicalElement, A, _index, anti, qualifier = chemicalElementAAndLevelIDsFromNuclearLevelID( id )
        if( not( isNucleus ) ) : raise ValueError( 'Invalid nucleus id = "%s"' % id )

        if( isinstance( index, int ) ) : index = str( index )
        intIndex = checkIndex( index )
        if( index != _index ) : raise ValueError( 'id = "%s" does not agree with index = "%s"' % 
                ( miscModule.toLimitedString( id ), miscModule.toLimitedString( index ) ) )

        self.__chemicalElement = chemicalElement
        self.__Z = chemicalElementModule.ZFromSymbol[chemicalElement]
        self.__A = A
        self.__intA = isotopeModule.checkA( A )
        self.__index = index
        self.__intIndex = intIndex
        self.__energy = self.addSuite( nuclearEnergyLevelModule )

    def __eq__( self, other ) :

        from ..groups import isotope as isotopeModule

        if(   isinstance( other, particle ) ) :
            return( self.id == other.id )
        elif( isinstance( other, isotopeModule.suite ) ) :
            _particle = other.particle
            return( self.id == _particle.id )
        else :
            return( False )

    @property
    def A( self ) :

        return( self.__A )

    @property
    def intA( self ) :

        return( self.__intA )

    @property
    def chemicalElement( self ) :

        return( self.__chemicalElement )

    @property
    def index( self ) :

        return( self.__index )

    @property
    def intIndex( self ) :

        return( self.__intIndex )

    @property
    def energy( self ) :

        return( self.__energy )

    @property
    def Z( self ) :

        return( self.__Z )

    def convertUnits( self, unitMap ) :

        particleModule.particle.convertUnits( self, unitMap )
        self.__energy.convertUnits( unitMap )

    def copy( self ) :

        _particle = particle( self.id, self.index )
        self.__copyStandardQuantities( _particle )
        for item in self.__energy : _particle.energy.add( item.copy( ) )
        return( _particle )

    def extraXMLAttributes( self ) :

        return( ' index="%s"' % self.index )

    def extraXMLElements( self, indent, **kwargs ) :

        return( self.energy.toXMLList( indent, **kwargs ) )

    def parseExtraXMLElement( self, element, xPath, linkData ) :

        if( element.tag == nuclearEnergyLevelModule.suite.moniker ) :
            nuclearEnergyLevelModule.suite.parseXMLNode( self.energy, element, xPath, linkData )
            return( True )

        return( False )

    def sortCompare( self, other ) :

        if( not( isinstance( other, particle ) ) ) : raise TypeError( 'Invalid other.' )
        return( self.intIndex - other.intIndex )

def nucleusNameFromNucleusNameAndIndex( isotopeName, index ) :

    nucleusName = isotopeName[0].lower( ) + isotopeName[1:]
    _index = checkIndex( index )
    if( isinstance( _index, int ) ) : return( "%s_e%s" % ( nucleusName, index ) )
    return( "%s_%s" % ( nucleusName, index ) )

def levelNameFromIsotopeNameAndIndex( isotopeName, index ) :

    _index = checkIndex( index )
    if( isinstance( _index, int ) ) : return( "%s_e%s" % ( isotopeName, index ) )
    return( "%s_%s" % ( isotopeName, index ) )

def chemicalElementAAndLevelIDsFromNuclearLevelID( id, qualifierAllowed = False ) :

    from ..groups import isotope as isotopeModule

    baseID, anti, qualifier = miscModule.baseAntiQualifierFromID( id, qualifierAllowed = qualifierAllowed )

    isotopeID, sep, levelID = baseID.rpartition( '_' )
    if( sep == '' ) : raise ValueError( 'Missing level id separator: %s' % miscModule.toLimitedString( id ) )
    if( levelID == '' ) : raise ValueError( 'Missing level id: %s' % miscModule.toLimitedString( id ) )
    if( levelID[0] == 'e' ) :
        checkIndex( levelID[1:] )
        levelID = levelID[1:]
    elif( not( checkIndex( levelID ) ) ) :
        raise ValueError( 'Invalid level id: %s' % miscModule.toLimitedString( id ) )

    if( len( isotopeID ) < 2 ) : raise ValueError( 'Invalid isotope id = "%s"' % isotopeID )
    isotopeIDUpper = isotopeID[0].upper( ) + isotopeID[1:]
    chemicalElementID, A, dummy, dummy = isotopeModule.chemicalElementAndAIDsFromIsotopeID( isotopeIDUpper )

    isNucleus = isotopeID != isotopeIDUpper
    return( isNucleus, chemicalElementID, A, levelID, anti, qualifier )

def checkIndex( index ) :

    if( not( isinstance( index, str ) ) ) : raise TypeError( 'index attribute not str object.' )
    if( index == 'c' ) :
        _index = continuumID
    elif( index == 's' ) :
        _index = sumID
    else :
        try :
            _index = int( index )
            if( _index < 0 ) : ValueError( 'Negative level index id = %s' % index )
        except :
            raise ValueError( 'index attribute not str of positive integer, "c" or "s": %s' % miscModule.toLimitedString( index ) )
    return( _index )
