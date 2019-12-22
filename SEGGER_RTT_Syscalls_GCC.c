/*********************************************************************
File    : SEGGER_RTT_Syscalls_GCC.c
Purpose : Low-level functions for using printf() via RTT in GCC.
          To use RTT for printf output, include this file in your 
          application.
Revision: $Rev: 4351 $
----------------------------------------------------------------------
*/
#if (defined __GNUC__) && !(defined __SES_ARM) && !(defined __CROSSWORKS_ARM)

#include <reent.h>  // required for _write_r
#include "SEGGER_RTT.h"


/*********************************************************************
*
*       Types
*
**********************************************************************
*/
//
// If necessary define the _reent struct 
// to match the one passed by the used standard library.
//
struct _reent;

/*********************************************************************
*
*       Function prototypes
*
**********************************************************************
*/
int _write(int file, char *ptr, int len);
_ssize_t _write_r(struct _reent *r, int file, const void *ptr, size_t len);

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*       _write()
*
* Function description
*   Low-level write function.
*   libc subroutines will use this system routine for output to all files,
*   including stdout.
*   Write data via RTT.
*/
int _write(int file, char *ptr, int len) {
  (void) file;  /* Not used, avoid warning */
  SEGGER_RTT_Write(0, ptr, len);
  return len;
}

/*********************************************************************
*
*       _write_r()
*
* Function description
*   Low-level reentrant write function.
*   libc subroutines will use this system routine for output to all files,
*   including stdout.
*   Write data via RTT.
*/
_ssize_t _write_r(struct _reent *r, int file, const void *ptr, size_t len) {
  (void) file;  /* Not used, avoid warning */
  (void) r;     /* Not used, avoid warning */
  SEGGER_RTT_Write(0, ptr, len);
  return len;
}

#endif
/****** End Of File *************************************************/
