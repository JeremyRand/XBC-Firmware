/* Sleep for a number of microseconds
   This no longer works with the itimer package, and only works under Linux
   */

#include <sys/types.h>
#include <sys/time.h>
#include <stddef.h>
#include <errno.h>
#include <stdio.h>
#include "sleep_us.h"

/* -------------------------------------------------------------------------
   This code from Steve's Adv Unix Prog, page 705
   This implements a simple delay using select
*/

// WARNING: THIS DEPENDS ON LINUX-SPECIFIC BEHAVIOR FOR SELECT
void
sleep_us (unsigned int nusecs)
{
    struct timeval tval;
    tval.tv_sec = nusecs / 1000000;
    tval.tv_usec = nusecs % 1000000;
    do {
      select (0, NULL, NULL, NULL, &tval);
    } while (timerisset(&tval));
    return;
}


/* This is from Randy Sargent, 3/4/00.  */
/* This was called utime(), but that's a UNIX system call... */
long long ustime(void)
{
    struct timeval tv;
    struct timezone tz;

    gettimeofday(&tv, &tz);

    return tv.tv_sec*((long long)1000000) + tv.tv_usec;
}

// Sleep until endtime.  Not completely accurate, user beware
// WARNING: THIS DEPENDS ON LINUX-SPECIFIC BEHAVIOR FOR SELECT AND
//    ONLY WORKS FOR TIMES OF < 71 minutes
void
sleep_until_us (long long endtime)
{
    struct timeval tval;
    long nusecs= (long) (endtime - ustime());

    if(nusecs<0) {		/* already expired, return now */
	return;
    }

    tval.tv_sec = nusecs / 1000000;
    tval.tv_usec = nusecs % 1000000;
    do {
      select (0, NULL, NULL, NULL, &tval);
    } while (timerisset(&tval));
    return;
}

// Subracts a-b, writes difference in microseconds to result, returns
// 0 on success, non-zero on failure
int usec_diff(struct timeval *a, struct timeval *b, long *result)
{
  long long a_usec, b_usec, diff;
  
  if(a==NULL || b==NULL || result==NULL) {	/* failure */
    return(EINVAL);
  }

  a_usec= ((a->tv_sec)*((long long)1000000)) + (a->tv_usec);
  b_usec= ((b->tv_sec)*((long long)1000000)) + (b->tv_usec);
  diff=a_usec-b_usec;
  *result=(long)diff;
  if((long long)((long)diff) != diff) {	/* overflow? */
    return(EOVERFLOW);
  }
  return(0);
}  

/* For emacs to interpret formatting uniformly despite dotfile differences:
 * Local variables:
 *  comment-column: 40
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 */
