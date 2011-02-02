//==========================================================================
//
//      io.cxx
//
//      Fileio IO operations
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):           nickg
// Contributors:        nickg
// Date:                2000-05-25
// Purpose:             Fileio IO operations
// Description:         These are the functions that operate on open files,
//                      such as read(), write(), fstat() etc.
//              
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <pkgconf/io_fileio.h>

#include <cyg/kernel/ktypes.h>         // base kernel types
#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros

#include <stdarg.h>                     // for fcntl()

#include "fio.h"                       // Private header

#include <cyg/kernel/mutex.hxx>        // mutex definitions

//==========================================================================
// File object locking

#define LOCK_FILE( fp ) cyg_file_lock( fp, fp->f_syncmode )

#define UNLOCK_FILE( fp ) cyg_file_unlock( fp, fp->f_syncmode )

//==========================================================================
// Read from or write to a file
// This is a common routine for doing both read and write
// operations. The direction argument controls slight the differences
// between them.

static ssize_t readwrite( int fd, void *buf, size_t len, int direction )
{
    FILEIO_ENTRY();

    CYG_CANCELLATION_POINT;
    
    ssize_t cnt;
    int ret;
    cyg_file *fp;
    
    if( len > SSIZE_MAX )
        FILEIO_RETURN(EINVAL);
    
    fp = cyg_fp_get( fd );

    if( fp == NULL )
        FILEIO_RETURN(EBADF);

    if( (fp->f_flag & direction) == 0 )
    {
        cyg_fp_free( fp );
        FILEIO_RETURN(EBADF);        
    }

    cyg_uio uio;
    cyg_iovec iov;
    cyg_fileop_readwrite *op;
    
    iov.iov_base        = buf;
    iov.iov_len         = len;
    uio.uio_iov         = &iov;
    uio.uio_iovcnt      = 1;
    uio.uio_resid       = len;
    uio.uio_segflg      = UIO_USERSPACE;

    cnt = len;

    if( direction == O_RDONLY )
        uio.uio_rw = UIO_READ, op = fp->f_ops->fo_read;
    else
        uio.uio_rw = UIO_WRITE, op = fp->f_ops->fo_write;

        
    LOCK_FILE( fp );
    
    ret = op( fp, &uio );
    
    UNLOCK_FILE( fp );
    
    cnt -= uio.uio_resid;

    cyg_fp_free( fp );

    CYG_CANCELLATION_POINT;

    if( ret != 0 )
        FILEIO_RETURN(ret);
 
    FILEIO_RETURN_VALUE(cnt);
}

//==========================================================================
// Read from file

__externC ssize_t read( int fd, void *buf, size_t len )
{
    return readwrite( fd, buf, len, O_RDONLY );
}

//==========================================================================
// Write to a file

__externC ssize_t write( int fd, const void *buf, size_t len )
{
    return readwrite( fd, (void *)buf, len, O_WRONLY );
}

//==========================================================================
// Close a file

__externC int close( int fd )
{
    FILEIO_ENTRY();

    CYG_CANCELLATION_POINT;
    
    int ret;
    cyg_file *fp;
    
    fp = cyg_fp_get( fd );

    if( fp == NULL )
        FILEIO_RETURN(EBADF);

    cyg_fp_free( fp );
    
    // The file's fo_close entry may be called as a side
    // effect of this operation...
    ret = cyg_fd_free( fd );

    CYG_CANCELLATION_POINT;
 
    FILEIO_RETURN(ret);
}

//==========================================================================
// Seek a file

__externC off_t lseek( int fd, off_t pos, int whence )
{
    FILEIO_ENTRY();

    int ret;
    cyg_file *fp;
    
    fp = cyg_fp_get( fd );

    if( fp == NULL )
        FILEIO_RETURN(EBADF);

    LOCK_FILE( fp );
    
    ret = fp->f_ops->fo_lseek( fp, &pos, whence );

    UNLOCK_FILE( fp );
    
    cyg_fp_free( fp );

    if( ret != 0 )
        FILEIO_RETURN(ret);
    
    FILEIO_RETURN_VALUE(pos);
}

//==========================================================================
// ioctl

__externC int ioctl( int fd, CYG_ADDRWORD com, CYG_ADDRWORD data )
{
    FILEIO_ENTRY();

    int ret;
    cyg_file *fp;
    
    fp = cyg_fp_get( fd );

    if( fp == NULL )
        FILEIO_RETURN(EBADF);

    LOCK_FILE( fp );
    
    ret = fp->f_ops->fo_ioctl( fp, com, data );

    UNLOCK_FILE( fp );
    
    cyg_fp_free( fp );

    FILEIO_RETURN(ret);
}

//==========================================================================
// fsync

__externC int fsync( int fd )
{
    FILEIO_ENTRY();

    CYG_CANCELLATION_POINT;
    
    int ret;
    cyg_file *fp;
    
    fp = cyg_fp_get( fd );

    if( fp == NULL )
        FILEIO_RETURN(EBADF);

    LOCK_FILE( fp );
    
    ret = fp->f_ops->fo_fsync( fp, CYG_FSYNC );

    UNLOCK_FILE( fp );
    
    cyg_fp_free( fp );

    CYG_CANCELLATION_POINT;
    
    FILEIO_RETURN(ret);
}

//==========================================================================
// fdatasync()

__externC int fdatasync( int fd )
{
    FILEIO_ENTRY();

    CYG_CANCELLATION_POINT;
    
    int ret;
    cyg_file *fp;
    
    fp = cyg_fp_get( fd );

    if( fp == NULL )
        FILEIO_RETURN(EBADF);

    LOCK_FILE( fp );
    
    ret = fp->f_ops->fo_fsync( fp, CYG_FDATASYNC );

    UNLOCK_FILE( fp );
    
    cyg_fp_free( fp );

    CYG_CANCELLATION_POINT;
    
    FILEIO_RETURN(ret);
}

//==========================================================================
// fstat

__externC int fstat( int fd, struct stat *buf )
{
    FILEIO_ENTRY();

    int ret;
    cyg_file *fp;
    
    fp = cyg_fp_get( fd );

    if( fp == NULL )
        FILEIO_RETURN(EBADF);

    LOCK_FILE( fp );
    
    ret = fp->f_ops->fo_fstat( fp, buf );

    UNLOCK_FILE( fp );
    
    cyg_fp_free( fp );

    FILEIO_RETURN(ret);
}

//==========================================================================
// fpathconf

__externC long fpathconf( int fd, int name )
{
    FILEIO_ENTRY();

    int ret;
    cyg_file *fp;
    
    fp = cyg_fp_get( fd );

    if( fp == NULL )
        FILEIO_RETURN(EBADF);

    struct cyg_pathconf_info info;

    info.name = name;
    info.value = 0;
    
    LOCK_FILE( fp );
    
    ret = fp->f_ops->fo_getinfo( fp, FILE_INFO_CONF, (char *)&info, sizeof(info) );

    UNLOCK_FILE( fp );
    
    cyg_fp_free( fp );

    if( ret != 0 )
        FILEIO_RETURN(ret);

    FILEIO_RETURN_VALUE(info.value);
}

//==========================================================================
// fcntl

__externC int fcntl( int fd, int cmd, ... )
{
    FILEIO_ENTRY();

    CYG_CANCELLATION_POINT;
    
    int ret = 0;
    cyg_file *fp;
    va_list a;

    fp = cyg_fp_get( fd );

    if( fp == NULL )
        FILEIO_RETURN(EBADF);

    va_start( a, cmd );
        
    switch( cmd )
    {
    case F_DUPFD:
    {
        int fda = va_arg(a, int);

        if( fda < 0 || fda >= OPEN_MAX )
        {
            errno = EBADF;
            break;
        }
        
        int fd2 = cyg_fd_alloc( fda );

        if( fd2 == -1 )
        {
            ret = EMFILE;
            break;
        }

        cyg_fd_assign( fd2, fp );
        
        break;
    }
    
    default:
        ret = ENOTSUP;
        break;
    }

    va_end(a);
    
    cyg_fp_free( fp );
   
    CYG_CANCELLATION_POINT;
    
    FILEIO_RETURN(ret);
}

//==========================================================================
// isatty()

__externC int isatty( int fd )
{
    FILEIO_ENTRY();

    int ret = 0;
    struct stat buf;
    int err;

    err = fstat( fd, &buf );

    // Any error and we return zero. If the client wants to
    // they can always pick up the error code from errno.
    if( err < 0 )
        FILEIO_RETURN_VALUE(0);

    // For now we assume that all char devices are ttys.
    // In future we may need to have a special getinfo()
    // call to decide this more specifically.
    
    if( S_ISCHR( buf.st_mode ) )
        ret = 1;
    
    FILEIO_RETURN_VALUE(ret);
}

// -------------------------------------------------------------------------
// EOF io.cxx
