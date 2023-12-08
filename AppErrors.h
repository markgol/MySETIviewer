#pragma once
// Application standardized error numbers for functions:
//      1 - success
//      0 - parameter or image header problem
//     -1 memory allocation failure
//     -2 open file failure
//     -3 file read failure
//     -4 incorect file type
//     -5 file size mismatch (filesize does not match expected filesize)
//     -6 not yet implemented

#define APP_SUCCESS	1
#define APPERR_PARAMETER 0
#define APPERR_MEMALLOC -1
#define APPERR_FILEOPEN -2
#define APPERR_FILEREAD -3
#define APPERR_FILETYPE -4
#define APPERR_FILESIZE -5
#define APPERR_NYI -6
