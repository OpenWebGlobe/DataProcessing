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

/******************************************************************************/
/*                    A collection of useful MPI stuff                        */
/******************************************************************************/

#ifndef _MPI_UTILS_H
#define _MPI_UTILS_H

#include <mpi.h>
#ifdef _OPENMP
#include <omp.h>
#endif
#include <stack>
#include <vector>
#include <iostream>
#include <ctime>

// High Performance job Manager: Distribute workload asynchronously.

template<class SJob>
class MPIJobManager
{
public:
   typedef void (*CallBack_Process)(const SJob& job, int rank);

   MPIJobManager(int nMaxWorkSize)
   {
      _nMaxWorkSize = nMaxWorkSize;
      MPI_Comm_size(MPI_COMM_WORLD, &_totalnodes);
      MPI_Comm_rank(MPI_COMM_WORLD, &_rank);
#     ifdef _OPENMP
         _nMaxthreads = omp_get_max_threads();
#     else
         _nMaxthreads = 1;
#     endif
   }
   virtual ~MPIJobManager(){}

   bool IsRoot() { return (_rank == 0);}

   // Add job stack (
   void AddJobStack(const std::stack<SJob>& js) { if (_rank == 0) _jobstack = js;}

   // Add job vector (
   void AddJobs(const std::vector<SJob>& js) 
   {
      if (_rank == 0)
      {
         for(int i = 0; i < js.size(); i ++)
         {
            _jobstack.push(js[i]);
         }
      }
   }

   // or add (many) single jobs
   void AddJob(const SJob& j) 
   { 
      if (_rank == 0) 
      {
         _jobstack.push(j);
      }
   }

   //---------------------------------------------------------------------------
   // Start Processing data. For each job the specified callback function is called.
   // you can also pass some userdata to it. But please keep in mind everything must be thread safe.
   void Process(CallBack_Process fnc, bool bVerbose=false)
   {
      if (_rank == 0 && _totalnodes == 1)
      {
         std::vector<SJob> vJobs;
         while (_jobstack.size()>0)
         {
            vJobs.push_back(_jobstack.top());
            _jobstack.pop();
         }

#        pragma omp parallel for
         for (int i=0;i<(int)vJobs.size();i++)
         {
              fnc(vJobs[i], 0);
         }
      }

      if (_rank == 0) 
      {
         int totaljobs = (int)_jobstack.size();
         clock_t tprog0, tprog1;
         tprog0 = clock();

         if (bVerbose)
         {
            std::cout << "Jobmanager is starting...\n";
            std::cout << "Total jobs: " << totaljobs << "\n" << std::flush;
         }

         for (int i=1;i<_totalnodes;i++)
         {
            _MakeJobPacket(_jobstack, i);
         }
      
         while (_lstActiveRequests.size()>0)
         {
            std::vector<int> freenodes;

            std::list< std::pair<MPI_Request, int> >::iterator it = _lstActiveRequests.begin();
            if (it != _lstActiveRequests.end())
            {
               MPI_Request& req = it->first;
               int flag;
               MPI_Test(&req, &flag, MPI_STATUS_IGNORE);

               if (flag)
               {
                  int targetnode = it->second;
                  it = _lstActiveRequests.erase(it);
                  if (_jobstack.size()>0)
                  {
                     freenodes.push_back(targetnode);
                  }
               }
            }
            if(freenodes.size() > 0)
            {
               std::vector<SJob> vecJobsRoot =_MakeRootJobPacket(_jobstack);
               if(freenodes.size() > 1)
               {
                  for (size_t i=0;i<freenodes.size()-1;i++)
                  {
                     _MakeJobPacket(_jobstack, freenodes[i]);
                  }
               }
               // -- Process on rank 0
               std::cout << "-->>Rank " << _rank << " is processing " << vecJobsRoot.size()<< " jobs....!\n"<< std::flush;
               #pragma omp parallel for
               for (int i=0;i<(int)vecJobsRoot.size();i++)
               {
                    fnc(vecJobsRoot[i], _rank);
               }
               std::cout << "<<--Rank " << _rank << " finished processing " << vecJobsRoot.size()<< " jobs!\n"<< std::flush;
            }
         }

         // send terminate
         for (int i=1;i<_totalnodes;i++)
         {
            _SendTerminate(i);
         }
      } 
      else
      {
         std::vector<SJob> vecJobs;
         while (_ReceiveJobs(vecJobs))
         {
            std::cout << "-->>Rank " << _rank << " is processing " << vecJobs.size()<< " jobs....!\n"<< std::flush;
            #pragma omp parallel for
            for (int i=0;i<(int)vecJobs.size();i++)
            {
                 fnc(vecJobs[i], _rank);
            }
            std::cout << "<<--Rank " << _rank << " finished processing " << vecJobs.size()<< " jobs!\n"<< std::flush;
         }
      }
      MPI_Barrier(MPI_COMM_WORLD);
   }

   //---------------------------------------------------------------------------

protected:
   //---------------------------------------------------------------------------
   // Member variables
   //---------------------------------------------------------------------------
   int                                       _totalnodes;
   int                                       _rank;
   int                                       _nMaxWorkSize;
   int                                       _nMaxthreads;
   std::stack<SJob>                          _jobstack;
   std::list< std::pair<MPI_Request, int> >  _lstActiveRequests;
   //---------------------------------------------------------------------------
   // private methods
   //---------------------------------------------------------------------------
   void _SendJobs(std::vector<SJob>& vecJobs, int target)
   {
      MPI_Request request;

      std::cout << " ..Sending "<< vecJobs.size()<<" jobs to rank " << target << "\n"<< std::flush;
      int count = vecJobs.size() * sizeof(SJob);
      if (count > 0)
      {
         void* adr = (void*) &(vecJobs[0]);
         MPI_Isend (adr, count, MPI_BYTE, target, 77, MPI_COMM_WORLD, &request);
         _lstActiveRequests.push_back(std::pair<MPI_Request, int>(request, target));
      }
   }
   //---------------------------------------------------------------------------
   void _SendTerminate(int target)
   {
      unsigned char data = 88;
      MPI_Send(&data, 1, MPI_BYTE, target, 88, MPI_COMM_WORLD);
   }
   //---------------------------------------------------------------------------
   // receive jobs or return false if there are no more jobs!
   bool _ReceiveJobs(std::vector<SJob>& vecJobs)
   {
      vecJobs.clear();
      MPI_Request request;
      int flag = 0;
      MPI_Status status;
      int msglen;

      while(true)
      {
         int flag = 0;
         MPI_Iprobe(0, 88, MPI_COMM_WORLD, &flag, &status);
         
         if (flag) // "terminate"
         {
            std::cout << " ..Rank "<< _rank <<" received TERMINATE signal\n" << std::flush;
            char buffer;
            MPI_Irecv (&buffer, 1, MPI_BYTE, 0, 88, MPI_COMM_WORLD, &request);
            MPI_Wait(&request, &status);
            return false;
         }

         MPI_Iprobe(0, 77, MPI_COMM_WORLD, &flag, &status);
        
         if (flag)
         {
            MPI_Get_count(&status, MPI_BYTE, &msglen);
            vecJobs.resize(msglen / sizeof(SJob));
            void* adr = (void*) &(vecJobs[0]);
            MPI_Irecv (adr, msglen, MPI_BYTE, 0, 77, MPI_COMM_WORLD, &request);
            MPI_Wait(&request, &status);
            std::cout << " ..Rank "<< _rank <<" received "<< vecJobs.size() << " Jobs\n" << std::flush;
            return true;
         }
      }
   }
   //---------------------------------------------------------------------------
   void _MakeJobPacket( std::stack<SJob> &jobs, int i) 
   {
      std::vector<SJob> vJobs;
      for (int w=0;w<_nMaxWorkSize;w++)
      {
         if (jobs.size()>0)
         {
            vJobs.push_back(jobs.top());
            jobs.pop();
         }
      }

      if (vJobs.size()>0)
      {
         _SendJobs(vJobs, i);
      }
   }

   //---------------------------------------------------------------------------
   std::vector<SJob> _MakeRootJobPacket(std::stack<SJob> &jobs) 
   {
      std::vector<SJob> vJobs;
      for (int w=0;w<_nMaxWorkSize;w++)
      {
         if (jobs.size()>0)
         {
            vJobs.push_back(jobs.top());
            jobs.pop();
         }
      }
      return vJobs;
   }

private:
   MPIJobManager(){}
   MPIJobManager(const MPIJobManager&){}
};




#endif