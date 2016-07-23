/*
    Copyright (C) 2015 Vladimir Glazachev

    This file is part of FLINT.

    FLINT is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.  See <http://www.gnu.org/licenses/>.
*/

#include "aprcl.h"

int
unity_zpq_equal(const unity_zpq f, const unity_zpq g)
{
    ulong i;

    if (f->p != g->p)
        return 0;
    if (f->q != g->q)
        return 0;
    if (fmpz_equal(f->n, g->n) == 0)
        return 0;

    for (i = 0; i < f->p; i++)
        if (fmpz_mod_poly_equal(f->polys[i], g->polys[i]) == 0)
            return 0;

    return 1;
}
