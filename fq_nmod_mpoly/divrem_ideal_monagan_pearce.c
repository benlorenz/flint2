/*
    Copyright (C) 2019 Daniel Schultz

    This file is part of FLINT.

    FLINT is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.  See <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include "fq_nmod_mpoly.h"

/*
   As for divrem_monagan_pearce except that an array of divisor polynomials is
   passed and an array of quotient polynomials is returned. These are not in
   low level format.
*/
slong _fq_nmod_mpoly_divrem_ideal_monagan_pearce(fq_nmod_mpoly_struct ** polyq,
               fq_nmod_struct ** polyr, ulong ** expr, slong * allocr,
           const fq_nmod_struct * poly2, const ulong * exp2, slong len2,
           fq_nmod_mpoly_struct * const * poly3, ulong * const * exp3,
                                            slong len, slong N, slong bits, 
                        const fq_nmod_mpoly_ctx_t ctx, const ulong * cmpmask)
{
    slong i, j, p, r_len, w;
    slong next_loc;
    slong * store, * store_base;
    slong len3;
    slong heap_len = 2; /* heap zero index unused */
    mpoly_heap_s * heap;
    mpoly_nheap_t ** chains;
    slong ** hinds;
    mpoly_nheap_t * x;
    fq_nmod_struct * r_coeff = *polyr;
    ulong * r_exp = *expr;
    ulong * exp, * exps, * texp;
    ulong ** exp_list;
    slong exp_next;
    ulong mask;
    slong * q_len, * s;
    fq_nmod_struct * lc_minus_inv;
    fq_nmod_t acc, pp;
    TMP_INIT;

    TMP_START;
   
    chains = (mpoly_nheap_t **) TMP_ALLOC(len*sizeof(mpoly_nheap_t *));
    hinds = (slong **) TMP_ALLOC(len*sizeof(mpoly_heap_t *));
    len3 = 0;
    for (w = 0; w < len; w++)
    {
        chains[w] = (mpoly_nheap_t *) TMP_ALLOC((poly3[w]->length)*sizeof(mpoly_nheap_t));
        hinds[w] = (slong *) TMP_ALLOC((poly3[w]->length)*sizeof(slong));
        len3 += poly3[w]->length;
        for (i = 0; i < poly3[w]->length; i++)
            hinds[w][i] = 1;
    }

    next_loc = len3 + 4;   /* something bigger than heap can ever be */
    heap = (mpoly_heap_s *) TMP_ALLOC((len3 + 1)*sizeof(mpoly_heap_s));
    store = store_base = (slong *) TMP_ALLOC(3*len3*sizeof(mpoly_nheap_t *));

    exps = (ulong *) TMP_ALLOC(len3*N*sizeof(ulong));
    exp_list = (ulong **) TMP_ALLOC(len3*sizeof(ulong *));
    texp = (ulong *) TMP_ALLOC(N*sizeof(ulong));
    exp = (ulong *) TMP_ALLOC(N*sizeof(ulong));
    q_len = (slong *) TMP_ALLOC(len*sizeof(slong));
    s = (slong *) TMP_ALLOC(len*sizeof(slong));

    exp_next = 0;
    for (i = 0; i < len3; i++)
        exp_list[i] = exps + i*N;

    mask = 0;
    for (i = 0; i < FLINT_BITS/bits; i++)
        mask = (mask << bits) + (UWORD(1) << (bits - 1));

    for (w = 0; w < len; w++)
    {
        q_len[w] = WORD(0);
        s[w] = poly3[w]->length;
    }
    r_len = WORD(0);
   
    x = chains[0] + 0;
    x->i = -WORD(1);
    x->j = 0;
    x->p = -WORD(1);
    x->next = NULL;
    heap[1].next = x;
    heap[1].exp = exp_list[exp_next++];
    mpoly_monomial_set(heap[1].exp, exp2, N);

    /* precompute leading coeff info */
    fq_nmod_init(pp, ctx->fqctx);
    fq_nmod_init(acc, ctx->fqctx);
    lc_minus_inv = (fq_nmod_struct *) TMP_ALLOC(len*sizeof(fq_nmod_struct));
    for (w = 0; w < len; w++)
    {
        fq_nmod_init(lc_minus_inv + w, ctx->fqctx);
        fq_nmod_inv(lc_minus_inv + w, poly3[w]->coeffs + 0, ctx->fqctx);
        fq_nmod_neg(lc_minus_inv + w, lc_minus_inv + w, ctx->fqctx);
    }

    while (heap_len > 1)
    {
        mpoly_monomial_set(exp, heap[1].exp, N);

        if (bits <= FLINT_BITS)
        {
            if (mpoly_monomial_overflows(exp, N, mask))
                goto exp_overflow;
        }
        else
        {
            if (mpoly_monomial_overflows_mp(exp, N, bits))
                goto exp_overflow;
        }

        fq_nmod_zero(acc, ctx->fqctx);
        do
        {
            exp_list[--exp_next] = heap[1].exp;
            x = _mpoly_heap_pop(heap, &heap_len, N, cmpmask);
            do
            {
                *store++ = x->i;
                *store++ = x->j;
                *store++ = x->p;
                if (x->i != -WORD(1))
                {
                    hinds[x->p][x->i] |= WORD(1);
                }

                if (x->i == -WORD(1))
                {
                    fq_nmod_sub(acc, acc, poly2 + x->j, ctx->fqctx);
                }
                else
                {
                    fq_nmod_mul(pp, poly3[x->p]->coeffs + x->i,
                                    polyq[x->p]->coeffs + x->j, ctx->fqctx);
                    fq_nmod_add(acc, acc, pp, ctx->fqctx);
                }
            } while ((x = x->next) != NULL);
        } while (heap_len > 1 && mpoly_monomial_equal(heap[1].exp, exp, N));

        while (store > store_base)
        {
            p = *--store;
            j = *--store;
            i = *--store;
            if (i == -WORD(1))
            {
                if (j + 1 < len2)
                {
                    x = chains[0] + 0;
                    x->i = -WORD(1);
                    x->j = j + 1;
                    x->p = p;
                    x->next = NULL;
                    mpoly_monomial_set(exp_list[exp_next], exp2 + x->j*N, N);
                    exp_next += _mpoly_heap_insert(heap, exp_list[exp_next], x,
                                             &next_loc, &heap_len, N, cmpmask);
                }
            }
            else
            {
                /* should we go right? */
                if ( (i + 1 < poly3[p]->length)
                   && (hinds[p][i + 1] == 2*j + 1)
                   )
                {
                    x = chains[p] + i + 1;
                    x->i = i + 1;
                    x->j = j;
                    x->p = p;
                    x->next = NULL;
                    hinds[p][x->i] = 2*(x->j + 1) + 0;
                    mpoly_monomial_add(exp_list[exp_next], exp3[x->p] + x->i*N,
                                                polyq[x->p]->exps + x->j*N, N);
                    exp_next += _mpoly_heap_insert(heap, exp_list[exp_next], x,
                                             &next_loc, &heap_len, N, cmpmask);
                }

                /* should we go up? */
                if (j + 1 == q_len[p])
                {
                    s[p]++;
                }
                else if (  ((hinds[p][i] & 1) == 1)
                          && ((i == 1) || (hinds[p][i - 1] >= 2*(j + 2) + 1)) )
                {
                    x = chains[p] + i;
                    x->i = i;
                    x->j = j + 1;
                    x->p = p;
                    x->next = NULL;
                    hinds[p][x->i] = 2*(x->j + 1) + 0;
                    mpoly_monomial_add_mp(exp_list[exp_next], exp3[x->p] + x->i*N,
                                                polyq[x->p]->exps + x->j*N, N);
                    exp_next += _mpoly_heap_insert(heap, exp_list[exp_next], x,
                                             &next_loc, &heap_len, N, cmpmask);
                }
            }
        }

        if (fq_nmod_is_zero(acc, ctx->fqctx))
            continue;

        for (w = 0; w < len; w++)
        {
            int divides;

            if (bits <= FLINT_BITS)
                divides = mpoly_monomial_divides(texp, exp, exp3[w] + N*0, N, mask);
            else
                divides = mpoly_monomial_divides_mp(texp, exp, exp3[w] + N*0, N, bits);

            if (divides)
            {
                fq_nmod_mpoly_fit_length(polyq[w], q_len[w] + 1, ctx);
                fq_nmod_mul(polyq[w]->coeffs + q_len[w],
                                            acc, lc_minus_inv + w, ctx->fqctx);
                mpoly_monomial_set(polyq[w]->exps + N*q_len[w], texp, N);
                if (s[w] > 1)
                {
                    i = 1;
                    x = chains[w] + i;
                    x->i = i;
                    x->j = q_len[w];
                    x->p = w;
                    x->next = NULL;
                    hinds[w][x->i] = 2*(x->j + 1) + 0;
                    mpoly_monomial_add_mp(exp_list[exp_next], exp3[w] + N*x->i, 
                                                   polyq[w]->exps + N*x->j, N);
                    exp_next += _mpoly_heap_insert(heap, exp_list[exp_next], x,
                                             &next_loc, &heap_len, N, cmpmask);
                }
                s[w] = 1;
                q_len[w]++;
                /* break out of w for loop and continue in heap loop */
                goto break_continue;
            }
        }

        /* if get here, no leading terms divided */
        _fq_nmod_mpoly_fit_length(&r_coeff, &r_exp, allocr, r_len + 1, N, ctx->fqctx);
        fq_nmod_neg(r_coeff + r_len, acc, ctx->fqctx);
        mpoly_monomial_set(r_exp + r_len*N, exp, N);
        r_len++;

break_continue:

        (void)(NULL);
    }

cleanup2:

    fq_nmod_clear(pp, ctx->fqctx);
    fq_nmod_clear(acc, ctx->fqctx);
    for (w = 0; w < len; w++)
    {
        fq_nmod_clear(lc_minus_inv + w, ctx->fqctx);
    }

    for (i = 0; i < len; i++)
    {
        _fq_nmod_mpoly_set_length(polyq[i], q_len[i], ctx);
    }

    (*polyr) = r_coeff;
    (*expr) = r_exp;

    TMP_END;

    return r_len;

exp_overflow:
    for (w = 0; w < len; w++)
        q_len[w] = WORD(0);

    r_len = -WORD(1);
    goto cleanup2;

}

/* Assumes divisor polys don't alias any output polys */
void fq_nmod_mpoly_divrem_ideal_monagan_pearce(fq_nmod_mpoly_struct ** q,
     fq_nmod_mpoly_t r, const fq_nmod_mpoly_t poly2, fq_nmod_mpoly_struct * const * poly3,
                                      slong len, const fq_nmod_mpoly_ctx_t ctx)
{
    slong i, exp_bits, N, lenr = 0;
    slong len3 = 0;
    ulong * cmpmask;
    ulong * exp2;
    ulong ** exp3;
    int free2 = 0;
    int * free3;
    fq_nmod_mpoly_t temp2;
    fq_nmod_mpoly_struct * tr;
    TMP_INIT;

    for (i = 0; i < len; i++)
    {  
        len3 = FLINT_MAX(len3, poly3[i]->length);
        if (poly3[i]->length == 0)
        {
            flint_throw(FLINT_DIVZERO,
                "Divide by zero in fq_nmod_mpoly_divrem_ideal_monagan_pearce");
        }
    }

    /* dividend is zero, write out quotients and remainder */
    if (poly2->length == 0)
    {
        for (i = 0; i < len; i++)
            fq_nmod_mpoly_zero(q[i], ctx);
        fq_nmod_mpoly_zero(r, ctx);
        return;
    }

    TMP_START;

    free3 = (int *) TMP_ALLOC(len*sizeof(int));
    exp3 = (ulong **) TMP_ALLOC(len*sizeof(ulong *));

    /* compute maximum degrees that can occur in any input or output polys */
    exp_bits = poly2->bits;
    for (i = 0; i < len; i++)
    {
        exp_bits = FLINT_MAX(exp_bits, poly3[i]->bits);
    }
    exp_bits = mpoly_fix_bits(exp_bits, ctx->minfo);

    N = mpoly_words_per_exp(exp_bits, ctx->minfo);
    cmpmask = (ulong*) TMP_ALLOC(N*sizeof(ulong));
    mpoly_get_cmpmask(cmpmask, N, exp_bits, ctx->minfo);

    /* ensure input exponents packed to same size as output exponents */
    exp2 = poly2->exps;
    free2 = 0;
    if (exp_bits > poly2->bits)
    {
        free2 = 1;
        exp2 = (ulong *) flint_malloc(N*poly2->length*sizeof(ulong));
        mpoly_repack_monomials(exp2, exp_bits, poly2->exps, poly2->bits,
                                                    poly2->length, ctx->minfo);
    }

    for (i = 0; i < len; i++)
    {
        exp3[i] = poly3[i]->exps;
        free3[i] = 0;
        if (exp_bits > poly3[i]->bits)
        {
            free3[i] = 1;
            exp3[i] = (ulong *) flint_malloc(N*poly3[i]->length*sizeof(ulong));
            mpoly_repack_monomials(exp3[i], exp_bits, poly3[i]->exps,
                                 poly3[i]->bits, poly3[i]->length, ctx->minfo);
        }
        fq_nmod_mpoly_fit_length(q[i], 1, ctx);
        fq_nmod_mpoly_fit_bits(q[i], exp_bits, ctx);
        q[i]->bits = exp_bits;
    }

    /* check leading mon. of at least one divisor is at most that of dividend */
    for (i = 0; i < len; i++)
    {
        if (!mpoly_monomial_lt(exp3[i], exp2, N, cmpmask))
            break;
    }

    if (i == len)
    {
        fq_nmod_mpoly_set(r, poly2, ctx);
        for (i = 0; i < len; i++)
            fq_nmod_mpoly_zero(q[i], ctx);

        goto cleanup3;
    }

    /* take care of aliasing */
    if (r == poly2)
    {
        fq_nmod_mpoly_init2(temp2, len3, ctx);
        fq_nmod_mpoly_fit_bits(temp2, exp_bits, ctx);
        temp2->bits = exp_bits;
        tr = temp2;
    }
    else
    {
        fq_nmod_mpoly_fit_length(r, len3, ctx);
        fq_nmod_mpoly_fit_bits(r, exp_bits, ctx);
        r->bits = exp_bits;
        tr = r;
    }

    /* do division with remainder */
    while (1)
    {
        slong old_exp_bits = exp_bits;
        ulong * old_exp2 = exp2, * old_exp3;

        lenr = _fq_nmod_mpoly_divrem_ideal_monagan_pearce(q, &tr->coeffs, &tr->exps,
                         &tr->alloc, poly2->coeffs, exp2, poly2->length,
                           poly3, exp3, len, N, exp_bits, ctx, cmpmask);

        if (lenr >= 0) /* check if division was successful */
            break;

        exp_bits = mpoly_fix_bits(exp_bits + 1, ctx->minfo);
        N = mpoly_words_per_exp(exp_bits, ctx->minfo);
        cmpmask = (ulong*) TMP_ALLOC(N*sizeof(ulong));
        mpoly_get_cmpmask(cmpmask, N, exp_bits, ctx->minfo);

        exp2 = (ulong *) flint_malloc(N*poly2->length*sizeof(ulong));
        mpoly_repack_monomials(exp2, exp_bits, old_exp2, old_exp_bits,
                                                    poly2->length, ctx->minfo);

        if (free2)
            flint_free(old_exp2);

        free2 = 1;

        fq_nmod_mpoly_fit_bits(tr, exp_bits, ctx);
        tr->bits = exp_bits;

        for (i = 0; i < len; i++)
        {
            old_exp3 = exp3[i];

            exp3[i] = (ulong *) flint_malloc(N*poly3[i]->length*sizeof(ulong));
            mpoly_repack_monomials(exp3[i], exp_bits, old_exp3, old_exp_bits,
                                                 poly3[i]->length, ctx->minfo);
   
            if (free3[i])
                flint_free(old_exp3);

            free3[i] = 1;

            fq_nmod_mpoly_fit_bits(q[i], exp_bits, ctx);
            q[i]->bits = exp_bits;
        }
    }

    /* take care of aliasing */
    if (r == poly2)
    {
        fq_nmod_mpoly_swap(temp2, r, ctx);
        fq_nmod_mpoly_clear(temp2, ctx);
    } 

    _fq_nmod_mpoly_set_length(r, lenr, ctx);

cleanup3:

    if (free2)
        flint_free(exp2);

    for (i = 0; i < len; i++)
    {
        if (free3[i])
            flint_free(exp3[i]);
    }

    TMP_END;
}