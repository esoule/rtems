/**
 * @file
 *
 * @brief SMP Scheduler Implementation
 *
 * @ingroup ScoreSchedulerSMP
 */

/*
 * Copyright (c) 2013-2014 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Dornierstr. 4
 *  82178 Puchheim
 *  Germany
 *  <rtems@embedded-brains.de>
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rtems.org/license/LICENSE.
 */

#ifndef _RTEMS_SCORE_SCHEDULERSMPIMPL_H
#define _RTEMS_SCORE_SCHEDULERSMPIMPL_H

#include <rtems/score/schedulersmp.h>
#include <rtems/score/assert.h>
#include <rtems/score/chainimpl.h>
#include <rtems/score/schedulersimpleimpl.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @addtogroup ScoreSchedulerSMP
 *
 * The scheduler nodes can be in four states
 * - @ref SCHEDULER_SMP_NODE_BLOCKED,
 * - @ref SCHEDULER_SMP_NODE_SCHEDULED, and
 * - @ref SCHEDULER_SMP_NODE_READY.
 *
 * State transitions are triggered via basic operations
 * - _Scheduler_SMP_Enqueue_ordered(),
 * - _Scheduler_SMP_Enqueue_scheduled_ordered(), and
 * - _Scheduler_SMP_Block().
 *
 * @dot
 * digraph {
 *   node [style="filled"];
 *
 *   bs [label="BLOCKED"];
 *   ss [label="SCHEDULED", fillcolor="green"];
 *   rs [label="READY", fillcolor="red"];
 *
 *   edge [label="enqueue"];
 *   edge [fontcolor="darkgreen", color="darkgreen"];
 *
 *   bs -> ss;
 *
 *   edge [fontcolor="red", color="red"];
 *
 *   bs -> rs;
 *
 *   edge [label="enqueue other"];
 *
 *   ss -> rs;
 *
 *   edge [label="block"];
 *   edge [fontcolor="black", color="black"];
 *
 *   ss -> bs;
 *   rs -> bs;
 *
 *   edge [label="block other"];
 *   edge [fontcolor="darkgreen", color="darkgreen"];
 *
 *   rs -> ss;
 * }
 * @enddot
 *
 * During system initialization each processor of the scheduler instance starts
 * with an idle thread assigned to it.  Lets have a look at an example with two
 * idle threads I and J with priority 5.  We also have blocked threads A, B and
 * C with priorities 1, 2 and 3 respectively.  The scheduler nodes are ordered
 * with respect to the thread priority from left to right in the below
 * diagrams.  The highest priority node (lowest priority number) is the
 * leftmost node.  Since the processor assignment is independent of the thread
 * priority the processor indices may move from one state to the other.
 *
 * @dot
 * digraph {
 *   node [style="filled"];
 *   edge [dir="none"];
 *   subgraph {
 *     rank = same;
 *
 *     i [label="I (5)", fillcolor="green"];
 *     j [label="J (5)", fillcolor="green"];
 *     a [label="A (1)"];
 *     b [label="B (2)"];
 *     c [label="C (3)"];
 *     i -> j;
 *   }
 *
 *   subgraph {
 *     rank = same;
 *
 *     p0 [label="PROCESSOR 0", shape="box"];
 *     p1 [label="PROCESSOR 1", shape="box"];
 *   }
 *
 *   i -> p0;
 *   j -> p1;
 * }
 * @enddot
 *
 * Lets start A.  For this an enqueue operation is performed.
 *
 * @dot
 * digraph {
 *   node [style="filled"];
 *   edge [dir="none"];
 *
 *   subgraph {
 *     rank = same;
 *
 *     i [label="I (5)", fillcolor="green"];
 *     j [label="J (5)", fillcolor="red"];
 *     a [label="A (1)", fillcolor="green"];
 *     b [label="B (2)"];
 *     c [label="C (3)"];
 *     a -> i;
 *   }
 *
 *   subgraph {
 *     rank = same;
 *
 *     p0 [label="PROCESSOR 0", shape="box"];
 *     p1 [label="PROCESSOR 1", shape="box"];
 *   }
 *
 *   i -> p0;
 *   a -> p1;
 * }
 * @enddot
 *
 * Lets start C.
 *
 * @dot
 * digraph {
 *   node [style="filled"];
 *   edge [dir="none"];
 *
 *   subgraph {
 *     rank = same;
 *
 *     a [label="A (1)", fillcolor="green"];
 *     c [label="C (3)", fillcolor="green"];
 *     i [label="I (5)", fillcolor="red"];
 *     j [label="J (5)", fillcolor="red"];
 *     b [label="B (2)"];
 *     a -> c;
 *     i -> j;
 *   }
 *
 *   subgraph {
 *     rank = same;
 *
 *     p0 [label="PROCESSOR 0", shape="box"];
 *     p1 [label="PROCESSOR 1", shape="box"];
 *   }
 *
 *   c -> p0;
 *   a -> p1;
 * }
 * @enddot
 *
 * Lets start B.
 *
 * @dot
 * digraph {
 *   node [style="filled"];
 *   edge [dir="none"];
 *
 *   subgraph {
 *     rank = same;
 *
 *     a [label="A (1)", fillcolor="green"];
 *     b [label="B (2)", fillcolor="green"];
 *     c [label="C (3)", fillcolor="red"];
 *     i [label="I (5)", fillcolor="red"];
 *     j [label="J (5)", fillcolor="red"];
 *     a -> b;
 *     c -> i -> j;
 *   }
 *
 *   subgraph {
 *     rank = same;
 *
 *     p0 [label="PROCESSOR 0", shape="box"];
 *     p1 [label="PROCESSOR 1", shape="box"];
 *   }
 *
 *   b -> p0;
 *   a -> p1;
 * }
 * @enddot
 *
 * Lets change the priority of thread A to 4.
 *
 * @dot
 * digraph {
 *   node [style="filled"];
 *   edge [dir="none"];
 *
 *   subgraph {
 *     rank = same;
 *
 *     b [label="B (2)", fillcolor="green"];
 *     c [label="C (3)", fillcolor="green"];
 *     a [label="A (4)", fillcolor="red"];
 *     i [label="I (5)", fillcolor="red"];
 *     j [label="J (5)", fillcolor="red"];
 *     b -> c;
 *     a -> i -> j;
 *   }
 *
 *   subgraph {
 *     rank = same;
 *
 *     p0 [label="PROCESSOR 0", shape="box"];
 *     p1 [label="PROCESSOR 1", shape="box"];
 *   }
 *
 *   b -> p0;
 *   c -> p1;
 * }
 * @enddot
 *
 * Now perform a blocking operation with thread B.  Please note that thread A
 * migrated now from processor 0 to processor 1 and thread C still executes on
 * processor 1.
 *
 * @dot
 * digraph {
 *   node [style="filled"];
 *   edge [dir="none"];
 *
 *   subgraph {
 *     rank = same;
 *
 *     c [label="C (3)", fillcolor="green"];
 *     a [label="A (4)", fillcolor="green"];
 *     i [label="I (5)", fillcolor="red"];
 *     j [label="J (5)", fillcolor="red"];
 *     b [label="B (2)"];
 *     c -> a;
 *     i -> j;
 *   }
 *
 *   subgraph {
 *     rank = same;
 *
 *     p0 [label="PROCESSOR 0", shape="box"];
 *     p1 [label="PROCESSOR 1", shape="box"];
 *   }
 *
 *   a -> p0;
 *   c -> p1;
 * }
 * @enddot
 *
 * @{
 */

typedef Scheduler_Node *( *Scheduler_SMP_Get_highest_ready )(
  Scheduler_Context *context,
  Scheduler_Node    *node
);

typedef Scheduler_Node *( *Scheduler_SMP_Get_lowest_scheduled )(
  Scheduler_Context *context,
  Scheduler_Node    *filter,
  Chain_Node_order   order
);

typedef void ( *Scheduler_SMP_Extract )(
  Scheduler_Context *context,
  Scheduler_Node    *node_to_extract
);

typedef void ( *Scheduler_SMP_Insert )(
  Scheduler_Context *context,
  Scheduler_Node    *node_to_insert
);

typedef void ( *Scheduler_SMP_Move )(
  Scheduler_Context *context,
  Scheduler_Node    *node_to_move
);

typedef void ( *Scheduler_SMP_Update )(
  Scheduler_Context *context,
  Scheduler_Node    *node_to_update,
  Priority_Control   new_priority
);

typedef void ( *Scheduler_SMP_Enqueue )(
  Scheduler_Context *context,
  Scheduler_Node    *node_to_enqueue
);

typedef void ( *Scheduler_SMP_Allocate_processor )(
  Scheduler_Context *context,
  Scheduler_Node    *scheduled,
  Scheduler_Node    *victim
);

static inline bool _Scheduler_SMP_Insert_priority_lifo_order(
  const Chain_Node *to_insert,
  const Chain_Node *next
)
{
  const Scheduler_SMP_Node *node_to_insert =
    (const Scheduler_SMP_Node *) to_insert;
  const Scheduler_SMP_Node *node_next =
    (const Scheduler_SMP_Node *) next;

  return node_to_insert->priority <= node_next->priority;
}

static inline bool _Scheduler_SMP_Insert_priority_fifo_order(
  const Chain_Node *to_insert,
  const Chain_Node *next
)
{
  const Scheduler_SMP_Node *node_to_insert =
    (const Scheduler_SMP_Node *) to_insert;
  const Scheduler_SMP_Node *node_next =
    (const Scheduler_SMP_Node *) next;

  return node_to_insert->priority < node_next->priority;
}

static inline Scheduler_SMP_Context *_Scheduler_SMP_Get_self(
  Scheduler_Context *context
)
{
  return (Scheduler_SMP_Context *) context;
}

static inline void _Scheduler_SMP_Initialize(
  Scheduler_SMP_Context *self
)
{
  _Chain_Initialize_empty( &self->Scheduled );
}

static inline Scheduler_SMP_Node *_Scheduler_SMP_Node_get(
  Thread_Control *thread
)
{
  return (Scheduler_SMP_Node *) _Scheduler_Node_get( thread );
}

static inline Scheduler_SMP_Node *_Scheduler_SMP_Node_downcast(
  Scheduler_Node *node
)
{
  return (Scheduler_SMP_Node *) node;
}

static inline void _Scheduler_SMP_Node_initialize(
  Scheduler_SMP_Node *node,
  Thread_Control     *thread
)
{
  _Scheduler_Node_do_initialize( &node->Base, thread );
  node->state = SCHEDULER_SMP_NODE_BLOCKED;
}

static inline void _Scheduler_SMP_Node_update_priority(
  Scheduler_SMP_Node *node,
  Priority_Control    new_priority
)
{
  node->priority = new_priority;
}

extern const bool _Scheduler_SMP_Node_valid_state_changes[ 3 ][ 3 ];

static inline void _Scheduler_SMP_Node_change_state(
  Scheduler_SMP_Node      *node,
  Scheduler_SMP_Node_state new_state
)
{
  _Assert(
    _Scheduler_SMP_Node_valid_state_changes[ node->state ][ new_state ]
  );

  node->state = new_state;
}

static inline bool _Scheduler_SMP_Is_processor_owned_by_us(
  const Scheduler_Context *context,
  const Per_CPU_Control   *cpu
)
{
  return cpu->scheduler_context == context;
}

static inline void _Scheduler_SMP_Update_heir(
  Per_CPU_Control *cpu_self,
  Per_CPU_Control *cpu_for_heir,
  Thread_Control  *heir
)
{
  cpu_for_heir->heir = heir;

  /*
   * It is critical that we first update the heir and then the dispatch
   * necessary so that _Thread_Get_heir_and_make_it_executing() cannot miss an
   * update.
   */
  _Atomic_Fence( ATOMIC_ORDER_SEQ_CST );

  /*
   * Only update the dispatch necessary indicator if not already set to
   * avoid superfluous inter-processor interrupts.
   */
  if ( !cpu_for_heir->dispatch_necessary ) {
    cpu_for_heir->dispatch_necessary = true;

    if ( cpu_for_heir != cpu_self ) {
      _Per_CPU_Send_interrupt( cpu_for_heir );
    }
  }
}

static inline void _Scheduler_SMP_Allocate_processor(
  Scheduler_Context *context,
  Scheduler_Node    *scheduled,
  Scheduler_Node    *victim
)
{
  Thread_Control *scheduled_thread = _Scheduler_Node_get_owner( scheduled );
  Thread_Control *victim_thread = _Scheduler_Node_get_owner( victim );
  Per_CPU_Control *scheduled_cpu = _Thread_Get_CPU( scheduled_thread );
  Per_CPU_Control *victim_cpu = _Thread_Get_CPU( victim_thread );
  Per_CPU_Control *cpu_self = _Per_CPU_Get();
  Thread_Control *heir;

  _Scheduler_SMP_Node_change_state(
    _Scheduler_SMP_Node_downcast( scheduled ),
    SCHEDULER_SMP_NODE_SCHEDULED
  );

  _Assert( _ISR_Get_level() != 0 );

  if ( _Thread_Is_executing_on_a_processor( scheduled_thread ) ) {
    if ( _Scheduler_SMP_Is_processor_owned_by_us( context, scheduled_cpu ) ) {
      heir = scheduled_cpu->heir;
      _Scheduler_SMP_Update_heir(
        cpu_self,
        scheduled_cpu,
        scheduled_thread
      );
    } else {
      /* We have to force a migration to our processor set */
      _Assert(
        scheduled_thread->Scheduler.debug_real_cpu->heir != scheduled_thread
      );
      heir = scheduled_thread;
    }
  } else {
    heir = scheduled_thread;
  }

  if ( heir != victim_thread ) {
    _Thread_Set_CPU( heir, victim_cpu );
    _Scheduler_SMP_Update_heir( cpu_self, victim_cpu, heir );
  }
}

static inline Scheduler_Node *_Scheduler_SMP_Get_lowest_scheduled(
  Scheduler_Context *context,
  Scheduler_Node    *filter,
  Chain_Node_order   order
)
{
  Scheduler_SMP_Context *self = _Scheduler_SMP_Get_self( context );
  Chain_Control *scheduled = &self->Scheduled;
  Scheduler_Node *lowest_scheduled =
    (Scheduler_Node *) _Chain_Last( scheduled );

  (void) filter;
  (void) order;

  _Assert( &lowest_scheduled->Node != _Chain_Tail( scheduled ) );

  return lowest_scheduled;
}

/**
 * @brief Enqueues a node according to the specified order function.
 *
 * The node must not be in the scheduled state.
 *
 * @param[in] context The scheduler instance context.
 * @param[in] node The node to enqueue.
 * @param[in] order The order function.
 * @param[in] insert_ready Function to insert a node into the set of ready
 *   nodes.
 * @param[in] insert_scheduled Function to insert a node into the set of
 *   scheduled nodes.
 * @param[in] move_from_scheduled_to_ready Function to move a node from the set
 *   of scheduled nodes to the set of ready nodes.
 * @param[in] get_lowest_scheduled Function to select the node from the
 *   scheduled nodes to replace.  It may not be possible to find one, in this
 *   case a pointer must be returned so that the order functions returns false
 *   if this pointer is passed as the second argument to the order function.
 * @param[in] allocate_processor Function to allocate a processor to a node
 *   based on the rules of the scheduler.
 */
static inline void _Scheduler_SMP_Enqueue_ordered(
  Scheduler_Context                  *context,
  Scheduler_Node                     *node,
  Chain_Node_order                    order,
  Scheduler_SMP_Insert                insert_ready,
  Scheduler_SMP_Insert                insert_scheduled,
  Scheduler_SMP_Move                  move_from_scheduled_to_ready,
  Scheduler_SMP_Get_lowest_scheduled  get_lowest_scheduled,
  Scheduler_SMP_Allocate_processor    allocate_processor
)
{
  Scheduler_Node *lowest_scheduled =
    ( *get_lowest_scheduled )( context, node, order );

  if ( ( *order )( &node->Node, &lowest_scheduled->Node ) ) {
    _Scheduler_SMP_Node_change_state(
      _Scheduler_SMP_Node_downcast( lowest_scheduled ),
      SCHEDULER_SMP_NODE_READY
    );
    ( *allocate_processor )( context, node, lowest_scheduled );
    ( *insert_scheduled )( context, node );
    ( *move_from_scheduled_to_ready )( context, lowest_scheduled );
  } else {
    ( *insert_ready )( context, node );
  }
}

/**
 * @brief Enqueues a scheduled node according to the specified order
 * function.
 *
 * @param[in] context The scheduler instance context.
 * @param[in] node The node to enqueue.
 * @param[in] order The order function.
 * @param[in] get_highest_ready Function to get the highest ready node.
 * @param[in] insert_ready Function to insert a node into the set of ready
 *   nodes.
 * @param[in] insert_scheduled Function to insert a node into the set of
 *   scheduled nodes.
 * @param[in] move_from_ready_to_scheduled Function to move a node from the set
 *   of ready nodes to the set of scheduled nodes.
 * @param[in] allocate_processor Function to allocate a processor to a node
 *   based on the rules of the scheduler.
 */
static inline void _Scheduler_SMP_Enqueue_scheduled_ordered(
  Scheduler_Context                *context,
  Scheduler_Node                   *node,
  Chain_Node_order                  order,
  Scheduler_SMP_Get_highest_ready   get_highest_ready,
  Scheduler_SMP_Insert              insert_ready,
  Scheduler_SMP_Insert              insert_scheduled,
  Scheduler_SMP_Move                move_from_ready_to_scheduled,
  Scheduler_SMP_Allocate_processor  allocate_processor
)
{
  Scheduler_Node *highest_ready = ( *get_highest_ready )( context, node );

  _Assert( highest_ready != NULL );

  /*
   * The node has been extracted from the scheduled chain.  We have to place
   * it now on the scheduled or ready set.
   */
  if ( ( *order )( &node->Node, &highest_ready->Node ) ) {
    ( *insert_scheduled )( context, node );
  } else {
    _Scheduler_SMP_Node_change_state(
      _Scheduler_SMP_Node_downcast( node ),
      SCHEDULER_SMP_NODE_READY
    );
    ( *allocate_processor) ( context, highest_ready, node );
    ( *insert_ready )( context, node );
    ( *move_from_ready_to_scheduled )( context, highest_ready );
  }
}

static inline void _Scheduler_SMP_Extract_from_scheduled(
  Scheduler_Node *node
)
{
  _Chain_Extract_unprotected( &node->Node );
}

static inline void _Scheduler_SMP_Schedule_highest_ready(
  Scheduler_Context                *context,
  Scheduler_Node                   *victim,
  Scheduler_SMP_Get_highest_ready   get_highest_ready,
  Scheduler_SMP_Move                move_from_ready_to_scheduled,
  Scheduler_SMP_Allocate_processor  allocate_processor
)
{
  Scheduler_Node *highest_ready = ( *get_highest_ready )( context, victim );

  ( *allocate_processor )( context, highest_ready, victim );
  ( *move_from_ready_to_scheduled )( context, highest_ready );
}

/**
 * @brief Blocks a thread.
 *
 * @param[in] context The scheduler instance context.
 * @param[in] thread The thread of the scheduling operation.
 * @param[in] extract_from_ready Function to extract a node from the set of
 * ready nodes.
 * @param[in] get_highest_ready Function to get the highest ready node.
 * @param[in] move_from_ready_to_scheduled Function to move a node from the set
 * of ready nodes to the set of scheduled nodes.
 */
static inline void _Scheduler_SMP_Block(
  Scheduler_Context                *context,
  Thread_Control                   *thread,
  Scheduler_SMP_Extract             extract_from_ready,
  Scheduler_SMP_Get_highest_ready   get_highest_ready,
  Scheduler_SMP_Move                move_from_ready_to_scheduled,
  Scheduler_SMP_Allocate_processor  allocate_processor
)
{
  Scheduler_SMP_Node *node = _Scheduler_SMP_Node_get( thread );
  bool is_scheduled = node->state == SCHEDULER_SMP_NODE_SCHEDULED;

  _Scheduler_SMP_Node_change_state( node, SCHEDULER_SMP_NODE_BLOCKED );

  if ( is_scheduled ) {
    _Scheduler_SMP_Extract_from_scheduled( &node->Base );

    _Scheduler_SMP_Schedule_highest_ready(
      context,
      &node->Base,
      get_highest_ready,
      move_from_ready_to_scheduled,
      allocate_processor
    );
  } else {
    ( *extract_from_ready )( context, &node->Base );
  }
}

static inline void _Scheduler_SMP_Unblock(
  Scheduler_Context     *context,
  Thread_Control        *thread,
  Scheduler_SMP_Enqueue  enqueue_fifo
)
{
  Scheduler_SMP_Node *node = _Scheduler_SMP_Node_get( thread );

  _Scheduler_SMP_Node_change_state( node, SCHEDULER_SMP_NODE_READY );

  ( *enqueue_fifo )( context, &node->Base );
}

static inline void _Scheduler_SMP_Change_priority(
  Scheduler_Context     *context,
  Thread_Control        *thread,
  Priority_Control       new_priority,
  bool                   prepend_it,
  Scheduler_SMP_Extract  extract_from_ready,
  Scheduler_SMP_Update   update,
  Scheduler_SMP_Enqueue  enqueue_fifo,
  Scheduler_SMP_Enqueue  enqueue_lifo,
  Scheduler_SMP_Enqueue  enqueue_scheduled_fifo,
  Scheduler_SMP_Enqueue  enqueue_scheduled_lifo
)
{
  Scheduler_SMP_Node *node = _Scheduler_SMP_Node_get( thread );

  if ( node->state == SCHEDULER_SMP_NODE_SCHEDULED ) {
    _Scheduler_SMP_Extract_from_scheduled( &node->Base );

    ( *update )( context, &node->Base, new_priority );

    if ( prepend_it ) {
      ( *enqueue_scheduled_lifo )( context, &node->Base );
    } else {
      ( *enqueue_scheduled_fifo )( context, &node->Base );
    }
  } else {
    ( *extract_from_ready )( context, &node->Base );

    ( *update )( context, &node->Base, new_priority );

    if ( prepend_it ) {
      ( *enqueue_lifo )( context, &node->Base );
    } else {
      ( *enqueue_fifo )( context, &node->Base );
    }
  }
}

static inline void _Scheduler_SMP_Yield(
  Scheduler_Context     *context,
  Thread_Control        *thread,
  Scheduler_SMP_Extract  extract_from_ready,
  Scheduler_SMP_Enqueue  enqueue_fifo,
  Scheduler_SMP_Enqueue  enqueue_scheduled_fifo
)
{
  Scheduler_SMP_Node *node = _Scheduler_SMP_Node_get( thread );

  if ( node->state == SCHEDULER_SMP_NODE_SCHEDULED ) {
    _Scheduler_SMP_Extract_from_scheduled( &node->Base );

    ( *enqueue_scheduled_fifo )( context, &node->Base );
  } else {
    ( *extract_from_ready )( context, &node->Base );

    ( *enqueue_fifo )( context, &node->Base );
  }
}

static inline void _Scheduler_SMP_Insert_scheduled_lifo(
  Scheduler_Context *context,
  Scheduler_Node    *node_to_insert
)
{
  Scheduler_SMP_Context *self = _Scheduler_SMP_Get_self( context );

  _Chain_Insert_ordered_unprotected(
    &self->Scheduled,
    &node_to_insert->Node,
    _Scheduler_SMP_Insert_priority_lifo_order
  );
}

static inline void _Scheduler_SMP_Insert_scheduled_fifo(
  Scheduler_Context *context,
  Scheduler_Node    *node_to_insert
)
{
  Scheduler_SMP_Context *self = _Scheduler_SMP_Get_self( context );

  _Chain_Insert_ordered_unprotected(
    &self->Scheduled,
    &node_to_insert->Node,
    _Scheduler_SMP_Insert_priority_fifo_order
  );
}

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _RTEMS_SCORE_SCHEDULERSMPIMPL_H */
