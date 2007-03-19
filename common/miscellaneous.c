/* miscellaneous.c - Stuff not fitting elsewhere
 *	Copyright (C) 2003, 2006 Free Software Foundation, Inc.
 *
 * This file is part of GnuPG.
 *
 * GnuPG is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GnuPG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

#include <config.h>
#include <stdlib.h>
#include <errno.h>

#include "util.h"
#include "iobuf.h"


/* Decide whether the filename is stdout or a real filename and return
 * an appropriate string.  */
const char *
print_fname_stdout (const char *s)
{
    if( !s || (*s == '-' && !s[1]) )
	return "[stdout]";
    return s;
}


/* Decide whether the filename is stdin or a real filename and return
 * an appropriate string.  */
const char *
print_fname_stdin (const char *s)
{
    if( !s || (*s == '-' && !s[1]) )
	return "[stdin]";
    return s;
}

/* fixme: Globally replace it by print_sanitized_buffer. */
void
print_string( FILE *fp, const byte *p, size_t n, int delim )
{
  print_sanitized_buffer (fp, p, n, delim);
}

void
print_utf8_string2 ( FILE *fp, const byte *p, size_t n, int delim )
{
  print_sanitized_utf8_buffer (fp, p, n, delim);
}

void
print_utf8_string( FILE *fp, const byte *p, size_t n )
{
    print_utf8_string2 (fp, p, n, 0);
}

/* Write LENGTH bytes of BUFFER to FP as a hex encoded string.
   RESERVED must be 0. */
void
print_hexstring (FILE *fp, const void *buffer, size_t length, int reserved)
{
#define tohex(n) ((n) < 10 ? ((n) + '0') : (((n) - 10) + 'A'))
  const unsigned char *s;

  for (s = buffer; length; s++, length--)
    {
      putc ( tohex ((*s>>4)&15), fp);
      putc ( tohex (*s&15), fp);
    }
#undef tohex
}

char *
make_printable_string (const void *p, size_t n, int delim )
{
  return sanitize_buffer (p, n, delim);
}



/*
 * Check if the file is compressed.
 */
int
is_file_compressed (const char *s, int *ret_rc)
{
    iobuf_t a;
    byte buf[4];
    int i, rc = 0;
    int overflow;

    struct magic_compress_s {
        size_t len;
        byte magic[4];
    } magic[] = {
        { 3, { 0x42, 0x5a, 0x68, 0x00 } }, /* bzip2 */
        { 3, { 0x1f, 0x8b, 0x08, 0x00 } }, /* gzip */
        { 4, { 0x50, 0x4b, 0x03, 0x04 } }, /* (pk)zip */
    };
    
    if ( iobuf_is_pipe_filename (s) || !ret_rc )
        return 0; /* We can't check stdin or no file was given */

    a = iobuf_open( s );
    if ( a == NULL ) {
        *ret_rc = gpg_error_from_syserror ();
        return 0;
    }

    if ( iobuf_get_filelength( a, &overflow ) < 4 && !overflow) {
        *ret_rc = 0;
        goto leave;
    }

    if ( iobuf_read( a, buf, 4 ) == -1 ) {
        *ret_rc = a->error;
        goto leave;
    }

    for ( i = 0; i < DIM( magic ); i++ ) {
        if ( !memcmp( buf, magic[i].magic, magic[i].len ) ) {
            *ret_rc = 0;
            rc = 1;
            break;
        }
    }

leave:    
    iobuf_close( a );
    return rc;
}


/* Try match against each substring of multistr, delimited by | */
int
match_multistr (const char *multistr,const char *match)
{
  do
    {
      size_t seglen = strcspn (multistr,"|");
      if (!seglen)
	break;
      /* Using the localized strncasecmp! */
      if (strncasecmp(multistr,match,seglen)==0)
	return 1;
      multistr += seglen;
      if (*multistr == '|')
	multistr++;
    }
  while (*multistr);

  return 0;
}


