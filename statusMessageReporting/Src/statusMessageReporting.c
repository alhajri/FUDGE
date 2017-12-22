/*
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
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef WIN32
#define va_copy(dst, src) ((dst) = (src))
#endif

#include "statusMessageReporting.h"

#define SMR_InitialMessageSize 1024
#define SMR_IncrementMessageSize 1024

static int smrIsSetup = 0;
static char smr_mallocFailed[] = "statusMessageReporting could not allocate memory for message";
static char statusStringOk[] = "Ok",  statusStringInfo[] = "Info", 
    statusStringWarning[] = "Warning", statusStringError[] = "Error", statusStringInvalid[] = "Invalid";

static int numberOfRegisteredLibraries = 0;
static char unknownLibrary[] = "unknownID";
static char tooManyLibrary[] = "tooManyIDs";
static char invalidLibrary[] = "invalidID";
static char errnoLibrary[] = "errnoID";
static char smrLibrary[] = "statusMessageReporting";
static char *registeredLibraries[smr_maximumNumberOfRegisteredLibraries];

static statusMessageReport *smr_reportNew( void );
static int smr_reportInitialize( statusMessageReport *report );
static void smr_reportRelease( statusMessageReport *report );
static int smr_setReport( statusMessageReporting *smr, void *userInterface, char const *file, int line, char const *function, int libraryID, int code, 
    enum smr_status status, char const *fmt, va_list *args );
static int smr_setAllocationFailure( statusMessageReport *report, char const *file, int line, char const *function, char const *fmt, va_list *args );
static statusMessageReport *smr_firstReport2( statusMessageReporting const *smr );
statusMessageReport *smr_nextReport2( statusMessageReport const *report );
static void smr_write2( statusMessageReport const *report, FILE *f );
/*
============================================================
*/
int smr_setup( void ) {

    int i;

    if( smrIsSetup ) return( 0 );
    smrIsSetup = 1;
    for( i = 0; i < smr_maximumNumberOfRegisteredLibraries; ++i ) registeredLibraries[i] = NULL;
    registeredLibraries[smr_unknownID] = unknownLibrary;
    ++numberOfRegisteredLibraries;
    registeredLibraries[smr_tooManyIDs] = tooManyLibrary;
    ++numberOfRegisteredLibraries;
    registeredLibraries[smr_invalidID] = invalidLibrary;
    ++numberOfRegisteredLibraries;
    registeredLibraries[smr_errnoID] = errnoLibrary;
    ++numberOfRegisteredLibraries;
    registeredLibraries[smr_smrID] = smrLibrary;
    ++numberOfRegisteredLibraries;
    return( 1 );
}
/*
============================================================
*/
int smr_cleanup( void ) {

    int i;

    if( smrIsSetup == 0 ) return( 0 );
    for( i = smr_smrID + 1; i < numberOfRegisteredLibraries; ++i ) smr_freeMemory( (void **) &(registeredLibraries[i]) );
    numberOfRegisteredLibraries = 0;
    smrIsSetup = 0;

    return( 0 );
}
/*
============================================================
*/
int smr_registerLibrary( char const *libraryName ) {

    int i;

    if( smrIsSetup == 0 ) smr_setup( );
    for( i = 0; i < numberOfRegisteredLibraries; ++i ) {             /* Check if name is already registered. */
        if( strcmp( libraryName, registeredLibraries[i] ) == 0 ) return( i );
    }
    if( numberOfRegisteredLibraries == smr_maximumNumberOfRegisteredLibraries ) return( smr_tooManyIDs );
    if( ( registeredLibraries[numberOfRegisteredLibraries] = strdup( libraryName ) ) == NULL ) return( -1 );
    ++numberOfRegisteredLibraries;
    return( numberOfRegisteredLibraries - 1 );
}
/*
============================================================
*/
int smr_numberOfRegisteredLibraries( void ) {

    return( numberOfRegisteredLibraries );
}
/*
============================================================
*/
char const *smr_getRegisteredLibrarysName( int ID ) {

    if( ( ID < 0 ) || ( ID >= smr_maximumNumberOfRegisteredLibraries ) ) return( NULL );
    return( registeredLibraries[ID] );
}
/*
============================================================
*/
statusMessageReporting *smr_new( statusMessageReporting *smr, enum smr_status verbosity ) {

    statusMessageReporting *new_SMR;

    if( ( new_SMR = (statusMessageReporting *) smr_malloc2( smr, sizeof( statusMessageReporting ), 0, "new_SMR" ) ) == NULL ) return( NULL );
    smr_initialize( new_SMR, verbosity );
    return( new_SMR );
}
/*
============================================================
*/
int smr_initialize( statusMessageReporting *smr, enum smr_status verbosity ) {

    if( smr == NULL ) return( 0 );
    smr->verbosity = verbosity;
    smr_reportInitialize( &(smr->report) );
    return( 0 );
}
/*
============================================================
*/
statusMessageReporting *smr_clone( statusMessageReporting const *smr ) {

    if( smr == NULL ) return( NULL );
    return( smr_new( NULL, smr->verbosity ) );
}
/*
============================================================
*/
void smr_release( statusMessageReporting *smr ) {

    statusMessageReport *current, *next, *first = smr_firstReport2( smr );

    if( smr == NULL ) return;
    for( current = first; current != NULL; current = next ) {
        next = smr_nextReport2( current );
        smr_reportRelease( current );
        if( current != first ) smr_freeMemory( (void **) &current );
    }
    smr_initialize( smr, smr->verbosity );
}
/*
============================================================
*/
void *smr_free( statusMessageReporting **smr ) {

    if( smr == NULL ) return( NULL );
    if( *smr != NULL ) {
        smr_release( *smr );
        smr_freeMemory( (void **) smr );
    }
    return( *smr );
}
/*
============================================================
*/
static statusMessageReport *smr_reportNew( void ) {

    statusMessageReport *report;

    if( ( report = (statusMessageReport *) smr_malloc2( NULL, sizeof( statusMessageReport ), 0, "report" ) ) == NULL ) return( NULL );
    smr_reportInitialize( report );
    return( report );
}
/*
============================================================
*/
static int smr_reportInitialize( statusMessageReport *report ) {

    report->next = NULL;
    report->status = smr_status_Ok;
    report->libraryID = smr_unknownID;
    report->code = smr_codeNULL;
    report->line = -1;
    report->fileName[0] = 0;
    report->function[0] = 0;
    report->message = NULL;
    return( 0 );
}
/*
============================================================
*/
static void smr_reportRelease( statusMessageReport *report ) {

    if( report->message != NULL ) {
        if( report->message != smr_mallocFailed ) smr_freeMemory( (void **) &(report->message) );
    }
    smr_reportInitialize( report );
}
/*
============================================================
*/
static int smr_setReport( statusMessageReporting *smr, void *userInterface, char const *file, int line, char const *function, int libraryID, int code, 
    enum smr_status status, char const *fmt, va_list *args ) {

    char *userMsg;
    statusMessageReport *report, *next;

    if( smr == NULL ) return( 0 );
    if( (int) status < (int) smr->verbosity ) return( 0 );
    if( status == smr_status_Ok ) return( 0 );
	if( smr->report.status == smr_status_Ok ) {
		report = &smr->report; }
	else {
    	if( ( report = smr_reportNew( ) ) == NULL ) return( smr_setAllocationFailure( NULL, file, line, function, fmt, args ) );
    	for( next = smr_firstReport2( smr ); next->next != NULL; next = next->next );
    	next->next = report;
	}
    report->status = status;
    if( ( libraryID < 0 ) || ( libraryID >= numberOfRegisteredLibraries ) ) libraryID = smr_invalidID;
    report->libraryID = libraryID;
    report->code = code;
    report->line = line;
    if( file != NULL ) strncpy( report->fileName, file, smr_maximumFileNameSize );
    report->fileName[smr_maximumFileNameSize] = 0;
    if( function != NULL ) strncpy( report->function, function, smr_maximumFileNameSize );
    report->function[smr_maximumFileNameSize] = 0;

    if( ( report->message = smr_vallocateFormatMessage( fmt, args ) ) == NULL ) return( smr_setAllocationFailure( report, file, line, function, fmt, args ) );
    if( userInterface != NULL ) {
        if( ( userMsg = (*(smr_userInterface *) userInterface)( (void *) userInterface ) ) != NULL ) {
            int userSize = (int) strlen( userMsg );
            if( ( report->message = (char *) smr_realloc2( NULL, report->message, strlen( report->message ) + userSize + 2, "report->message" ) ) == NULL ) {
                free( userMsg );
                return( smr_setAllocationFailure( report, file, line, function, fmt, args ) );
            }
            strcat( report->message, userMsg );
            free( userMsg );
        }
    }
    return( 0 );
}
/*
============================================================
*/
static int smr_setAllocationFailure( statusMessageReport *report, char const *file, int line, char const *function, char const *fmt, va_list *args ) {

    vfprintf( stderr, fmt, *args );
    va_end( *args );
    fprintf( stderr, "\n    At line %d of %s in function %s\n", line, file, function );
    if( report != NULL ) {
        report->status = smr_status_Error;
        report->message = (char *) smr_mallocFailed;
        return( 1 );
    }
    return( -1 );
}
/*
============================================================
*/
int smr_setReportInfo( statusMessageReporting *smr, void *userInterface, char const *file, int line, char const *function, int libraryID, int code, char const *fmt, ... ) {

    int status;
    va_list args;

    va_start( args, fmt );
    status = smr_setReport( smr, userInterface, file, line, function, libraryID, code, smr_status_Info, fmt, &args );
    va_end( args );
    return( status );
}
/*
============================================================
*/
int smr_vsetReportInfo( statusMessageReporting *smr, void *userInterface, char const *file, int line, char const *function, int libraryID, int code, char const *fmt, va_list *args ) {

    return( smr_setReport( smr, userInterface, file, line, function, libraryID, code, smr_status_Info, fmt, args ) );
}
/*
============================================================
*/
int smr_setReportWarning( statusMessageReporting *smr, void *userInterface, char const *file, int line, char const *function, int libraryID, int code, char const *fmt, ... ) {

    int status;
    va_list args;

    va_start( args, fmt );
    status = smr_setReport( smr, userInterface, file, line, function, libraryID, code, smr_status_Warning, fmt, &args );
    va_end( args );
    return( status );
}
/*
============================================================
*/
int smr_vsetReportWarning( statusMessageReporting *smr, void *userInterface, char const *file, int line, char const *function, int libraryID, int code, char const *fmt, va_list *args ) {

    return( smr_setReport( smr, userInterface, file, line, function, libraryID, code, smr_status_Warning, fmt, args ) );
}
/*
============================================================
*/
int smr_setReportError( statusMessageReporting *smr, void *userInterface, char const *file, int line, char const *function, int libraryID, int code, char const *fmt, ... ) {

    int status;
    va_list args;

    va_start( args, fmt );
    status = smr_setReport( smr, userInterface, file, line, function, libraryID, code, smr_status_Error, fmt, &args );
    va_end( args );
    return( status );
}
/*
============================================================
*/
int smr_vsetReportError( statusMessageReporting *smr, void *userInterface, char const *file, int line, char const *function, int libraryID, int code, char const *fmt, va_list *args ) {

    return( smr_setReport( smr, userInterface, file, line, function, libraryID, code, smr_status_Error, fmt, args ) );
}
/*
============================================================
*/
enum smr_status smr_highestStatus( statusMessageReporting const *smr ) {

    enum smr_status status = smr_status_Ok;
    statusMessageReport const *report;

    if( smr == NULL ) return( smr_status_Ok );
    for( report = smr_firstReport( smr ); report != NULL; report = smr_nextReport( report ) ) if( report->status > status ) status = report->status;
    return( status );
}
/*
============================================================
*/
int smr_isOk( statusMessageReporting const *smr ) { 

    return( smr_highestStatus( smr ) == smr_status_Ok );
}
/*
============================================================
*/
int smr_isInfo( statusMessageReporting const *smr ) { 

    return( smr_highestStatus( smr ) == smr_status_Info );
}
/*
============================================================
*/
int smr_isWarning( statusMessageReporting const *smr ) { 

    return( smr_highestStatus( smr ) == smr_status_Warning );
}
/*
============================================================
*/
int smr_isError( statusMessageReporting const *smr ) { 

    return( smr_highestStatus( smr ) == smr_status_Error );
}
/*
============================================================
*/
int smr_isWarningOrError( statusMessageReporting const *smr ) { 

    enum smr_status status = smr_highestStatus( smr );

    return( ( status == smr_status_Warning ) || ( status == smr_status_Error ) );
}
/*
============================================================
*/
int smr_isReportOk( statusMessageReport const *report ) { 

    if( report == NULL ) return( 0 );
    return( report->status == smr_status_Ok );
}
/*
============================================================
*/
int smr_isReportInfo( statusMessageReport const *report ) { 

    if( report == NULL ) return( 0 );
    return( report->status == smr_status_Info );
}
/*
============================================================
*/
int smr_isReportWarning( statusMessageReport const *report ) { 

    if( report == NULL ) return( 0 );
    return( report->status == smr_status_Warning );
}
/*
============================================================
*/
int smr_isReportError( statusMessageReport const *report ) { 

    if( report == NULL ) return( 0 );
    return( report->status == smr_status_Error );
}
/*
============================================================
*/
int smr_isReportWarningOrError( statusMessageReport const *report ) { 

    if( report == NULL ) return( 0 );
    return( ( report->status == smr_status_Warning ) || ( report->status == smr_status_Error ) );
}
/*
============================================================
*/
int smr_numberOfReports( statusMessageReporting const *smr ) {

    int n = 0;
    statusMessageReport const *report;

    if( smr == NULL ) return( 0 );
    if( smr->report.status == smr_status_Ok ) return( 0 );
    for( report = smr_firstReport( smr ); report != NULL; report = smr_nextReport( report ) ) ++n;
    return( n );
}
/*
============================================================
*/
statusMessageReport const *smr_firstReport( statusMessageReporting const *smr ) {

    return( (statusMessageReport const *) smr_firstReport2( smr ) );
}
/*
============================================================
*/
static statusMessageReport *smr_firstReport2( statusMessageReporting const *smr ) {

    if( smr == NULL ) return( NULL );
    if( smr->report.status == smr_status_Ok ) return( NULL );
    return( &(((statusMessageReporting *) smr)->report) );
}
/*
============================================================
*/
statusMessageReport const *smr_nextReport( statusMessageReport const *report ) {

    return( smr_nextReport2( report ) );
}
/*
============================================================
*/
statusMessageReport *smr_nextReport2( statusMessageReport const *report ) {

    if( report == NULL ) return( NULL );
    return( report->next );
}
/*
============================================================
*/
enum smr_status smr_getVerbosity( statusMessageReporting const *smr ) {

    if( smr == NULL ) return( smr_status_Ok );
    return( smr->verbosity );
}
/*
============================================================
*/
int smr_getLibraryID( statusMessageReport const *report ) {

    if( report == NULL ) return( 0 );
    return( report->libraryID );
}
/*
============================================================
*/
int smr_getCode( statusMessageReport const *report ) {

    if( report == NULL ) return( -1 );
    return( report->code );
}
/*
============================================================
*/
int smr_getLine( statusMessageReport const *report ) {

    if( report == NULL ) return( -1 );
    return( report->line );
}
/*
============================================================
*/
char const *smr_getFile( statusMessageReport const *report ) {

    if( report == NULL ) return( NULL );
    return( report->fileName );
}
/*
============================================================
*/
char const *smr_getFunction( statusMessageReport const *report ) {

    if( report == NULL ) return( NULL );
    return( report->function );
}
/*
============================================================
*/
char const *smr_getMessage( statusMessageReport const *report ) {

    if( report == NULL ) return( NULL );
    return( report->message );
}
/*
============================================================
*/
char *smr_copyMessage( statusMessageReport const *report ) {

    if( report == NULL ) return( NULL );
    if( report->status == smr_status_Ok ) return( NULL );
    return( smr_allocateFormatMessage( report->message ) );
}
/*
============================================================
*/
char *smr_copyFullMessage( statusMessageReport const *report ) {

    if( report == NULL ) return( NULL );
    if( report->status == smr_status_Ok ) return( NULL );
    return( smr_allocateFormatMessage( "%s\n    At line %d of %s in function %s", report->message, report->line, report->fileName, report->function ) );
}
/*
============================================================
*/
void smr_print( statusMessageReporting *smr, int clear ) {

    smr_write( smr, stdout, clear );
}
/*
============================================================
*/
void smr_write( statusMessageReporting *smr, FILE *f, int clear ) {

    if( smr == NULL ) return;
    fprintf( f, "======= Status message reports =======\n" );
    smr_write2( smr_firstReport( smr ), f );
    if( clear ) smr_release( smr );
}
/*
============================================================
*/
static void smr_write2( statusMessageReport const *report, FILE *f ) {

    if( report == NULL ) return;

    smr_write2( smr_nextReport( report ), f );
    smr_reportWrite( report, f );
}
/*
============================================================
*/
void smr_reportPrint( statusMessageReport const *report ) {

    smr_reportWrite( report, stdout );
}
/*
============================================================
*/
void smr_reportWrite( statusMessageReport const *report, FILE *f ) {

    if( report->message != NULL ) fprintf( f, "%s\n    At line %d of %s in function %s\n", report->message, report->line, report->fileName, report->function );
}
/*
============================================================
*/
char const *smr_statusToString( enum smr_status status ) {

    switch( status ) {
    case smr_status_Ok : return( statusStringOk );
    case smr_status_Info : return( statusStringInfo );
    case smr_status_Warning : return( statusStringWarning );
    case smr_status_Error : return( statusStringError );
    }
    return( statusStringInvalid );
}
/*
============================================================
*/
char *smr_allocateFormatMessage( char const *fmt, ... ) {

    char *s;
    va_list args;

    va_start( args, fmt );
    s = smr_vallocateFormatMessage( fmt, &args );
    va_end( args );
    return( s );
}
/*
============================================================
*/
char *smr_vallocateFormatMessage( char const *fmt, va_list *args ) {

    int n, size = SMR_InitialMessageSize;
    char buffer[SMR_InitialMessageSize], *message = buffer;
    va_list args_;

    while( 1 ) {
        va_copy( args_, *args );
        n = vsnprintf( message, size, fmt, args_ );
        va_end( args_ );
        if( ( n > -1 ) && ( n < size ) ) break;
        if( n > -1 ) {      /* glibc 2.1 */
            size = n + 3; }
        else {              /* glibc 2.0 */
            size *= 2;
        }
        if( message == buffer ) message = NULL;
        if( ( message = (char *) realloc( message, size ) ) == NULL ) return( NULL );
    }
    if( message == buffer ) {
        if( ( message = (char *) malloc( n + 1 ) ) == NULL ) return( NULL );
        strcpy( message, buffer ); }
    else {
        if( ( message = (char *) realloc( message, n + 1 ) ) == NULL ) return( NULL );
    }
    return( message );
}
/*
============================================================
*/
void *smr_malloc( statusMessageReporting *smr, size_t size, int zero, char const *forItem, char const *file, int line, char const *function ) {

    void *p = smr_realloc( smr, NULL, size, forItem, file, line, function );
    size_t i;
    char *c;
    long long *l;

    if( ( p != NULL ) && zero ) {
        for( i = 0, l = (long long *) p; i < size / sizeof( long long ); i++, l++ ) *l = 0;
        for( i *= sizeof( long long ), c = (char *) l; i < size; i++, c++ ) *c = 0;
    }

    return( p );
}
/*
============================================================
*/
void *smr_realloc( statusMessageReporting *smr, void *pOld, size_t size, char const *forItem, char const *file, int line, char const *function ) {

    void *p = realloc( pOld, size );

    if( ( p == NULL ) && ( smr != NULL ) ) {
        smr_setReportError( smr, NULL, file, line, function, smr_smrID, smr_codeMemoryAllocating, " smr_realloc: failed to realloc size = %z for variable %s\n", size, forItem );
    }
    return( p );
}
/*
============================================================
*/
void *smr_freeMemory( void **p ) {

    if( p == NULL ) return( NULL );
    if( *p != NULL ) {
        free( *p );
        *p = NULL;
    }
    return( *p );
}
/*
============================================================
*/
char *smr_allocateCopyString( statusMessageReporting *smr, char const *s, char const *forItem, char const *file, int line, char const *function ) {
/*
*   User must free returned string.
*/
    char *c = strdup( s );

    if( c == NULL ) smr_setReportError( smr, NULL, file, line, function, smr_smrID, smr_codeMemoryAllocating, " smr_allocateCopyString: strdup failed for strlen( s ) = %z for variable %s",
    		strlen( s ), forItem );
    return( c );
}
/*
============================================================
*/
char *smr_allocateCopyStringN( statusMessageReporting *smr, char const *s, size_t n, char const *forItem, char const *file, int line, char const *function ) {
/*
*   User must free returned string.
*/
    size_t l = strlen( s );
    char *c;

    if( l > n ) l = n;
    if( ( c = (char *) smr_malloc( smr, l + 1, 0, forItem, file, line, function ) ) != NULL ) {
        strncpy( c, s, n );
        c[l] = 0;
    }
/*
    c = strndup( s, l );        # Not standard on enough systems.
    if( c != NULL ) {
        c[l] = 0; }
    else {
         smr_setReportError( smr, NULL, file, line, function, smr_smrID, smr_codeMemoryAllocating, " smr_allocateCopyStringN: strndup failed for strlen( s ) = %z for variable %s",
            strlen( s ), forItem );
    }
*/
    return( c );
}
