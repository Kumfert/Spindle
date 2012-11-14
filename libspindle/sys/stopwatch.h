//
// solaris_stopwatch.h -- a simple timer.
//
// $Id: stopwatch.h,v 1.3 2000/03/07 22:22:09 kumfert Exp $
//
//  Gary Kumfert, Old Dominion University
//  Copyright(c) 1997, Old Dominion University.  All rights reserved.
// 
//  Permission to use, copy, modify , distribute and sell this software and
//  its documentation for any purpose is hereby granted without fee, 
//  provided that the above copyright notice appear in all copies and
//  that both that copyright notice and this permission notice appear
//  in supporting documentation.  Old Dominion University makes no
//  representations about the suitability of this software for any 
//  purpose.  It is provided "as is" without express or implied warranty.
//
///////////////////////////////////////////////////////////////////////
//

#ifndef __cplusplus
#error stopwatch.h is a C++ header file
#endif

#ifndef SPINDLE_STOPWATCH_H_
#define SPINDLE_STOPWATCH_H_

#ifdef HAVE_LIMITS_H
#  include <limits.h>
#endif

#ifndef DISABLE_STOPWATCH
#  if defined( HAVE_SYS_TIMES_H ) && ! defined( __CYGWIN32__ )
#    include <sys/times.h>
#    define USING_STRUCT_TMS
#  elif defined( TIME_WITH_SYS_TIME ) || defined ( __CYGWIN32__ )
#    include <sys/time.h>
#    include <time.h>
#    define USING_CLOCK_T
#  elif defined( HAVE_SYS_TIME_H )
#    include <sys/time.h>
#    define USING_CLOCK_T
#  elsif defined( HAVE_TIME_H )
#    include <time.h>
#    define USING_CLOCK_T
#  endif
#endif

SPINDLE_BEGIN_NAMESPACE


/**
 * @memo Basic Timer
 * @type class
 * 
 * Timers are intensely platform specific.
 * This little class has several implementations in
 * #$(SPINDLE_HOME)/src/sys/ *_stopwatch.h#
 * Only one of which will be included in the compiled version.
 * 
 * @author Gary Kumfert
 * @version #$Id: stopwatch.h,v 1.3 2000/03/07 22:22:09 kumfert Exp $#
 */
class stopwatch {
public:

  /// This typedef depends on the implementation, use #stopwatch::ticks#
  typedef long ticks;
      
  /** @name constructors/destructors */
  //@{
  /// default constructor
  stopwatch();

  /// default destructor
  ~stopwatch() { }
  //@}

  /** @name basic features */
  //@{
  /**
   * start the timer running
   * @return true if and only if it wasn't already running 
   */
  inline bool start();
  /** 
   * stop the timer from running
   * @return true if and only if it was running
   */
  inline bool stop();
 /**
   * mark the next lap.  Identical to a #start(); stop()# sequence.
   * @return true if and only if the timer was running.
   */
  inline bool lap();  
  /**
   * reset the timers and lap counter
   * @return true if and only if the timer was already stopped.
   */
  inline bool reset();

  //@}

  /** @name accessor methods 
   * All these methods assume the stopwatch is stopped
   */
  //@{

  /// get total user time since last #reset()#
  inline ticks queryTotalUserTicks() const; 

  /// get total system time since last #reset()#
  inline ticks queryTotalSystemTicks() const;

  /// get user time since last #start()# or #lap()#
  inline ticks queryLapUserTicks() const;    

  /// get system time since last #start()# or #lap()#
  inline ticks queryLapSystemTicks() const;  

  /// get number of #stop()# or #lap()# since last #reset()#
  inline int   queryNLaps() const; 

  /// get number of ticks per second
  static ticks queryTicksPerSec() { 
# if defined( USING_STRUCT_TMS )
    return CLK_TCK; 
# elif defined( USING_CLOCK_T )
    return CLOCKS_PER_SEC;
# else
    return 1L;
# endif
  }

  /** @name advanced modifiers 
   * The following operations do not affect lap counts.
   */
  //@{
  /// Add the total time from another stopwatch to this one.
  stopwatch& operator+=( const stopwatch& timer );
  /// Subtract total time from another stopwatch from this one.
  stopwatch& operator-=( const stopwatch& timer );
  //@}

private:
#if defined( USING_STRUCT_TMS )
  typedef struct tms timer_data;
#elif defined( USING_CLOCK_T )
  typedef clock_t timer_data;
#endif

  timer_data begin;
  timer_data end;
  ticks userTicks;
  ticks systemTicks;
  int nLaps;
  bool isRunning;

};

inline stopwatch::stopwatch(){
    isRunning=false;
#if defined( USING_STRUCT_TMS )
    end.tms_utime = (clock_t) -1; 
    end.tms_stime = (clock_t) -1;
    begin.tms_utime = 0;
    begin.tms_stime = 0;
#elif defined( USING_CLOCK_T )
    end = (clock_t) -1;
    begin = (clock_t) 0;
#endif
    userTicks = 0;
    systemTicks = 0;
    nLaps = 0;
  }

inline bool stopwatch::start(){
  if (isRunning) { return false; }
  ++nLaps;
#if defined( USING_STRUCT_TMS )
  if (end.tms_utime != -1) {
    userTicks += (end.tms_utime - begin.tms_utime);
    systemTicks += (end.tms_stime - begin.tms_stime);
  }
  times(&begin);
#elif defined( USING_CLOCK_T )
  if ( end != (clock_t) -1 ) {
    userTicks += ( end - begin );
  }
  begin = clock();
#endif
  return isRunning=true;
}

inline bool stopwatch::stop(){
  if (!isRunning) { return false; }
#if defined( USING_STRUCT_TMS )
  times(&end);
#elif defined( USING_CLOCK_T )
  end = clock();
#endif
  isRunning = false;
  return true;
}

inline bool stopwatch::lap(){
  if (!isRunning) { return false; }
  ++nLaps;
#if defined( USING_STRUCT_TMS )
  times(&end);
  userTicks += (end.tms_utime - begin.tms_utime);
  systemTicks += (end.tms_stime - begin.tms_stime);
  begin.tms_utime = end.tms_utime;
  begin.tms_stime = end.tms_stime;
#elif defined( USING_CLOCK_T )
  end = clock();
  userTicks += ( end - begin );
  begin = end;
#endif
  return true;
}

inline bool stopwatch::reset(){
  if (isRunning) { return false; }
#if defined( USING_STRUCT_TMS )
  end.tms_utime = (clock_t) -1; 
  end.tms_stime = (clock_t) -1;
  begin.tms_utime = 0;
  begin.tms_stime = 0;
#elif defined( USING_CLOCK_T )
  end = (clock_t) -1;
  begin = 0;
#endif
  userTicks = 0;
  systemTicks = 0;
  nLaps = 0;
  return true;
}

inline stopwatch& stopwatch::operator+=( const stopwatch& timer ) {
  if ( timer.isRunning ) { return *this; }
  userTicks += timer.queryTotalUserTicks();
  systemTicks += timer.queryTotalSystemTicks();
  return *this;
}


inline stopwatch& stopwatch::operator-=( const stopwatch& timer ) {
  if ( timer.isRunning ) { return *this; }
  userTicks -= timer.queryTotalUserTicks();
  systemTicks -= timer.queryTotalSystemTicks();
  return *this;
}


inline stopwatch::ticks stopwatch::queryTotalUserTicks() const{
  if (isRunning) { return -1; }
#if defined( USING_STRUCT_TMS )
  if ( end.tms_utime == -1 ) { return userTicks; }
  return userTicks + (end.tms_utime - begin.tms_utime);
#elif defined( USING_CLOCK_T )
  if ( end == -1 ) { return userTicks; }
  return userTicks + (end - begin);
#else
  return 0;
#endif

}

inline stopwatch::ticks stopwatch::queryTotalSystemTicks() const {
  if (isRunning) { return -1; }
#if defined( USING_STRUCT_TMS )
  if ( end.tms_utime == -1 ) { return systemTicks; }
  return systemTicks + (end.tms_stime - begin.tms_stime);
#elif defined( USING_CLOCK_T )
  return 0;
#else
  return 0;
#endif
}

inline stopwatch::ticks stopwatch::queryLapUserTicks() const {
#if defined( USING_STRUCT_TMS )
  if (end.tms_utime ==  -1) { return -1L; }
  return end.tms_utime - begin.tms_utime;
#elif defined( USING_CLOCK_T )
  if (end == (clock_t) -1) { return -1L; }
  return end-begin;
#else
  return 0;
#endif
}

inline stopwatch::ticks stopwatch::queryLapSystemTicks() const { 
#if defined( USING_STRUCT_TMS )
  if (end.tms_stime ==  -1) { return -1L; }
  return end.tms_stime - begin.tms_stime;
#elif defined( USING_CLOCK_T )
  if (end == (clock_t) -1) { return -1L; }
  return 0;
#else
  return 0;
#endif
}

inline int stopwatch::queryNLaps() const { return nLaps; }

#define STOPWATCH_TICKS_PER_SEC (stopwatch::queryTicksPerSec())

SPINDLE_END_NAMESPACE

#if defined( USING_STRUCT_TMS )
#undef USING_STRUCT_TMS
#endif

#if defined( USING_CLOCK_T )
#undef USING_CLOCK_T
#endif

#endif
