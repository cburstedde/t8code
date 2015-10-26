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

/** \file t8_cmesh.h
 * We define the coarse mesh of trees in this file.
 */

#ifndef T8_CMESH_H
#define T8_CMESH_H

#include <t8.h>
#include <t8_eclass.h>

/* TODO: do set_mpicomm and figure out dup logic */

typedef struct t8_cmesh *t8_cmesh_t;
typedef struct t8_ctree *t8_ctree_t;
typedef struct t8_cghost *t8_cghost_t;

T8_EXTERN_C_BEGIN ();

/** Create a new cmesh with reference count one.
 * This cmesh needs to be specialized with the t8_cmesh_set_* calls.
 * Then it needs to be set up with \see t8_cmesh_commit.
 * \param [in,out] pcmesh       On input, this pointer must be non-NULL.
 *                              On return, this pointer set to the new cmesh.
 */
void                t8_cmesh_init (t8_cmesh_t * pcmesh);

/** Set MPI communicator to use in commiting a new cmesh.
 * This call is only valid when the cmesh is not yet committed via a call
 * to \see t8_cmesh_commit.
 * \param [in,out] cmesh        The cmesh whose communicator will be set.
 * \param [in] mpicomm          This MPI communicator must be valid.
 * \param [in] do_dup           If true, the communicator will be duped in
 *                              this creation and whenever another cmesh is
 *                              derived from it in the future.
 *                              If false, no duping takes place at all.
 */
void                t8_cmesh_set_mpicomm (t8_cmesh_t cmesh,
                                          sc_MPI_Comm mpicomm, int do_dup);

/** Return the MPI communicator of a cmesh.
 * \param [in] cmesh       The cmesh whose communicator will be returned.
 * \param [out] do_dup     This variable is filled with the do_dup entry of \a cmesh.
 * \return                 The MPI communicator associated to \a cmesh.
 */
sc_MPI_Comm         t8_cmesh_get_mpicomm (t8_cmesh_t cmesh, int *do_dup);

/** Declare if the cmesh is understood as a partitioned cmesh or a
 * replicated cmesh. Replicated (each processor owns the whole mesh) is
 * the default and in this case \ref t8_cmesh_set_partitioned is the same as
 * \ref t8_cmesh_set_num_trees and the values \a first_local_tree and
 * \a num_ghosts are ignored.
 * \param [in,out] cmesh        The cmesh to be updated.
 * \param [in]     set_partitioned A nonzero value specifies that \a cmesh
 *                              is interpreted as a partitioned mesh.
 * \param [in]     num_global_trees The global number of trees in the mesh.
 * \param [in]     first_local_tree The global index of the first tree on this process.
 *                                  Only used if \a set_partitioned is nonzero.
 * \param [in]     num_ghosts The number of trees from other processes that are
 *                            face-neighbor to trees on this process.
 *                            Only used if \a set_partitioned is nonzero.
 */
void                t8_cmesh_set_partitioned (t8_cmesh_t cmesh,
                                              int set_partitioned,
                                              t8_topidx_t num_global_trees,
                                              t8_topidx_t first_local_tree,
                                              t8_topidx_t num_ghosts);

/** Set the sizes of the attributes for each tree class and create the attribute mempools.
 * A size of zero can be set, in this case no mempool is created.
 * It is not allowed to call this function after \ref t8_cmesh_commit.
 * \param [in,out] cmesh    The cmesh to be updated.
 * \param [in]     attr_sizes An array of size_t specifying for each eclass the
 *                          size (in bytes) of the attributes.
 * \param [in]     num_sizes The number of entries in \a attr_sizes. Must match
 *                           \ref T8_ECLASS_LAST.
 */
void                t8_cmesh_set_attribute_sizes (t8_cmesh_t cmesh,
                                                  size_t attr_sizes[],
                                                  int num_sizes);

/** Set the size of the attribute for one specific tree class and create the attribute
 * mempool.
 * It is not allowed to call this function after \ref t8_cmesh_commit.
 * \param [in,out] cmesh    The cmesh to be updated.
 * \param [in]     attr_size The size (in bytes) to be set. Must be greater than 0.
 * \param [in]     tree_class The tree class for which the attribute size is to be set.
 */
void                t8_cmesh_set_attribute_size_single (t8_cmesh_t cmesh,
                                                        size_t attr_size,
                                                        t8_eclass_t
                                                        tree_class);

/** Set the sizes of the attributes to hold a \f$R^3\f$ coordinate for each tree corner.
 * It is not allowed to call this function after \ref t8_cmesh_commit.
 * \param [in,out] cmesh    The cmesh to be updated.
 */
void                t8_cmesh_set_attribute_to_vertices (t8_cmesh_t cmesh);

/** Set the number of trees for a cmesh.
 * It is not allowed to call this function after \ref t8_cmesh_commit.
 * \param [in,out] cmesh        The cmesh to be updated.
 * \param [in]     num_trees    The number of trees to be set.
 * \param [in]     num_trees_per_eclass An array storing for each t8_eclass
 *                              the number of trees of this class.
 */
void                t8_cmesh_set_num_trees (t8_cmesh_t cmesh,
                                            t8_topidx_t num_trees);

/** Set the class of a tree in the cmesh.
 * It is not allowed to call this function after \ref t8_cmesh_commit.
 * \param [in,out] cmesh        The cmesh to be updated.
 * \param [in]     tree_id      The global number of the tree.
 * \param [in]     tree_class   The element class of this tree.
 */
void                t8_cmesh_set_tree_class (t8_cmesh_t cmesh,
                                             t8_topidx_t tree_id,
                                             t8_eclass_t tree_class);

/** Set the vertices of a tree in the cmesh.
 * Before using this function, \ref t8_cmesh_set_attribute_to_vertices has to be called.
 * It is not allowed to call this function after \ref t8_cmesh_commit.
 * The eclass of the tree has to be set before calling this function.
 * \param [in,out] cmesh        The cmesh to be updated.
 * \param [in]     tree_id      The global number of the tree.
 * \param [in]     vertices     An array of as many vertex indices as the tree
 *                              has corners.
 * \param [in]     num_vertices The number of verticess in \a vertices. Must
 *                              match the number of corners of the tree.
 */
void                t8_cmesh_set_tree_vertices (t8_cmesh_t cmesh,
                                                t8_topidx_t tree_id,
                                                double *vertices,
                                                t8_topidx_t num_vertices);

/** Set the class of a ghost in a partitioned cmesh.
 * \param [in,out] cmesh        The cmesh to be updated.
 * \param [in]     ghost_id     The global number of the ghost.
 * \param [in]     ghost_eclass The element class of this ghost.
 */
void                t8_cmesh_set_ghost (t8_cmesh_t cmesh,
                                        t8_topidx_t ghost_id,
                                        t8_eclass_t ghost_eclass);

/** Insert a face-connection between two trees in a cmesh.
 * \param [in,out] cmesh        The cmesh to be updated.
 * \param [in]     tree1        The tree id of the first of the two trees.
 * \param [in]     tree2        The tree id of the second of the two trees.
 * \param [in]     face1        The face number of the first tree.
 * \param [in]     face2        The face number of the second tree.
 * \param [in]     orientation  Specify how face1 and face2 are oriented to each other
 * \param [in]     ghost_eclass If one of the trees is a ghost, the element
 *                              class of it has to be specified here.
 * TODO: document orientation
 */
void                t8_cmesh_join_faces (t8_cmesh_t cmesh, t8_topidx_t tree1,
                                         t8_topidx_t tree2, int face1,
                                         int face2, int orientation);

/* returns true if cmesh_a equals cmesh_b */
/* TODO: document */
/** Check whether two given cmeshes carry the same information.
 * \param [in]    cmesh_a       The first of the two cmeshes to be checked.
 * \param [in]    cmesh_b       The second of the two cmeshes to be checked.
 * \return                      True if both cmeshes carry the same information,
 *                              false otherwise.
 * Currently the attributes of the trees are not compared.
 */
int                 t8_cmesh_is_equal (t8_cmesh_t cmesh_a,
                                       t8_cmesh_t cmesh_b);

/** Broadcast a cmesh structure that exists only on one process to all
 *  processes in the cmesh's communicator.
 *  On the other processors, it will be allocated.
 *  It is not allowed to call this function after \ref t8_cmesh_commit.
 *  \param [in] cmesh_in For the root process the cmesh to be broadcast,
 *                      for the other processes it must be NULL.
 *  \param [in] root    The rank of the process that provides the cmesh.
 *  \param [in] comm    The mpi communicator. Must match cmesh's communicator
 *                      on the root process.
 *  \return             For the root process this is a pointer to \a cmesh_in.
 *                      Else, a pointer to a newly allocated cmesh
 *                      structure with the same values as \a conn_in on the
 *                      root process.
 */
t8_cmesh_t          t8_cmesh_bcast (t8_cmesh_t cmesh_in, int root,
                                    sc_MPI_Comm comm);

/** After allocating and adding properties to a cmesh, finish its construction.
 * \param [in,out] cmesh        Must be created with \see t8_cmesh_init and
 *                              specialized with t8_cmesh_set_* calls first.
 */
void                t8_cmesh_commit (t8_cmesh_t cmesh);

/** Return the eclass of a given tree.
 * \param [in]    cmesh         The cmesh to be considered.
 * \param [in]    tree_id       The id of the tree whose eclass will be returned.
 * \return                      The eclass of the given tree.
 * \a cmesh must be committed before calling this function.
 */
t8_eclass_t         t8_cmesh_get_tree_class (t8_cmesh_t cmesh,
                                             t8_topidx_t tree_id);

/** Return the global numbet of corners in a cmesh.
 * \param [in] cmesh       The cmesh to be considered.
 * \return                 The number of corners associated to \a cmesh.
 * \a cmesh must be committed before calling this function.
 */
t8_topidx_t         t8_cmesh_get_num_corners (t8_cmesh_t cmesh);

/** Return the global numbet of vertices in a cmesh.
 * \param [in] cmesh       The cmesh to be considered.
 * \return                 The number of vertices associated to \a cmesh.
 * \a cmesh must be committed before calling this function.
 */
t8_topidx_t         t8_cmesh_get_num_vertices (t8_cmesh_t cmesh);

/** Return the global number of trees in a cmesh.
 * \param [in] cmesh       The cmesh to be considered.
 * \return                 The number of trees associated to \a cmesh.
 * \a cmesh must be committed before calling this function.
 */
t8_topidx_t         t8_cmesh_get_num_trees (t8_cmesh_t cmesh);

/** Return the processor local number of trees in a cmesh.
 * If the cmesh is not partitioned this is the same as
 * the global number of trees.
 * \param [in] cmesh       The cmesh to be considered.
 * \return                 The number of trees associated to \a cmesh.
 * \a cmesh must be committed before calling this function.
 */
t8_topidx_t         t8_cmesh_get_local_num_trees (t8_cmesh_t cmesh);

/** Calculate the section of a uniform forest for the current rank.
 * TODO: this requires that cmesh knows its MPI communicator.
 * \param [in]    cmesh         The cmesh to be considered.
 * \param [in]    level         The uniform refinement level to be created.
 * \param [out]   first_local_tree  The first tree that contains elements belonging to the calling processor.
 * \param [out]   child_in_tree_begin The global index of the first element belonging to the calling processor.
 * \param [out]   last_local_tree  The last tree that contains elements belonging to the calling processor.
 * \param [out]   child_in_tree_end The global index of the last element belonging to the calling processor.
 */
void                t8_cmesh_uniform_bounds (t8_cmesh_t cmesh, int level,
                                             t8_topidx_t * first_local_tree,
                                             t8_gloidx_t *
                                             child_in_tree_begin,
                                             t8_topidx_t * last_local_tree,
                                             t8_gloidx_t * child_in_tree_end);

/** Increase the reference counter of a cmesh.
 * \param [in,out] cmesh        On input, this cmesh must exist with positive
 *                              reference count.  It may be in any state.
 */
void                t8_cmesh_ref (t8_cmesh_t cmesh);

/** Decrease the reference counter of a cmesh.
 * If the counter reaches zero, this cmesh is destroyed.
 * \param [in,out] pcmesh       On input, the cmesh pointed to must exist
 *                              with positive reference count.  It may be in
 *                              any state.  If the reference count reaches
 *                              zero, the cmesh is destroyed and this pointer
 *                              set to NULL.
 *                              Otherwise, the pointer is not changed and
 *                              the cmesh is not modified in other ways.
 */
void                t8_cmesh_unref (t8_cmesh_t * pcmesh);

/** Create a coarse mesh that consists of a single triangle.
 * \return          A valid cmesh, as if _init and _commit had been called.
 */
t8_cmesh_t          t8_cmesh_new_tri (sc_MPI_Comm comm, int do_dup);

/** Create a coarse mesh that consists of a single tetrahedron.
 * \return          A valid cmesh, as if _init and _commit had been called.
 */
t8_cmesh_t          t8_cmesh_new_tet (sc_MPI_Comm comm, int do_dup);

/** Create a coarse mesh that consists of a single square.
 * \return          A valid cmesh, as if _init and _commit had been called.
 */
t8_cmesh_t          t8_cmesh_new_quad (sc_MPI_Comm comm, int do_dup);

/** Create a coarse mesh that consists of a single hexahedron.
 * \return          A valid cmesh, as if _init and _commit had been called.
 */
t8_cmesh_t          t8_cmesh_new_hex (sc_MPI_Comm comm, int do_dup);

/** Construct a hypercube forest from one primitive tree class.
 * \param [in] eclass       This element class determines the dimension and
 *                          the number of trees needed to construct a cube.
 * \param [in] comm         The mpi communicator to be used.
 * \param [in] do_dup       Whether \a comm is to be duplicated.
 * \param [in] do_bcast     If this flag is nonzero the cmesh is only constructed
 *                          on processor 0 and then broadcasted to the other
 *                          processors in \a comm.
 */
t8_cmesh_t          t8_cmesh_new_hypercube (t8_eclass_t eclass,
                                            sc_MPI_Comm comm, int do_dup,
                                            int do_bcast);

/** Construct a unit interval/square/cube forest that is periodic in each direction.
 * \param [in] comm         The mpi communicator to use.
 * \param [in] do_dup       Whether the mpi communicator is to be duplicated.
 * \param [in] dim          The dimension of the forest, 1, 2 or 3.
 * \return                  A valid cmesh, as if _init and _commit had been called.
 */
t8_cmesh_t          t8_cmesh_new_periodic (sc_MPI_Comm comm, int do_dup,
                                           int dim);

T8_EXTERN_C_END ();

#endif /* !T8_CMESH_H */
