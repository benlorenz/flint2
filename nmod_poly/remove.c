/*=============================================================================

    This file is part of FLINT.

    FLINT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    FLINT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FLINT; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

=============================================================================*/
/******************************************************************************

    Copyright (C) 2007 David Howden
    Copyright (C) 2007, 2008, 2009, 2010 William Hart
    Copyright (C) 2008 Richard Howell-Peak
    Copyright (C) 2011 Fredrik Johansson

******************************************************************************/

#include <stdio.h>
#include <mpir.h>
#include "flint.h"
#include "nmod_poly.h"

ulong
nmod_poly_remove(nmod_poly_t f, const nmod_poly_t p)
{
    nmod_poly_t q, r;
    ulong i = 0;

    nmod_poly_init_preinv(q, p->mod.n, p->mod.ninv);
    nmod_poly_init_preinv(r, p->mod.n, p->mod.ninv);

    do
    {
        if (f->length < p->length)
            break;
        nmod_poly_divrem(q, r, f, p);
        if (r->length == 0)
            nmod_poly_swap(q, f);
        else
            break;
        i++;
    }
    while (1);

    nmod_poly_clear(q);
    nmod_poly_clear(r);

    return i;
}
