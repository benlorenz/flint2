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

    Copyright (C) 2009 William Hart

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <mpir.h>
#include "flint.h"
#include "fmpz.h"
#include "fmpz_poly.h"
#include "ulong_extras.h"

int
main(void)
{
    int i, result;
    flint_rand_t state;

    printf("divrem_divconquer....");
    fflush(stdout);

    flint_randinit(state);

    /* Check q*b + r = a, no aliasing */
    for (i = 0; i < 2000; i++)
    {
        fmpz_poly_t a, b, q, r, prod;

        fmpz_poly_init(a);
        fmpz_poly_init(b);
        fmpz_poly_init(q);
        fmpz_poly_init(r);
        fmpz_poly_init(prod);
        fmpz_poly_randtest(a, state, n_randint(100, state), 100);
        fmpz_poly_randtest_not_zero(b, state, n_randint(100, state) + 1, 100);

        fmpz_poly_divrem_divconquer(q, r, a, b);
        fmpz_poly_mul(prod, q, b);
        fmpz_poly_add(prod, prod, r);

        result = (fmpz_poly_equal(a, prod));
        if (!result)
        {
            printf("FAIL:\n");
            fmpz_poly_print(a), printf("\n\n");
            fmpz_poly_print(prod), printf("\n\n");
            fmpz_poly_print(q), printf("\n\n");
            fmpz_poly_print(r), printf("\n\n");
            abort();
        }

        fmpz_poly_clear(a);
        fmpz_poly_clear(b);
        fmpz_poly_clear(q);
        fmpz_poly_clear(r);
        fmpz_poly_clear(prod);
    }

    /* Check r and a alias */
    for (i = 0; i < 1000; i++)
    {
        fmpz_poly_t a, b, q, r;

        fmpz_poly_init(a);
        fmpz_poly_init(b);
        fmpz_poly_init(q);
        fmpz_poly_init(r);
        fmpz_poly_randtest(a, state, n_randint(100, state), 100);
        fmpz_poly_randtest_not_zero(b, state, n_randint(100, state) + 1, 100);

        fmpz_poly_divrem_divconquer(q, r, a, b);
        fmpz_poly_divrem_divconquer(q, a, a, b);

        result = (fmpz_poly_equal(a, r));
        if (!result)
        {
            printf("FAIL:\n");
            fmpz_poly_print(a), printf("\n\n");
            fmpz_poly_print(q), printf("\n\n");
            fmpz_poly_print(r), printf("\n\n");
            abort();
        }

        fmpz_poly_clear(a);
        fmpz_poly_clear(b);
        fmpz_poly_clear(q);
        fmpz_poly_clear(r);
    }

    /* Check r and b alias */
    for (i = 0; i < 1000; i++)
    {
        fmpz_poly_t a, b, q, r;

        fmpz_poly_init(a);
        fmpz_poly_init(b);
        fmpz_poly_init(q);
        fmpz_poly_init(r);
        fmpz_poly_randtest(a, state, n_randint(100, state), 100);
        fmpz_poly_randtest_not_zero(b, state, n_randint(100, state) + 1, 100);

        fmpz_poly_divrem_divconquer(q, r, a, b);
        fmpz_poly_divrem_divconquer(q, b, a, b);

        result = (fmpz_poly_equal(b, r));
        if (!result)
        {
            printf("FAIL:\n");
            fmpz_poly_print(a), printf("\n\n");
            fmpz_poly_print(q), printf("\n\n");
            fmpz_poly_print(r), printf("\n\n");
            abort();
        }

        fmpz_poly_clear(a);
        fmpz_poly_clear(b);
        fmpz_poly_clear(q);
        fmpz_poly_clear(r);
    }

    /* Check q and a alias */
    for (i = 0; i < 1000; i++)
    {
        fmpz_poly_t a, b, q, r;

        fmpz_poly_init(a);
        fmpz_poly_init(b);
        fmpz_poly_init(q);
        fmpz_poly_init(r);
        fmpz_poly_randtest(a, state, n_randint(100, state), 100);
        fmpz_poly_randtest_not_zero(b, state, n_randint(100, state) + 1, 100);

        fmpz_poly_divrem_divconquer(q, r, a, b);
        fmpz_poly_divrem_divconquer(a, r, a, b);

        result = (fmpz_poly_equal(a, q));
        if (!result)
        {
            printf("FAIL:\n");
            fmpz_poly_print(a), printf("\n\n");
            fmpz_poly_print(q), printf("\n\n");
            fmpz_poly_print(r), printf("\n\n");
            abort();
        }

        fmpz_poly_clear(a);
        fmpz_poly_clear(b);
        fmpz_poly_clear(q);
        fmpz_poly_clear(r);
    }

    /* Check q and b alias */
    for (i = 0; i < 1000; i++)
    {
        fmpz_poly_t a, b, q, r;

        fmpz_poly_init(a);
        fmpz_poly_init(b);
        fmpz_poly_init(q);
        fmpz_poly_init(r);
        fmpz_poly_randtest(a, state, n_randint(100, state), 100);
        fmpz_poly_randtest_not_zero(b, state, n_randint(100, state) + 1, 100);

        fmpz_poly_divrem_divconquer(q, r, a, b);
        fmpz_poly_divrem_divconquer(b, r, a, b);

        result = (fmpz_poly_equal(b, q));
        if (!result)
        {
            printf("FAIL:\n");
            fmpz_poly_print(a), printf("\n\n");
            fmpz_poly_print(q), printf("\n\n");
            fmpz_poly_print(r), printf("\n\n");
            abort();
        }

        fmpz_poly_clear(a);
        fmpz_poly_clear(b);
        fmpz_poly_clear(q);
        fmpz_poly_clear(r);
    }

    flint_randclear(state);
    _fmpz_cleanup();
    printf("PASS\n");
    return 0;
}
