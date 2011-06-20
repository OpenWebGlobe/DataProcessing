/*******************************************************************************
#      ____               __          __  _      _____ _       _               #
#     / __ \              \ \        / / | |    / ____| |     | |              #
#    | |  | |_ __   ___ _ __ \  /\  / /__| |__ | |  __| | ___ | |__   ___      #
#    | |  | | '_ \ / _ \ '_ \ \/  \/ / _ \ '_ \| | |_ | |/ _ \| '_ \ / _ \     #
#    | |__| | |_) |  __/ | | \  /\  /  __/ |_) | |__| | | (_) | |_) |  __/     #
#     \____/| .__/ \___|_| |_|\/  \/ \___|_.__/ \_____|_|\___/|_.__/ \___|     #
#           | |                                                                #
#           |_|                                                                #
#                                                                              #
#                                (c) 2011 by                                   #
#           University of Applied Sciences Northwestern Switzerland            #
#                     Institute of Geomatics Engineering                       #
#                           martin.christen@fhnw.ch                            #
********************************************************************************
*     Licensed under MIT License. Read the file LICENSE for more information   *
*******************************************************************************/


#include "Timer.h"
#include <cassert>
#include <algorithm>

/*
-----------------------------------------------------------------------------
Parts of this source file is part of OGRE 
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/


//static bool bTimerInitDone = false;

//*****************************************************************************
// WINDOWS CODE
//*****************************************************************************
#if defined(_MSC_VER)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define FREQUENCY_RESAMPLE_RATE 200

class Win32Timer
{
private:
   DWORD mStartTick;
   LONGLONG mLastTime;
   LARGE_INTEGER mStartTime;
   LARGE_INTEGER mFrequency;

   DWORD_PTR mProcMask;
   DWORD_PTR mSysMask;

   HANDLE mThread;

   DWORD mQueryCount;
public:    
   Win32Timer() { reset(); }
   void reset();
   unsigned long getMilliseconds();
   unsigned long getMicroseconds();
   unsigned long getNanoseconds();
};

void Win32Timer::reset()
{    
   QueryPerformanceFrequency(&mFrequency);
   QueryPerformanceCounter(&mStartTime);
   mStartTick = GetTickCount();
   mLastTime = 0;
   mQueryCount = 0;

   // Save the current process
   HANDLE mProc = GetCurrentProcess();

   // Get the current Affinity
   GetProcessAffinityMask(mProc, &mProcMask, &mSysMask);

   mThread = GetCurrentThread();
}
//-------------------------------------------------------------------------
unsigned long Win32Timer::getMilliseconds()
{
   LARGE_INTEGER curTime;

   // Set affinity to the first core
   SetThreadAffinityMask(mThread, 1);

   // Query the timer
   QueryPerformanceCounter(&curTime);

   // Reset affinity
   SetThreadAffinityMask(mThread, mProcMask);

   // Resample the frequency
   mQueryCount++;
   if(mQueryCount == FREQUENCY_RESAMPLE_RATE)
   {
      mQueryCount = 0;
      QueryPerformanceFrequency(&mFrequency);
   }

   LONGLONG newTime = curTime.QuadPart - mStartTime.QuadPart;

   // scale by 1000 for milliseconds
   unsigned long newTicks = (unsigned long) (1000 * newTime / mFrequency.QuadPart);

   // detect and compensate for performance counter leaps
   // (surprisingly common, see Microsoft KB: Q274323)
   unsigned long check = GetTickCount() - mStartTick;
   signed long msecOff = (signed long)(newTicks - check);
   if (msecOff < -100 || msecOff > 100)
   {
      // We must keep the timer running forward :)
      LONGLONG adjust = (std::min)(msecOff * mFrequency.QuadPart / 1000, newTime - mLastTime);
      mStartTime.QuadPart += adjust;
      newTime -= adjust;

      // Re-calculate milliseconds
      newTicks = (unsigned long) (1000 * newTime / mFrequency.QuadPart);
   }

   // Record last time for adjust
   mLastTime = newTime;

   return newTicks;
}
//-------------------------------------------------------------------------
unsigned long Win32Timer::getMicroseconds()
{
   LARGE_INTEGER curTime;
   QueryPerformanceCounter(&curTime);
   LONGLONG newTime = curTime.QuadPart - mStartTime.QuadPart;

   unsigned long newTicks = (unsigned long) (1000 * newTime / mFrequency.QuadPart);

   // detect + compensate performance-counter-leaps
   // see Microsoft KB: Q274323
   unsigned long check = GetTickCount() - mStartTick;
   signed long msecOff = (signed long)(newTicks - check);
   if (msecOff < -100 || msecOff > 100)
   {
      LONGLONG adjust = (std::min)(msecOff * mFrequency.QuadPart / 1000, newTime - mLastTime);
      mStartTime.QuadPart += adjust;
      newTime -= adjust;
   }

   mLastTime = newTime;

   // scale by 1000000 for microseconds
   unsigned long newMicro = (unsigned long) (1000000 * newTime / mFrequency.QuadPart);

   return newMicro;
}
//-------------------------------------------------------------------------
unsigned long Win32Timer::getNanoseconds()
{
   LARGE_INTEGER curTime;
   QueryPerformanceCounter(&curTime);
   LONGLONG newTime = curTime.QuadPart - mStartTime.QuadPart;

   unsigned long newTicks = (unsigned long) (1000 * newTime / mFrequency.QuadPart);

   unsigned long check = GetTickCount() - mStartTick;
   signed long msecOff = (signed long)(newTicks - check);
   if (msecOff < -100 || msecOff > 100)
   {
      LONGLONG adjust = (std::min)(msecOff * mFrequency.QuadPart / 1000, newTime - mLastTime);
      mStartTime.QuadPart += adjust;
      newTime -= adjust;
   }

   mLastTime = newTime;

   // scale by 1000000000 for nanoseconds
   unsigned long newNano = (unsigned long) (1000000000 * newTime / mFrequency.QuadPart);

   return newNano;
}


//----------------------------------------------------------------------------

Win32Timer _internal_windows_timer;

//-----------------------------------------------------------------------------

void Timer::initTimer()
{
   //bTimerInitDone = true;
   _internal_windows_timer.reset();
}

//-----------------------------------------------------------------------------

unsigned int Timer::getRealTime()
{
   //assert(bTimerInitDone);
   return (unsigned int)_internal_windows_timer.getMilliseconds();
}

//-----------------------------------------------------------------------------

double Timer::getRealTimeHighPrecision()
{
   //assert(bTimerInitDone); // if you get assertion here you forgot to call initTimer!
   return ((double)_internal_windows_timer.getMicroseconds()) / 1000.0;
}

//-----------------------------------------------------------------------------

#else
//*****************************************************************************
// UNIX CODE
//*****************************************************************************
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

	void Timer::initTimer()
	{
      //bTimerInitDone = true;
	}

   //--------------------------------------------------------------------------

	unsigned int Timer::getRealTime()
	{
      //assert(bTimerInitDone); // if you get assertion here you forgot to call initTimer!

		timeval tv;
		gettimeofday(&tv, 0);
		return (unsigned int)(tv.tv_sec * 1000) + (tv.tv_usec / 1000);
	}
	
	double Timer::getRealTimeHighPrecision()
	{
      //assert(bTimerInitDone); // if you get assertion here you forgot to call initTimer!

      timeval tv;
      gettimeofday(&tv, 0);
      return (double)(tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);
	}

#endif

