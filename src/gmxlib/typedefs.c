/*
 * $Id$
 * 
 *                This source code is part of
 * 
 *                 G   R   O   M   A   C   S
 * 
 *          GROningen MAchine for Chemical Simulations
 * 
 *                        VERSION 3.2.0
 * Written by David van der Spoel, Erik Lindahl, Berk Hess, and others.
 * Copyright (c) 1991-2000, University of Groningen, The Netherlands.
 * Copyright (c) 2001-2004, The GROMACS development team,
 * check out http://www.gromacs.org for more information.

 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * If you want to redistribute modifications, please consider that
 * scientific software is very special. Version control is crucial -
 * bugs must be traceable. We will be happy to consider code for
 * inclusion in the official distribution, but derived work must not
 * be called official GROMACS. Details are found in the README & COPYING
 * files - if they are missing, get the official version at www.gromacs.org.
 * 
 * To help us fund GROMACS development, we humbly ask that you cite
 * the papers on the package - you can find them in the top README file.
 * 
 * For more info, check our website at http://www.gromacs.org
 * 
 * And Hey:
 * GROningen Mixture of Alchemy and Childrens' Stories
 */
/* This file is completely threadsafe - keep it that way! */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "smalloc.h"
#include "assert.h"
#include "symtab.h"
#include "vec.h"
#include <string.h>

void init_block(t_block *block)
{
  int i;

  block->nr    = 0;
  block->nra   = 0;
  snew(block->index,1);
  block->index[0] = 0;
  block->a     = NULL;
  for(i=0; (i<MAXNODES); i++)
    block->multinr[i]=0;
}

void init_atom(t_atoms *at)
{
  int i;
  
  init_block(&(at->excl));
  at->nr       = 0;
  at->nres     = 0;
  at->ngrpname = 0;
  at->atom     = NULL;
  at->resname  = NULL;
  at->atomname = NULL;
  at->atomtype = NULL;
  at->atomtypeB= NULL;
  at->grpname  = NULL;
  at->pdbinfo  = NULL;
  for(i=0; (i<egcNR); i++) {
    at->grps[i].nr=0;
    at->grps[i].nm_ind=NULL;
  }
}

void init_top (t_topology *top)
{
  int i;
  
  top->name = NULL;
  open_symtab(&top->symtab);
  init_atom (&(top->atoms));
  for (i=0; (i<ebNR); i++)
    init_block(&(top->blocks[i]));
}

void init_inputrec(t_inputrec *ir)
{
  memset(ir,0,(size_t)sizeof(*ir));
}

void stupid_fill(t_block *grp,int natom,bool bOneIndexGroup)
{
  int i;

  snew(grp->a,natom);
  for(i=0; (i<natom); i++)
    grp->a[i]=i;
  grp->nra=natom;
  
  if (bOneIndexGroup) {
    snew(grp->index,2);
    grp->index[0]=0;
    grp->index[1]=natom;
    grp->nr=1;
  }
  else {
    snew(grp->index,natom+1);
    for(i=0; (i<=natom); i++)
      grp->index[i]=i;
    grp->nr=natom;
  }
  grp->multinr[0] = natom-1;
}

void done_block(t_block *block)
{
  block->nr    = 0;
  block->nra   = 0;
  sfree(block->index);
  sfree(block->a);
}

void done_atom (t_atoms *at)
{
  done_block(&(at->excl));
  at->nr       = 0;
  at->nres     = 0;
  sfree(at->atom);
  sfree(at->resname);
  sfree(at->atomname);
}

void done_top(t_topology *top)
{
  int i;
  
  done_atom (&(top->atoms));
  done_symtab(&(top->symtab));
  for (i=0; (i<ebNR); i++)
    done_block(&(top->blocks[i]));
}

void done_inputrec(t_inputrec *ir)
{
  int m;
  
  for(m=0; (m<DIM); m++) {
    if (ir->ex[m].a)   sfree(ir->ex[m].a);
    if (ir->ex[m].phi) sfree(ir->ex[m].phi);
    if (ir->et[m].a)   sfree(ir->et[m].a);
    if (ir->et[m].phi) sfree(ir->et[m].phi);
  }
  if (ir->opts.nrdf)    sfree(ir->opts.nrdf);
  if (ir->opts.ref_t)   sfree(ir->opts.ref_t);
  if (ir->opts.tau_t)   sfree(ir->opts.tau_t);
  if (ir->opts.acc)     sfree(ir->opts.acc);
  if (ir->opts.nFreeze) sfree(ir->opts.nFreeze);
}

void init_gtc_state(t_state *state,int ngtc)
{
  int i;

  state->ngtc = ngtc;
  if (state->ngtc > 0) {
    snew(state->nosehoover_xi,state->ngtc);
    snew(state->tcoupl_lambda,state->ngtc);
    for(i=0; i<state->ngtc; i++) {
      state->nosehoover_xi[i] = 0.0;
      state->tcoupl_lambda[i] = 1.0;
    }
  } else {
    state->nosehoover_xi = NULL;
    state->tcoupl_lambda = NULL;
  }
}

void init_state(t_state *state,int natoms,int ngtc)
{
  int i;

  state->natoms = natoms;
  state->lambda = 0;
  clear_mat(state->box);
  clear_mat(state->boxv);
  clear_mat(state->pcoupl_mu);
  for(i=0; i<DIM; i++)
    state->pcoupl_mu[i][i] = 1.0;
  init_gtc_state(state,ngtc);
  if (state->natoms > 0) {
    snew(state->x,state->natoms);
    snew(state->v,state->natoms);
  } else {
    state->x = NULL;
    state->v = NULL;
  }
}

void done_state(t_state *state)
{
  if (state->nosehoover_xi) sfree(state->nosehoover_xi);
  if (state->tcoupl_lambda) sfree(state->tcoupl_lambda);
  if (state->x) sfree(state->x);
  if (state->v) sfree(state->v);
}

void init_t_atoms(t_atoms *atoms, int natoms, bool bPdbinfo)
{
  atoms->nr=natoms;
  atoms->nres=0;
  atoms->ngrpname=0;
  snew(atoms->atomname,natoms);
  atoms->atomtype=NULL;
  atoms->atomtypeB=NULL;
  snew(atoms->resname,natoms);
  snew(atoms->atom,natoms);
  snew(atoms->grpname,natoms);
  if (bPdbinfo)
    snew(atoms->pdbinfo,natoms);
  else
    atoms->pdbinfo=NULL;
  init_block(&atoms->excl);
}

void free_t_atoms(t_atoms *atoms)
{
  int i;

  for(i=0; i<atoms->nr; i++) {
    sfree(*atoms->atomname[i]);
    *atoms->atomname[i]=NULL;
  }
  sfree(atoms->atomname);
  /* Do we need to free atomtype and atomtypeB as well ? */
  sfree(atoms->resname);
  sfree(atoms->atom);
  if (atoms->pdbinfo)
    sfree(atoms->pdbinfo);
  atoms->nr=0; 
  atoms->nres=0;
  done_block(&atoms->excl);
}     

