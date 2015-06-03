/*
  This file is part of t8code.
  t8code is a C library to manage a collection (a forest) of multiple
  connected adaptive space-trees of general element classes in parallel.

  Copyright (C) 2015 the developers

  t8code is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  t8code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with t8code; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <p4est_bits.h>
#include "t8_default_common.h"
#include "t8_default_quad.h"

#ifdef T8_ENABLE_DEBUG

static int
t8_default_quad_surround_matches (const p4est_quadrant_t * q,
                                  const p4est_quadrant_t * r)
{
  return T8_QUAD_GET_TDIM (q) == T8_QUAD_GET_TDIM (r) &&
    (T8_QUAD_GET_TDIM (q) == -1 ||
     (T8_QUAD_GET_TNORMAL (q) == T8_QUAD_GET_TNORMAL (r) &&
      T8_QUAD_GET_TCOORD (q) == T8_QUAD_GET_TCOORD (r)));
}

#endif /* T8_ENABLE_DEBUG */

static              size_t
t8_default_quad_size (void)
{
  return sizeof (t8_pquad_t);
}

static int
t8_default_quad_maxlevel (void)
{
  return P4EST_QMAXLEVEL;
}

static              t8_eclass_t
t8_default_quad_child_eclass (int childid)
{
  T8_ASSERT (0 <= childid && childid < P4EST_CHILDREN);

  return T8_ECLASS_QUAD;
}

static int
t8_default_quad_level (const t8_element_t * elem)
{
  return (int) ((const p4est_quadrant_t *) elem)->level;
}

static void
t8_default_quad_copy_surround (const p4est_quadrant_t * q,
                               p4est_quadrant_t * r)
{
  T8_QUAD_SET_TDIM (r, T8_QUAD_GET_TDIM (q));
  T8_QUAD_SET_TNORMAL (r, T8_QUAD_GET_TNORMAL (q));
  T8_QUAD_SET_TCOORD (r, T8_QUAD_GET_TCOORD (q));
}

static void
t8_default_quad_parent (const t8_element_t * elem, t8_element_t * parent)
{
  const p4est_quadrant_t *q = (const p4est_quadrant_t *) elem;
  p4est_quadrant_t   *r = (p4est_quadrant_t *) parent;

  p4est_quadrant_parent (q, r);
  t8_default_quad_copy_surround (q, r);
}

static void
t8_default_quad_sibling (const t8_element_t * elem,
                         int sibid, t8_element_t * sibling)
{
  const p4est_quadrant_t *q = (const p4est_quadrant_t *) elem;
  p4est_quadrant_t   *r = (p4est_quadrant_t *) sibling;

  p4est_quadrant_sibling (q, r, sibid);
  t8_default_quad_copy_surround (q, r);
}

static void
t8_default_quad_child (const t8_element_t * elem,
                       int childid, t8_element_t * child)
{
  const p4est_quadrant_t *q = (const p4est_quadrant_t *) elem;
  const p4est_qcoord_t shift = P4EST_QUADRANT_LEN (q->level + 1);
  p4est_quadrant_t   *r = (p4est_quadrant_t *) child;

  T8_ASSERT (p4est_quadrant_is_extended (q));
  T8_ASSERT (q->level < P4EST_QMAXLEVEL);
  T8_ASSERT (childid >= 0 && childid < P4EST_CHILDREN);

  r->x = childid & 0x01 ? (q->x | shift) : q->x;
  r->y = childid & 0x02 ? (q->y | shift) : q->y;
  r->level = q->level + 1;
  T8_ASSERT (p4est_quadrant_is_parent (q, r));

  t8_default_quad_copy_surround (q, r);
}

static void
t8_default_quad_children (const t8_element_t * elem,
                          int length, t8_element_t * c[])
{
  const p4est_quadrant_t *q = (const p4est_quadrant_t *) elem;
  int                 i;

  T8_ASSERT (length == P4EST_CHILDREN);

  p4est_quadrant_childrenpv (q, (p4est_quadrant_t **) c);
  for (i = 0; i < P4EST_CHILDREN; ++i) {
    t8_default_quad_copy_surround (q, (p4est_quadrant_t *) c[i]);
  }
}

static void
t8_default_quad_set_linear_id (t8_element_t * elem, int level, uint64_t id)
{
  T8_ASSERT (0 <= level && level <= P4EST_QMAXLEVEL);
  T8_ASSERT (0 <= id && id < (uint64_t) 1 << P4EST_CHILDREN * level);

  p4est_quadrant_set_morton ((p4est_quadrant_t *) elem, level, id);
}

static void
t8_default_quad_successor (const t8_element_t * elem1,
                           t8_element_t * elem2, int level)
{
  uint64_t            id;
  T8_ASSERT (0 <= level && level <= P4EST_QMAXLEVEL);

  id = p4est_quadrant_linear_id ((const p4est_quadrant_t *) elem1, level);
  T8_ASSERT (id + 1 < (1 << P4EST_CHILDREN * level));
  p4est_quadrant_set_morton ((p4est_quadrant_t *) elem1, level, id + 1);
}

static void
t8_default_quad_nca (const t8_element_t * elem1, const t8_element_t * elem2,
                     t8_element_t * nca)
{
  const p4est_quadrant_t *q1 = (const p4est_quadrant_t *) elem1;
  const p4est_quadrant_t *q2 = (const p4est_quadrant_t *) elem2;
  p4est_quadrant_t   *r = (p4est_quadrant_t *) nca;

  T8_ASSERT (t8_default_quad_surround_matches (q1, q2));

  p4est_nearest_common_ancestor (q1, q2, r);
  t8_default_quad_copy_surround (q1, r);
}

static void
t8_default_quad_boundary (const t8_element_t * elem,
                          int min_dim, int length, t8_element_t ** boundary)
{
#ifdef T8_ENABLE_DEBUG
  int                 per_eclass[T8_ECLASS_LAST];
#endif

  T8_ASSERT (length ==
             t8_eclass_count_boundary (T8_ECLASS_QUAD, min_dim, per_eclass));

  /* TODO: write this function */
  SC_ABORT_NOT_REACHED ();
}

t8_eclass_scheme_t *
t8_default_scheme_new_quad (void)
{
  t8_eclass_scheme_t *ts;

  ts = T8_ALLOC (t8_eclass_scheme_t, 1);
  ts->eclass = T8_ECLASS_QUAD;

  ts->elem_size = t8_default_quad_size;
  ts->elem_maxlevel = t8_default_quad_maxlevel;
  ts->elem_child_eclass = t8_default_quad_child_eclass;

  ts->elem_level = t8_default_quad_level;
  ts->elem_parent = t8_default_quad_parent;
  ts->elem_sibling = t8_default_quad_sibling;
  ts->elem_child = t8_default_quad_child;
  ts->elem_children = t8_default_quad_children;
  ts->elem_nca = t8_default_quad_nca;
  ts->elem_boundary = t8_default_quad_boundary;
  ts->elem_set_linear_id = t8_default_quad_set_linear_id;
  ts->elem_successor = t8_default_quad_successor;

  ts->elem_new = t8_default_mempool_alloc;
  ts->elem_destroy = t8_default_mempool_free;

  ts->ts_destroy = t8_default_scheme_mempool_destroy;
  ts->ts_context = sc_mempool_new (sizeof (t8_pquad_t));

  return ts;
}
