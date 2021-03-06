/**
 * @file
 *
 * @brief Inlined Routines Associated with the Manipulation of the Scheduler
 *
 * This inline file contains all of the inlined routines associated with
 * the manipulation of the scheduler.
 */

/*
 *  Copyright (C) 2010 Gedare Bloom.
 *  Copyright (C) 2011 On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.org/license/LICENSE.
 */

#ifndef _RTEMS_SCORE_SCHEDULERIMPL_H
#define _RTEMS_SCORE_SCHEDULERIMPL_H

#include <rtems/score/scheduler.h>
#include <rtems/score/cpusetimpl.h>
#include <rtems/score/smpimpl.h>
#include <rtems/score/threadimpl.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup ScoreScheduler
 */
/**@{**/

/**
 *  @brief Initializes the scheduler to the policy chosen by the user.
 *
 *  This routine initializes the scheduler to the policy chosen by the user
 *  through confdefs, or to the priority scheduler with ready chains by
 *  default.
 */
void _Scheduler_Handler_initialization( void );

RTEMS_INLINE_ROUTINE const Scheduler_Control *_Scheduler_Get(
  const Thread_Control *the_thread
)
{
#if defined(RTEMS_SMP)
  return the_thread->Scheduler.control;
#else
  (void) the_thread;

  return &_Scheduler_Table[ 0 ];
#endif
}

RTEMS_INLINE_ROUTINE const Scheduler_Control *_Scheduler_Get_by_CPU_index(
  uint32_t cpu_index
)
{
#if defined(RTEMS_SMP)
  return _Scheduler_Assignments[ cpu_index ].scheduler;
#else
  (void) cpu_index;

  return &_Scheduler_Table[ 0 ];
#endif
}

RTEMS_INLINE_ROUTINE const Scheduler_Control *_Scheduler_Get_by_CPU(
  const Per_CPU_Control *cpu
)
{
  uint32_t cpu_index = _Per_CPU_Get_index( cpu );

  return _Scheduler_Get_by_CPU_index( cpu_index );
}

/**
 * The preferred method to add a new scheduler is to define the jump table
 * entries and add a case to the _Scheduler_Initialize routine.
 *
 * Generic scheduling implementations that rely on the ready queue only can
 * be found in the _Scheduler_queue_XXX functions.
 */

/*
 * Passing the Scheduler_Control* to these functions allows for multiple
 * scheduler's to exist simultaneously, which could be useful on an SMP
 * system.  Then remote Schedulers may be accessible.  How to protect such
 * accesses remains an open problem.
 */

/**
 * @brief General scheduling decision.
 *
 * This kernel routine implements the scheduling decision logic for
 * the scheduler. It does NOT dispatch.
 *
 * @param[in] the_thread The thread which state changed previously.
 */
RTEMS_INLINE_ROUTINE void _Scheduler_Schedule( Thread_Control *the_thread )
{
  const Scheduler_Control *scheduler = _Scheduler_Get( the_thread );

  ( *scheduler->Operations.schedule )( scheduler, the_thread );
}

/**
 * @brief Scheduler yield with a particular thread.
 *
 * This routine is invoked when a thread wishes to voluntarily transfer control
 * of the processor to another thread.
 *
 * @param[in] the_thread The yielding thread.
 */
RTEMS_INLINE_ROUTINE void _Scheduler_Yield( Thread_Control *the_thread )
{
  const Scheduler_Control *scheduler = _Scheduler_Get( the_thread );

  ( *scheduler->Operations.yield )( scheduler, the_thread );
}

/**
 * @brief Blocks a thread with respect to the scheduler.
 *
 * This routine removes @a the_thread from the scheduling decision for
 * the scheduler. The primary task is to remove the thread from the
 * ready queue.  It performs any necessary schedulering operations
 * including the selection of a new heir thread.
 *
 * @param[in] the_thread The thread.
 */
RTEMS_INLINE_ROUTINE void _Scheduler_Block( Thread_Control *the_thread )
{
  const Scheduler_Control *scheduler = _Scheduler_Get( the_thread );

  ( *scheduler->Operations.block )( scheduler, the_thread );
}

/**
 * @brief Unblocks a thread with respect to the scheduler.
 *
 * This routine adds @a the_thread to the scheduling decision for
 * the scheduler.  The primary task is to add the thread to the
 * ready queue per the schedulering policy and update any appropriate
 * scheduling variables, for example the heir thread.
 *
 * @param[in] the_thread The thread.
 */
RTEMS_INLINE_ROUTINE void _Scheduler_Unblock( Thread_Control *the_thread )
{
  const Scheduler_Control *scheduler = _Scheduler_Get( the_thread );

  ( *scheduler->Operations.unblock )( scheduler, the_thread );
}

/**
 * @brief Propagates a priority change of a thread to the scheduler.
 *
 * The caller must ensure that the thread is in the ready state.  The caller
 * must ensure that the priority value actually changed and is not equal to the
 * current priority value.
 *
 * @param[in] the_thread The thread changing its priority.
 * @param[in] new_priority The new thread priority.
 * @param[in] prepend_it In case this is true, then enqueue the thread as the
 * first of its priority group, otherwise enqueue the thread as the last of its
 * priority group.
 */
RTEMS_INLINE_ROUTINE void _Scheduler_Change_priority(
  Thread_Control          *the_thread,
  Priority_Control         new_priority,
  bool                     prepend_it
)
{
  const Scheduler_Control *scheduler = _Scheduler_Get( the_thread );

  ( *scheduler->Operations.change_priority )(
    scheduler,
    the_thread,
    new_priority,
    prepend_it
  );
}

/**
 * @brief Initializes a scheduler node.
 *
 * The scheduler node contains arbitrary data on function entry.  The caller
 * must ensure that _Scheduler_Node_destroy() will be called after a
 * _Scheduler_Node_initialize() before the memory of the scheduler node is
 * destroyed.
 *
 * @param[in] scheduler The scheduler instance.
 * @param[in] the_thread The thread containing the scheduler node.
 */
RTEMS_INLINE_ROUTINE void _Scheduler_Node_initialize(
  const Scheduler_Control *scheduler,
  Thread_Control          *the_thread
)
{
  return ( *scheduler->Operations.node_initialize )( scheduler, the_thread );
}

/**
 * @brief Destroys a scheduler node.
 *
 * The caller must ensure that _Scheduler_Node_destroy() will be called only
 * after a corresponding _Scheduler_Node_initialize().
 *
 * @param[in] scheduler The scheduler instance.
 * @param[in] the_thread The thread containing the scheduler node.
 */
RTEMS_INLINE_ROUTINE void _Scheduler_Node_destroy(
  const Scheduler_Control *scheduler,
  Thread_Control          *the_thread
)
{
  ( *scheduler->Operations.node_destroy )( scheduler, the_thread );
}

/**
 * @brief Updates the scheduler about a priority change of a not ready thread.
 *
 * @param[in] the_thread The thread.
 * @param[in] new_priority The new priority of the thread.
 */
RTEMS_INLINE_ROUTINE void _Scheduler_Update_priority(
  Thread_Control   *the_thread,
  Priority_Control  new_priority
)
{
  const Scheduler_Control *scheduler = _Scheduler_Get( the_thread );

  ( *scheduler->Operations.update_priority )(
    scheduler,
    the_thread,
    new_priority
  );
}

/**
 * @brief Compares two priority values.
 *
 * @param[in] scheduler The scheduler instance.
 * @param[in] p1 The first priority value.
 * @param[in] p2 The second priority value.
 *
 * @retval negative The value @a p1 encodes a lower priority than @a p2 in the
 * intuitive sense of priority.
 * @retval 0 The priorities @a p1 and @a p2 are equal.
 * @retval positive The value @a p1 encodes a higher priority than @a p2 in the
 * intuitive sense of priority.
 *
 * @see _Scheduler_Is_priority_lower_than() and
 * _Scheduler_Is_priority_higher_than().
 */
RTEMS_INLINE_ROUTINE int _Scheduler_Priority_compare(
  const Scheduler_Control *scheduler,
  Priority_Control         p1,
  Priority_Control         p2
)
{
  return ( *scheduler->Operations.priority_compare )( p1, p2 );
}

/**
 * @brief Releases a job of a thread with respect to the scheduler.
 *
 * @param[in] the_thread The thread.
 * @param[in] length The period length.
 */
RTEMS_INLINE_ROUTINE void _Scheduler_Release_job(
  Thread_Control *the_thread,
  uint32_t        length
)
{
  const Scheduler_Control *scheduler = _Scheduler_Get( the_thread );

  ( *scheduler->Operations.release_job )( scheduler, the_thread, length );
}

/**
 * @brief Scheduler method invoked at each clock tick.
 *
 * This method is invoked at each clock tick to allow the scheduler
 * implementation to perform any activities required.  For the
 * scheduler which support standard RTEMS features, this includes
 * time-slicing management.
 */
RTEMS_INLINE_ROUTINE void _Scheduler_Tick( void )
{
  uint32_t cpu_count = _SMP_Get_processor_count();
  uint32_t cpu_index;

  for ( cpu_index = 0 ; cpu_index < cpu_count ; ++cpu_index ) {
    const Per_CPU_Control *cpu = _Per_CPU_Get_by_index( cpu_index );
    const Scheduler_Control *scheduler = _Scheduler_Get_by_CPU( cpu );
    Thread_Control *executing = cpu->executing;

    if ( scheduler != NULL && executing != NULL ) {
      ( *scheduler->Operations.tick )( scheduler, executing );
    }
  }
}

/**
 * @brief Starts the idle thread for a particular processor.
 *
 * @param[in,out] the_thread The idle thread for the processor.
 * @parma[in,out] processor The processor for the idle thread.
 *
 * @see _Thread_Create_idle().
 */
RTEMS_INLINE_ROUTINE void _Scheduler_Start_idle(
  const Scheduler_Control *scheduler,
  Thread_Control          *the_thread,
  Per_CPU_Control         *cpu
)
{
  ( *scheduler->Operations.start_idle )( scheduler, the_thread, cpu );
}

#if defined(RTEMS_SMP)
RTEMS_INLINE_ROUTINE const Scheduler_Assignment *_Scheduler_Get_assignment(
  uint32_t cpu_index
)
{
  return &_Scheduler_Assignments[ cpu_index ];
}

RTEMS_INLINE_ROUTINE bool _Scheduler_Is_mandatory_processor(
  const Scheduler_Assignment *assignment
)
{
  return (assignment->attributes & SCHEDULER_ASSIGN_PROCESSOR_MANDATORY) != 0;
}

RTEMS_INLINE_ROUTINE bool _Scheduler_Should_start_processor(
  const Scheduler_Assignment *assignment
)
{
  return assignment->scheduler != NULL;
}
#endif /* defined(RTEMS_SMP) */

RTEMS_INLINE_ROUTINE bool _Scheduler_Has_processor_ownership(
  const Scheduler_Control *scheduler,
  uint32_t cpu_index
)
{
#if defined(RTEMS_SMP)
  const Scheduler_Assignment *assignment =
    _Scheduler_Get_assignment( cpu_index );

  return assignment->scheduler == scheduler;
#else
  (void) scheduler;
  (void) cpu_index;

  return true;
#endif
}

RTEMS_INLINE_ROUTINE void _Scheduler_Set(
  const Scheduler_Control *scheduler,
  Thread_Control          *the_thread
)
{
#if defined(RTEMS_SMP)
  const Scheduler_Control *current_scheduler = _Scheduler_Get( the_thread );

  if ( current_scheduler != scheduler ) {
    _Thread_Set_state( the_thread, STATES_MIGRATING );
    _Scheduler_Node_destroy( current_scheduler, the_thread );
    the_thread->Scheduler.control = scheduler;
    _Scheduler_Node_initialize( scheduler, the_thread );
    _Scheduler_Update_priority( the_thread, the_thread->current_priority );
    _Thread_Clear_state( the_thread, STATES_MIGRATING );
  }
#else
  (void) scheduler;
#endif
}

#if defined(__RTEMS_HAVE_SYS_CPUSET_H__)

RTEMS_INLINE_ROUTINE void _Scheduler_Get_processor_set(
  const Scheduler_Control *scheduler,
  size_t                   cpusetsize,
  cpu_set_t               *cpuset
)
{
  uint32_t cpu_count = _SMP_Get_processor_count();
  uint32_t cpu_index;

  CPU_ZERO_S( cpusetsize, cpuset );

  for ( cpu_index = 0 ; cpu_index < cpu_count ; ++cpu_index ) {
#if defined(RTEMS_SMP)
    if ( _Scheduler_Has_processor_ownership( scheduler, cpu_index ) ) {
      CPU_SET_S( (int) cpu_index, cpusetsize, cpuset );
    }
#else
    (void) scheduler;

    CPU_SET_S( (int) cpu_index, cpusetsize, cpuset );
#endif
  }
}

RTEMS_INLINE_ROUTINE bool _Scheduler_default_Get_affinity_body(
  const Scheduler_Control *scheduler,
  Thread_Control          *the_thread,
  size_t                   cpusetsize,
  cpu_set_t               *cpuset
)
{
  (void) the_thread;

  _Scheduler_Get_processor_set( scheduler, cpusetsize, cpuset );

  return true;
}

bool _Scheduler_Get_affinity(
  Thread_Control *the_thread,
  size_t          cpusetsize,
  cpu_set_t      *cpuset
);

RTEMS_INLINE_ROUTINE bool _Scheduler_default_Set_affinity_body(
  const Scheduler_Control *scheduler,
  Thread_Control          *the_thread,
  size_t                   cpusetsize,
  const cpu_set_t         *cpuset
)
{
  uint32_t cpu_count = _SMP_Get_processor_count();
  uint32_t cpu_index;
  bool     ok = true;

  for ( cpu_index = 0 ; cpu_index < cpu_count ; ++cpu_index ) {
#if defined(RTEMS_SMP)
    const Scheduler_Control *scheduler_of_cpu =
      _Scheduler_Get_by_CPU_index( cpu_index );

    ok = ok
      && ( CPU_ISSET_S( (int) cpu_index, cpusetsize, cpuset )
        || ( !CPU_ISSET_S( (int) cpu_index, cpusetsize, cpuset )
          && scheduler != scheduler_of_cpu ) );
#else
    (void) scheduler;

    ok = ok && CPU_ISSET_S( (int) cpu_index, cpusetsize, cpuset );
#endif
  }

  return ok;
}

bool _Scheduler_Set_affinity(
  Thread_Control          *the_thread,
  size_t                   cpusetsize,
  const cpu_set_t         *cpuset
);

#endif /* defined(__RTEMS_HAVE_SYS_CPUSET_H__) */

RTEMS_INLINE_ROUTINE void _Scheduler_Update_heir(
  Thread_Control *heir,
  bool force_dispatch
)
{
  Thread_Control *executing = _Thread_Executing;

  _Thread_Heir = heir;

  if ( executing != heir && ( force_dispatch || executing->is_preemptible ) )
    _Thread_Dispatch_necessary = true;
}

RTEMS_INLINE_ROUTINE void _Scheduler_Generic_block(
  const Scheduler_Control *scheduler,
  Thread_Control          *the_thread,
  void                  ( *extract )(
                             const Scheduler_Control *,
                             Thread_Control * ),
  void                  ( *schedule )(
                             const Scheduler_Control *,
                             Thread_Control *,
                             bool )
)
{
  ( *extract )( scheduler, the_thread );

  /* TODO: flash critical section? */

  if ( _Thread_Is_executing( the_thread ) || _Thread_Is_heir( the_thread ) ) {
    ( *schedule )( scheduler, the_thread, true );
  }
}

/**
 * @brief Returns true if @a p1 encodes a lower priority than @a p2 in the
 * intuitive sense of priority.
 */
RTEMS_INLINE_ROUTINE bool _Scheduler_Is_priority_lower_than(
  const Scheduler_Control *scheduler,
  Priority_Control         p1,
  Priority_Control         p2
)
{
  return _Scheduler_Priority_compare( scheduler, p1,  p2 ) < 0;
}

/**
 * @brief Returns true if @a p1 encodes a higher priority than @a p2 in the
 * intuitive sense of priority.
 */
RTEMS_INLINE_ROUTINE bool _Scheduler_Is_priority_higher_than(
  const Scheduler_Control *scheduler,
  Priority_Control         p1,
  Priority_Control         p2
)
{
  return _Scheduler_Priority_compare( scheduler, p1,  p2 ) > 0;
}

/**
 * @brief Returns the priority encoding @a p1 or @a p2 with the higher priority
 * in the intuitive sense of priority.
 */
RTEMS_INLINE_ROUTINE Priority_Control _Scheduler_Highest_priority_of_two(
  const Scheduler_Control *scheduler,
  Priority_Control         p1,
  Priority_Control         p2
)
{
  return _Scheduler_Is_priority_higher_than( scheduler, p1, p2 ) ? p1 : p2;
}

/**
 * @brief Sets the thread priority to @a priority if it is higher than the
 * current priority of the thread in the intuitive sense of priority.
 */
RTEMS_INLINE_ROUTINE void _Scheduler_Set_priority_if_higher(
  const Scheduler_Control *scheduler,
  Thread_Control          *the_thread,
  Priority_Control         priority
)
{
  Priority_Control current = the_thread->current_priority;

  if ( _Scheduler_Is_priority_higher_than( scheduler, priority, current ) ) {
    _Thread_Set_priority( the_thread, priority );
  }
}

/**
 * @brief Changes the thread priority to @a priority if it is higher than the
 * current priority of the thread in the intuitive sense of priority.
 */
RTEMS_INLINE_ROUTINE void _Scheduler_Change_priority_if_higher(
  const Scheduler_Control *scheduler,
  Thread_Control          *the_thread,
  Priority_Control         priority,
  bool                     prepend_it
)
{
  Priority_Control current = the_thread->current_priority;

  if ( _Scheduler_Is_priority_higher_than( scheduler, priority, current ) ) {
    _Thread_Change_priority( the_thread, priority, prepend_it );
  }
}

RTEMS_INLINE_ROUTINE Scheduler_Context *_Scheduler_Get_context(
  const Scheduler_Control *scheduler
)
{
  return scheduler->context;
}

RTEMS_INLINE_ROUTINE uint32_t _Scheduler_Get_processor_count(
  const Scheduler_Control *scheduler
)
{
#if defined(RTEMS_SMP)
  return _Scheduler_Get_context( scheduler )->processor_count;
#else
  (void) scheduler;

  return 1;
#endif
}

RTEMS_INLINE_ROUTINE Objects_Id _Scheduler_Build_id( uint32_t scheduler_index )
{
  return _Objects_Build_id(
    OBJECTS_FAKE_OBJECTS_API,
    OBJECTS_FAKE_OBJECTS_SCHEDULERS,
    _Objects_Local_node,
    scheduler_index + 1
  );
}

RTEMS_INLINE_ROUTINE uint32_t _Scheduler_Get_index_by_id( Objects_Id id )
{
  uint32_t minimum_id = _Scheduler_Build_id( 0 );

  return id - minimum_id;
}

RTEMS_INLINE_ROUTINE bool _Scheduler_Get_by_id(
  Objects_Id                id,
  const Scheduler_Control **scheduler_p
)
{
  uint32_t index = _Scheduler_Get_index_by_id( id );
  const Scheduler_Control *scheduler = &_Scheduler_Table[ index ];

  *scheduler_p = scheduler;

  return index < _Scheduler_Count
    && _Scheduler_Get_processor_count( scheduler ) > 0;
}

RTEMS_INLINE_ROUTINE bool _Scheduler_Is_id_valid( Objects_Id id )
{
  const Scheduler_Control *scheduler;
  bool ok = _Scheduler_Get_by_id( id, &scheduler );

  (void) scheduler;

  return ok;
}

RTEMS_INLINE_ROUTINE uint32_t _Scheduler_Get_index(
  const Scheduler_Control *scheduler
)
{
  return (uint32_t) (scheduler - &_Scheduler_Table[ 0 ]);
}

RTEMS_INLINE_ROUTINE Scheduler_Node *_Scheduler_Node_get(
  Thread_Control *the_thread
)
{
  return the_thread->Scheduler.node;
}

RTEMS_INLINE_ROUTINE void _Scheduler_Node_do_initialize(
  Scheduler_Node *node,
  Thread_Control *the_thread
)
{
#if defined(RTEMS_SMP)
  node->owner = the_thread;
#else
  (void) node;
  (void) the_thread;
#endif
}

#if defined(RTEMS_SMP)
RTEMS_INLINE_ROUTINE Thread_Control *_Scheduler_Node_get_owner(
  const Scheduler_Node *node
)
{
  return node->owner;
}
#endif

/** @} */

#ifdef __cplusplus
}
#endif

#endif
/* end of include file */
