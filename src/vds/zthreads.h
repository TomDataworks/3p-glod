#ifndef ZTHREADS
#define ZTHREADS
/******************************************************************************
 * ZThreads Library Interface (Version 0.2)
 *
 * Written by:    John Thornley
 * Last modified: 15 May 2000
 * Language:      C++
 *
 * Description:
 *
 * The ZThreads namespace containing the ZThreads classes and functions.
 *
 * Copyright (c) John Thornley, 1996-2000.
 ******************************************************************************/

/******************************************************************************
 * Errors and Exceptions:
 *
 * Preconditions: Operations of the ZThreads library are subject to specified
 * preconditions.  If the preconditions do not hold, a ztBadParametersError
 * exception is thrown.
 *
 * Exceptions: Operations of the ZThreads library throw exceptions in the case
 * of resource exhaustion or failure, or erroneous use of the operations.
 * Exceptions are derived from the ztException base class.
 *
 * Atomicity: Certain operations of the ZThreads library are required not to
 * be called concurrently with certain other operations of the ZThreads library.
 * The result of violating these atomicity requirements is not defined.
 *
 * Fatal Errors: A few operations of the ZThreads library are subject to fatal
 * error conditions.  These error conditions result in termination of the
 * process.
 ******************************************************************************/

namespace ZThreads {

/******************************************************************************
 * Implementation-dependent constants
 ******************************************************************************/
//------------------------------------------------------------------------------
// ZT_PRIORITY_PARENT
//------------------------------------------------------------------------------ 

enum { ZT_PRIORITY_PARENT = (int) -1 };

// Description:
// - Set the scheduling priority of a thread to the scheduling priority of
//   it's parent thread.
// Guarantees:
// - ZT_PRIORITY_PARENT < 0.

//------------------------------------------------------------------------------
// ZT_PROCESSORS_ALL
//------------------------------------------------------------------------------

const bool *const ZT_PROCESSORS_ALL = (const bool *const) 1;

// Description:
// - Set the processors on which a thread may execute to the set of
//   all available processors.
// Guarantees:
// - ZT_PROCESSORS_ALL != 0.
// - ZT_PROCESSORS_ALL != ZT_PROCESSORS_PARENT.
// - ZT_PROCESSORS_ALL != any valid pointer to memory.

//------------------------------------------------------------------------------
// ZT_PROCESSORS_PARENT
//------------------------------------------------------------------------------

const bool *const ZT_PROCESSORS_PARENT = (const bool *const) 2;

// Description:
// - Set the processors on which a thread may execute to the set of
//   processors on which its parent thread may execute.
// Guarantees:
// - ZT_PROCESSORS_PARENT != 0.
// - ZT_PROCESSORS_PARENT != ZT_PROCESSORS_ALL.
// - ZT_PROCESSORS_PARENT != any valid pointer to memory.

/******************************************************************************
 * Implementation-independent constants (fixed in all implementations)
 ******************************************************************************/
//------------------------------------------------------------------------------
// ZT_STACKSIZE_DEFAULT
//------------------------------------------------------------------------------ 

enum { ZT_STACKSIZE_DEFAULT = (unsigned int) 0 };

// Description:
// - Set the scheduling priority of a thread to the scheduling priority of
//   it's parent thread.
// Guarantees:
// - ZT_STACKSIZE_DEFAULT == 0.

/******************************************************************************
 * Exceptions
 ******************************************************************************/
//------------------------------------------------------------------------------
// The following exceptions are thrown by operations of the ZThreads library.
// Exceptions are divided into "failure" and "error" exceptions.
//------------------------------------------------------------------------------

class ztException {};

//------------------------------------------------------------------------------
// Failure exceptions are thrown in response to exhaustion or failure of
// system resources.
//------------------------------------------------------------------------------

class ztFailureException : public ztException {};

class ztMemoryAllocationFailure : public ztFailureException {};
// Attempt to allocate memory failed, e.g., not enough memory available.

class ztThreadCreationFailure : public ztFailureException {};
// Attempt to create a thread failed, e.g., too many active threads.

class ztSyncObjectCreationFailure : public ztFailureException {};
// Attempt to create a synchronization object failed, e.g., too many
// active synchronization objects.

//------------------------------------------------------------------------------
// Error exceptions are thrown in response to erroneous use of ZThreads 
// operations.  Operations that throw an error exception are guaranteed not to
// have acquired any system resources (memory, synchronization objects, etc.),
// created any threads, or altered the state of any object.
///------------------------------------------------------------------------------

class ztErrorException : public ztException {};

class ztBadParametersError : public ztErrorException {};
// Parameters of a method/function do not satisfy a precondition.

class ztDestructBusySyncObjectError : public ztErrorException {};
// Destructor on synchronization object with suspended threads.

class ztSetPreviouslySetFlagError : public ztErrorException {};
// Set operation on previously set flag.

class ztLockHeldLockError : public ztErrorException {};
// Lock operation on lock that is already held by the calling thread.

class ztUnlockUnheldLockError : public ztErrorException {};
// Unlock operation on lock that is not held by the calling thread.

class ztIncrementCounterOverflowError : public ztErrorException {};
// Increment operation on counter that increases the counter's value to greater
// then the maximum unsigned int value.

class ztPassInvalidThreadBarrierError : public ztErrorException {};
// Pass operation on barrier called by more than barrier.numThreads different
// threads.

class ztResetBusySyncObjectError : public ztErrorException {};
// Reset operation on synchronization object with suspended threads.

/******************************************************************************
 * ZThreads version identification
 ******************************************************************************/

void ztGetVersion(
		unsigned int* majorVersionNum,
		unsigned int* minorVersionNum,
		unsigned int* releaseNum );

// Description:
// - Returns numbers identifying the current version of the ZThreads library.
//   * majorVersionNum identifies major changes in features and functionality.
//   * minorVersionNum identifies minor changes in features and functionality.
//   * releaseNum identifies releases of the same version, e.g., after
//     bug fixes and performance improvements.  
// Requires:
// - majorVersionNum != 0.
// - minorVersionNum != 0.
// - releaseNum != 0.

/******************************************************************************
 * Thread stack size functions
 ******************************************************************************/
//------------------------------------------------------------------------------
// The size of the stack that is allocated to a thread on creation is specified
// as a parameter to the thread creation call. 
//------------------------------------------------------------------------------

unsigned int ztStackSizeDefault( void );

// Description:
// - Returns the default stack size (in bytes) that is allocated to
//   a thread on creation.
// Guarantees:
// - ztStackSizeDefault() >= ztStackSizeMinimum().
// - ztStackSizeDefault() <= ztStackSizeMaximum().

//------------------------------------------------------------------------------
 
unsigned int ztStackSizeMinimum( void );

// Description:
// - Returns the minimum stack size (in bytes) that can be allocated to
//   a thread on creation.
// Guarantees:
// - ztStackSizeMinimum <= ztStackSizeDefault().
// - ztStackSizeMinimum <= ztStackSizeMaximum().

//------------------------------------------------------------------------------
 
unsigned int ztStackSizeMaximum( void );

// Description:
// - Returns the maximum stack size (in bytes) that can be allocated to
//   a thread on creation.
// Guarantees:
// - ztStackSizeMaximum >= ztStackSizeDefault().
// - ztStackSizeMaximum >= ztStackSizeMaximum().

/******************************************************************************
 * Scheduling priority functions
 ******************************************************************************/
//------------------------------------------------------------------------------
// A scheduling priority is a value from an implementation-dependent range of
// consecutive nonnegative int values.  Lower values represent lower thread
// scheduling priority and higher values represent higher thread scheduling
// priority.  The scheduling priority of a thread is set when the thread is
// created and can be changed by the thread during execution.  The scheduling
// priority of a thread does not change unless it is explicitly changed.
//------------------------------------------------------------------------------

int ztPriorityLowest( void );

// Description:
// - Returns the lowest possible scheduling priority for the threads of the 
//   current process.
// Guarantees:
// - ztPriorityLowest() >= 0.
// - ztPriorityLowest() <= ztPriorityHighest().

//------------------------------------------------------------------------------

int ztPriorityHighest( void );

// Description:
// - Returns the highest possible scheduling priority for the threads of the 
//   current process.
// Guarantees:
// - ztPriorityHighest() >= 0.
// - ztPriorityHighest() >= ztPriorityLowest().

//------------------------------------------------------------------------------

int ztPriorityMiddle( void );

// Description:
// - Returns
//       (ztPriorityLowest() + (ztPriorityHighest() - ztPriorityLowest())/2).
// Guarantees:
// - ztPriorityMiddle() ==
//       (ztPriorityLowest() + (ztPriorityHighest() - ztPriorityLowest())/2).
 
//------------------------------------------------------------------------------
 
void ztSetThreadPriority( int priority );

// Description:
// - Sets the scheduling priority of the current thread.
// Input Parameters:
// - priority: The scheduling priority.
// Preconditions:
// - priority >= ztPriorityLowest().
// - priority <= ztPriorityHighest().

//------------------------------------------------------------------------------
 
void ztGetThreadPriority( int* priority );

// Description:
// - Returns the scheduling priority of the current thread.
// Output Parameters:
// - *priority: The scheduling priority.
// Preconditions:
// - priority != 0.
// Guarantees:
// - *priority >= ztPriorityLowest().
// - *priority <= ztPriorityHighest().

/******************************************************************************
 * Processor assignment functions
 ******************************************************************************/
//------------------------------------------------------------------------------
// Each processor that is present on the system is represented by a value in
// the range 0 .. ztNumSystemProcessors() - 1.  A set of processors is
// represented as a bool array with one element for every value in this range.
//------------------------------------------------------------------------------

int ztNumSystemProcessors( void );

// Description:
// - Returns the number of processors on the system.
// Guarantees:
// - ztNumSystemProcessors() >= 1.

//------------------------------------------------------------------------------

void ztGetAvailableProcessors(
		bool processorMap[/*ztNumSystemProcessors()*/] );

// Description:
// - Returns the processors available to the threads of the current process.
// Output Parameters:
// - processorMap: The processor map.
// Preconditions:
// - processorMap != 0.
// Guarantees:
// - processorMap is not empty.

//------------------------------------------------------------------------------

void ztSetThreadProcessors(
		const bool processorMap[/*ztNumSystemProcessors()*/] );

// Description:
// - Sets the processors on which the current thread may execute.
// Input Parameters:
// - processorMap: The processor map.
// Preconditions:
// - (processorMap == ZT_PROCESSORS_ALL) ||
//   ((processorMap != 0 ) &&
//    (processorMap is non-empty) &&
//    (processorMap is a subset of the processors available to
//     the threads of the current process)).

//------------------------------------------------------------------------------

void ztGetThreadProcessors(
		bool processorMap[/*ztNumSystemProcessors()*/] );

// Description:
// - Returns the processors on which the current thread may execute.
// Output Parameters:
// - processorMap: The processor map.
// Preconditions:
// - processorMap != 0.
// Guarantees:
// - processorMap is not empty.
// - processorMap is a subset of the processors available to
//   the threads of the current process.

//------------------------------------------------------------------------------

void ztGetNumAvailableProcessors( int* numProcessors );

// Description:
// - Returns the number of processors available to
//   the threads of the current process.
// Output Parameters:
// - *numProcessors: The number of processors.
// Preconditions:
// - numProcessors != 0.
// Guarantees:
// - *numProcessors >= 1.
// - *numProcessors <= ztNumSystemProcessors().

//------------------------------------------------------------------------------

void ztSetNumThreadProcessors( int numProcessors );

// Description:
// - Sets the processors on which the current thread may execute to
//   an implementation-dependent set of processors.
// Input Parameters:
// - numProcessors: The number of processors.
// Preconditions:
// - numProcessors >= 1.
// - numprocessors <= the number of processors available to
//                    the threads of the current process.

/******************************************************************************
 * CreateThread function
 ******************************************************************************/
//------------------------------------------------------------------------------
// The ztCreateThread function creates a single asynchronous thread.  The
// function takes two required parameters: the function executed by the thread
// and the parameter passed to the thread.  The function also takes three
// optional parameters: the size of the stack that is allocated to the thread,
// the initial scheduling priority of the thread, and the processors on which
// the thread may initially execute.
//------------------------------------------------------------------------------

typedef void* ztThreadParams;
typedef void (*ztThreadFunc)( ztThreadParams params );

void ztCreateThread(
		ztThreadFunc threadFunc,
		ztThreadParams threadParams );

void ztCreateThread(
		ztThreadFunc threadFunc,
		ztThreadParams threadParams,
		unsigned int stackSize );

void ztCreateThread(
		ztThreadFunc threadFunc,
		ztThreadParams threadParams,
		unsigned int stackSize,
		int priority );

void ztCreateThread(
		ztThreadFunc threadFunc,
		ztThreadParams threadParams,
		unsigned int stackSize,
		int priority,
		const bool processorMap[/*ztNumSystemProcessors()*/] );

// Description:
// - Executes threadFunc(threadParams) as an asynchronous thread.  Returns
//   immediately after creating the thread.
// Input Parameters:
// - threadFunc   : The function executed by the thread.
// - threadParams : The parameter passed to the threadFunc call.
// - stackSize    : The size of the stack that is allocated to the thread.
//                  Default = ZT_STACKSIZE_DEFAULT.
// - priority     : The initial scheduling priority of the thread.
//                  Default = ZT_PRIORITY_PARENT.
// - processorMap : The processors on which the thread may initially execute.
//                  Default = ZT_PROCESSORS_PARENT.
// Preconditions:
// - threadFunc != 0.
// - (stackSize == ZT_STACKSIZE_DEFAULT) ||
//   ((stackSize >= ztStackSizeMinimum()) && 
//    (stackSize <= ztStackSizeMaximum())).
// - (priority == ZT_PRIORITY_PARENT) ||
//   ((priority >= ztPriorityLowest()) && (priority <= ztPriorityHighest())).
// - (processorMap == ZT_PROCESSORS_PARENT) ||
//   (processorMap == ZT_PROCESSORS_ALL) ||
//   ((processorMap != 0) &&
//    (processorMap is not empty) &&
//    (processorMap is a subset of the processors available to
//     the threads of the current process)).
// Notable Failure Exceptions:
// - If the thread cannot be created, throws ztThreadCreationFailure.
// Fatal Errors:
// - threadFunc is not a valid function pointer.
// - The thread terminates exceptionally.

/******************************************************************************
 * ForkAndJoinThreads function
 ******************************************************************************/
//------------------------------------------------------------------------------
// The ztForkAndJoinThreads function creates a set of threads then waits for all
// the threads to terminate.  All the threads execute the same function and are
// passed the same parameter, but are additionally passed a distinguishing index
// parameter from the range 0 .. numThreads.  The function takes three required
// parameters: the number of threads, the function executed by the threads, and
// the parameter passed to all the threads.  The function also takes three
// optional parameters: the sizes of the stacks that are allocated to the
// threads, the initial scheduling priorities of the threads, and the processors
// on which the threads may initially execute.  The three optional parameters
// each take two forms: either a single value that applies to all the threads,
// or an array of values with a different value for each thread.
//------------------------------------------------------------------------------

typedef void (*ztForkedThreadFunc)( 
		unsigned int index, unsigned int numThreads, ztThreadParams params );

void ztForkAndJoinThreads(
		unsigned int numThreads,
		ztForkedThreadFunc threadFunc,
		ztThreadParams threadParams );

void ztForkAndJoinThreads(
		unsigned int numThreads,
		ztForkedThreadFunc threadFunc,
		ztThreadParams threadParams,
		unsigned int stackSize );

void ztForkAndJoinThreads(
		unsigned int numThreads,
		ztForkedThreadFunc threadFunc,
		ztThreadParams threadParams,
		const unsigned int stackSize[/*numThreads*/] );

void ztForkAndJoinThreads(
		unsigned int numThreads,
		ztForkedThreadFunc threadFunc,
		ztThreadParams threadParams,
		unsigned int stackSize,
		int priority );

void ztForkAndJoinThreads(
		unsigned int numThreads,
		ztForkedThreadFunc threadFunc,
		ztThreadParams threadParams,
		unsigned int stackSize,
		const int priority[/*numThreads*/] );

void ztForkAndJoinThreads(
		unsigned int numThreads,
		ztForkedThreadFunc threadFunc,
		ztThreadParams threadParams,
		const unsigned int stackSize[/*numThreads*/],
		int priority );

void ztForkAndJoinThreads(
		unsigned int numThreads,
		ztForkedThreadFunc threadFunc,
		ztThreadParams threadParams,
		const unsigned int stackSize[/*numThreads*/],
		const int priority[/*numThreads*/] );

void ztForkAndJoinThreads(
		unsigned int numThreads,
		ztForkedThreadFunc threadFunc,
		ztThreadParams threadParams,
		unsigned int stackSize,
		int priority,
		const bool processorMap[/*ztNumSystemProcessors()*/] );

void ztForkAndJoinThreads(
		unsigned int numThreads,
		ztForkedThreadFunc threadFunc,
		ztThreadParams threadParams,
		unsigned int stackSize,
		int priority,
		const bool *const processorMap[/*numThreads*/] );

void ztForkAndJoinThreads(
		unsigned int numThreads,
		ztForkedThreadFunc threadFunc,
		ztThreadParams threadParams,
		unsigned int stackSize,
		const int priority[/*numThreads*/],
		const bool processorMap[/*ztNumSystemProcessors()*/] );

void ztForkAndJoinThreads(
		unsigned int numThreads,
		ztForkedThreadFunc threadFunc,
		ztThreadParams threadParams,
		unsigned int stackSize,
		const int priority[/*numThreads*/],
		const bool *const processorMap[/*numThreads*/] );

void ztForkAndJoinThreads(
		unsigned int numThreads,
		ztForkedThreadFunc threadFunc,
		ztThreadParams threadParams,
		const unsigned int stackSize[/*numThreads*/],
		int priority,
		const bool processorMap[/*ztNumSystemProcessors()*/] );

void ztForkAndJoinThreads(
		unsigned int numThreads,
		ztForkedThreadFunc threadFunc,
		ztThreadParams threadParams,
		const unsigned int stackSize[/*numThreads*/],
		int priority,
		const bool *const processorMap[/*numThreads*/] );

void ztForkAndJoinThreads(
		unsigned int numThreads,
		ztForkedThreadFunc threadFunc,
		ztThreadParams threadParams,
		const unsigned int stackSize[/*numThreads*/],
		const int priority[/*numThreads*/],
		const bool processorMap[/*ztNumSystemProcessors()*/] );

void ztForkAndJoinThreads(
		unsigned int numThreads,
		ztForkedThreadFunc threadFunc,
		ztThreadParams threadParams,
		const unsigned int stackSize[/*numThreads*/],
		const int priority[/*numThreads*/],
		const bool *const processorMap[/*numThreads*/] );

// Description:
// - Executes threadFunc(i, numThreads, threadParams) as asynchronous threads,
//   for i = 0 .. numThreads - 1.  Returns after all the created threads have
//   terminated.
// Input Parameters:
// - numThreads   : The number of threads to be created.
// - threadFunc   : The function executed by the threads.
// - threadParams : The parameter passed to the threadFunc calls.
// - stackSize    : The size of the stack allocated to the threads.
//                  Default = ZT_STACKSIZE_DEFAULT.
// - priority     : The initial scheduling priority of the threads.
//                  Default = ZT_PRIORITY_PARENT.
// - processorMap : The processors on which the threads may execute.
//                  Default = ZT_PROCESSORS_PARENT.
// Preconditions (general):
// - threadFunc != 0. 
// Preconditions (unsigned int stackSize):
// - (stackSize == ZT_STACKSIZE_DEFAULT) ||
//   ((stackSize >= ztStackSizeMinimum()) && 
//    (stackSize <= ztStackSizeMaximum())).
// Preconditions (unsigned int stackSize[/*numThreads*/]):
// - stackSize != 0.
// - for (unsigned int t = 0; t < numThreads; t++)
//       (stackSize[t] == ZT_STACKSIZE_DEFAULT) ||
//       ((stackSize[t] >= ztStackSizeMinimum()) && 
//        (stackSize[t] <= ztStackSizeMaximum())).
// Preconditions (int priority):
// - (priority == ZT_PRIORITY_PARENT) ||
//   ((priority >= ztPriorityLowest()) &&
//    (priority <= ztPriorityHighest())).
// Preconditions (int priority[/*numThreads*/]):
// - priority != 0.
// - for (unsigned int t = 0; t < numThreads; t++)
//       (priority[t] == ZT_PRIORITY_PARENT) ||
//       ((priority[t] >= ztPriorityLowest()) &&
//        (priority[t] <= ztPriorityHighest())).
// Preconditions (const bool processorMap[]):
// - (processorMap == ZT_PROCESSORS_PARENT) ||
//   (processorMap == ZT_PROCESSORS_ALL) ||
//   ((processorMap != 0) &&
//    (processorMap is not empty) &&
//    (processorMap is a subset of the processors available to
//     the threads of the current process)).
// Preconditions (const bool *const processorMap[/*numThreads*/]):
// - processorMap != 0.
// - for (unsigned int t = 0; t < numThreads; t++) 
//       (processorMap[t] == ZT_PROCESSORS_PARENT) ||
//       (processorMap[t] == ZT_PROCESSORS_ALL) ||
//       ((processorMap[t] != 0) &&
//        (processorMap[t] is not empty) &&
//        (processorMap[t] is a subset of the processors available to
//         the threads of the current process)).
// Notable Failure Exceptions:
// - If all of the threads cannot be created, throws ztThreadCreationFailure.
// Special Error Handling:
// - threadFunc is not a valid function pointer.
// - Any of the threads terminate exceptionally.

/******************************************************************************
 * Flag class
 ******************************************************************************/
//------------------------------------------------------------------------------
// A ztFlag synchronization object has a value of either "set" or "not set",
// and three operations: Check, Set, and Reset.
//------------------------------------------------------------------------------

struct ztFlagImpl; // hidden implementation

class ztFlag {
public:

	ztFlag( void );  // constructor
	~ztFlag( void ); // destructor

	void Check( void );
	void Set( void );
	void Reset( void );

private:

	ztFlag( const ztFlag& ); // copying not permitted	
	ztFlag& operator=( const ztFlag& ); // assignment not permitted

	ztFlagImpl* impl; // pointer to hidden implementation
};

// flag.ztFlag() (constructor)
// Description:
// - Constructs flag and sets flag.value to "not set".
// Notable Failure Exceptions:
// - If flag cannot be constructed, throws ztSyncCreationFailure.

// flag.~ztFlag() (destructor)
// Description:
// - Destructs flag.
// Error Exceptions:
// - If any thread is suspended in a flag.Check() operation, throws
//   ztDestructBusySyncObjectError.

// flag.Check()
// Description:
// - If flag.value == "set", continues execution with no effect.
// - If flag.value == "not set", suspends execution of the calling thread.

// flag.Set()
// Description:
// - Sets flag.value to "set".
// - Resumes execution of all threads suspended in flag.Check() operations.
// Error Exceptions:
// - If flag.value == "set", throws ztSetPreviouslySetFlagError.

// flag.Reset()
// Description:
// - Sets flag.value to "not set".
// Error Exceptions:
// - If any thread is suspended in a flag.Check() operation, throws
//   ztResetBusySyncObjectError.
// Atomicity:
// - Must not be called concurrently with any other operation on flag.

/******************************************************************************
 * Lock class
 ******************************************************************************/
//------------------------------------------------------------------------------
// A ztLock synchronization object has a value of either "not locked" or
// "locked by T" (for some thread T), and two operations: Lock and Unlock.
//------------------------------------------------------------------------------

struct ztLockImpl; // hidden implementation

class ztLock {
public:

	ztLock( void );  // constructor
	~ztLock( void ); // destructor 

	void Lock( void );
	void Unlock( void );

	void dump();

private:

	ztLock( const ztLock& ); // copying not permitted	
	ztLock& operator=( const ztLock& ); // assignment not permitted

	ztLockImpl* impl; // pointer to hidden implementation
};

// lock.ztLock() (constructor)
// Description:
// - Constructs lock and sets lock.value to "not locked".
// Notable Failure Exceptions:
// - If lock cannot be constructed, throws ztSyncCreationFailure.

// lock.~ztLock() (destructor)
// Description:
// - Destructs lock.
// Error Exceptions:
// - If any thread is suspended in a lock.Lock() operation, throws
//   ztDestructBusySyncObjectError.

// lock.Lock() by thread T
// Description:
// - If lock.value is "not locked", sets lock.value to "locked by T".
// - If lock.value is "locked by X" (for some X != T) suspends execution of
//   the calling thread.
// Error Exceptions:
// - If lock.value is already "locked by T", throws ztLockHeldLockError.

// lock.Unlock() by thread T
// Description:
// - If no threads are suspended in lock.Lock() operations, sets lock.value
//   to "not locked".
// - If one or more threads are suspended in lock.Lock() operations, resumes
//   execution of one thread X suspended in a lock.Lock() operation and sets
//   lock.value to "locked by X".
// Error Exceptions:
// - If lock.value is not "locked by T", throws ztUnlockUnheldLockError.

/******************************************************************************
 * Counter class
 ******************************************************************************/
//------------------------------------------------------------------------------
// A ztCounter synchronization object has an unsigned int value, and four
// operations: Check, Increment, IncrementToMaximum, and Reset.
//------------------------------------------------------------------------------

struct ztCounterImpl; // hidden implementation

class ztCounter {
public:

	ztCounter( void );  // constructor
	~ztCounter( void ); // destructor 

	void Check( unsigned int level );
	void Increment( unsigned int amount = 1 );
	void IncrementToMaximum( void );
	void Reset( void );

private:

	ztCounter( const ztCounter& ); // copying not permitted	
	ztCounter& operator=( const ztCounter& ); // assignment not permitted

	ztCounterImpl* impl; // pointer to hidden implementation
};

// counter.ztCounter() (constructor)
// Description:
// - Constructs counter and sets counter.value to zero.
// Notable Failure Exceptions:
// - If counter cannot be constructed, throws ztSyncCreationFailure.

// counter.~ztCounter() (destructor)
// Description:
// - Destructs counter.
// Error Exceptions:
// - If any thread is suspended in a counter.Check() operation, throws
//   ztDestructBusySyncObjectError.

// counter.Check(level)
// Description:
// - If counter.value >= level, continues execution with no effect.
// - If counter.value < level, suspends execution of the calling thread.

// counter.Increment(amount)
// Description:
// - Increments counter.value by amount.
// - Resumes execution of all threads suspended in counter.Check(level)
//   operations with level <= counter.value.
// Error Exceptions:
// - If counter.value + amount is greater than the maximum unsigned int value,
//   throws ztIncrementCounterOverflowError.

// counter.IncrementToMaximum()
// Description:
// - Sets counter.value to the maximum unsigned int value.
// - Resumes execution of all threads suspended in counter.Check() operations.

// counter.Reset()
// Description:
// - Sets counter.value to zero.
// Error Exceptions:
// - If any thread is suspended in a counter.Check() operation, throws
//   ztResetBusySyncObjectError.
// Atomicity:
// - Must not be called concurrently with any other operation on counter.

/******************************************************************************
 * Barrier class
 ******************************************************************************/
//------------------------------------------------------------------------------
// A ztBarrier synchronization object has two unsigned int values: numThreads
// and numWaiting, and two operations: Pass and Reset.
//------------------------------------------------------------------------------

struct ztBarrierImpl; // hidden implementation

class ztBarrier {
public:

	ztBarrier( unsigned int numThreads ); // constructor
	~ztBarrier( void ); // destructor

	void Pass( void );
	void Reset( unsigned int numThreads );

private:

	ztBarrier( const ztBarrier& ); // copying not permitted	
	ztBarrier& operator=( const ztBarrier& ); // assignment not permitted

	ztBarrierImpl* impl; // pointer to hidden implementation
};

// barrier.ztBarrier(numThreads) (constructor)
// Description:
// - Constructs barrier, sets barrier.numThreads to numThreads, and sets
//   barrier.numWaiting to zero.
// Preconditions:
// - numThreads >= 1.
// Notable Failure Exceptions:
// - If barrier cannot be constructed, throws ztSyncCreationFailure.

// barrier.~ztBarrier() (destructor)
// Description:
// - Destructs barrier.
// Error Exceptions:
// - If any thread is suspended in a barrier.Pass() operation, throws
//   ztDestructBusySyncObjectError.

// barrier.Pass()
// Description:
// - If barrier.numWaiting + 1 == barrier.numThreads, sets barrier.numWaiting
//   to zero and resumes execution of all threads suspended in barrier.Pass()
//   operations.
// - If barrier.numWaiting + 1 < barrier.numThreads, increments
//   barrier.numWaiting by one and suspends execution of the calling thread.
// Error Exceptions:
// - If the calling thread is a different thread from the first
//   barrier.numThreads threads to perform barrier.Pass() operations since the
//   most recent barrier.ztBarrier() or barrier.Reset() operation, throws
//   ztPassInvalidThreadBarrierError.

// barrier.Reset(numThreads)
// Description:
// - Sets barrier.numThreads to numThreads and sets barrier.numWaiting to zero.
// Preconditions:
// - numThreads >= 1.
// Error Exceptions:
// - If any thread is suspended in a barrier.Pass() operation, throws
//   ztResetBusySyncObjectError.
// Atomicity:
// - Must not be called concurrently with any other operation on barrier.

/******************************************************************************/

} // End of ZThreads namespace.

/******************************************************************************/
#endif // !ZTHREADS
