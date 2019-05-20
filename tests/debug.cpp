// System include(s):
#include "cor/cor.hpp"
#include "modules/pool.hpp"

#include <thread>
#include <vector>
#include <numeric>

// POOL include(s)
//#include "include/CpuTimer.h"
//#include "include/Rand.h"
//#include "include/PoolAdditional.h"
#include "modules/utils.hpp"

// ROOT include(s):
#include "TROOT.h"
#include "TChain.h"
#include "TString.h"
#include "TError.h"


// TO REMOVE
#include <chrono>
#include <thread>

extern "C"
{
  //Int_t Main(int argc, char *argv[]);
  void Main(int argc, char *argv[]);
}
 
//void readFiles(Int_t id)
static Int_t fid = 0;
static std::vector<int> vec(2);
static constexpr idm_t MASTER = 0;

void readFiles(void *)
{
  auto domain = cor::GetDomain();
  //auto agent_idp = domain->GetActiveResourceIdp();
  //auto group_size= GroupSize(agent_idp);
  //auto agent_idm =Rank(agent_idp);
  
  // Set up the TChain for reading all the files:
  TChain chain( "CollectionTree" );
  // for( Int_t i = 0; i < 10; ++i ) {
  // chain.Add( TString::Format( "/Users/amp/rootPlacor/file%i.root", i ) );
  //  }
  
  chain.Add( TString::Format( "/Users/brunoribeiro/Desktop/rootfiles/file%i.root", fid ) );

  // Set up reading the variables:
  Int_t intVar = 0;
  chain.SetBranchAddress( "IntVar", &intVar );
  Float_t floatVar = 0;
  chain.SetBranchAddress( "FloatVar", &floatVar );

  // Summary variables:
  Int_t intSum = 0;
  Float_t floatSum = 0;

  // Loop over the events:
  const Long64_t entries = chain.GetEntries();

  //int beg, end; 
  std::size_t beg, end; 
  beg = 0;                    // initialize [beg, end) to global range
  end = entries;
  //auto rank= Rank();
  auto rank = GetRank<cor::Group>();
  //AgentRange(beg, end);  // now [beg, end) is range for this thread
  AgentRange<cor::Group>(beg, end);
  //std::cout << " beg " << beg << "end " << end << " fid: " << fid<< std::endl;
  
  for( Long64_t entry = beg; entry < end; ++entry ) {
    // Load the current event:
    const Long64_t treeEntry = chain.LoadTree( entry );
    if( chain.GetEntry( treeEntry ) <= 0 ) {
      Error( "readFiles", "Failed to read entry %lld", entry );
      return;
    }
    // Increment the sums:
    intSum += intVar;
    floatSum += floatVar;
  }

  // Print the summed variables:
  auto nF = TString::Format( "file%i.root", fid);
    
  //Info("", "file = %s, Entries = %lli, id = %i, intSum = %i, floatSum = %g", nF.Data(), entries, agent_idm, intSum, floatSum );
  vec[rank]=intSum;
}

COR_REGISTER_TYPE(cor::Data<int>); 
//Int_t Main(int argc, char *argv[])
void Main(int argc, char *argv[])
{
  size_t arg=2;

  //CpuTimer TR;         // object to measure execution times
  auto domain = cor::GetDomain();
  auto agent_idp = domain->GetActiveResourceIdp();
  auto agent = domain->GetLocalResource<cor::Agent<void(int,char**)>>(agent_idp);
  if (argc >=1) { arg= std::atoi(argv[0]); vec.resize(arg);}
  size_t sleep = 0;
  //if (argc==2) sleep=std::atoi(argv[1]);
  // Enable thread correctness for ROOT:
  ROOT::EnableThreadSafety();

  auto data = domain->CreateLocal<cor::Data<int>>(domain->Idp(), "data_", 0);

    auto comm_idp = domain->GetPredecessorIdp(agent_idp);
    auto comm = domain->GetLocalResource<cor::Communicator>(comm_idp);
    auto comm_size = comm->GetTotalMembers();
    //auto rank = comm->GetIdm(agent_idp);
  
    auto pool = new cor::Pool(arg);
    //auto mutex = domain->CreateLocal<cor::Mutex>(domain->Idp(), "Guarda");
  
  Int_t result=0;
  std::size_t beg, end;
  beg =0; 
  end = 10;  //number of files
  //AgentRangeComm(beg, end);
  AgentRange<cor::Communicator>(beg, end);
  auto rank = GetRank<cor::Communicator>();
  fid= beg; 
  //std::cout << "arranque" << rank << std::endl;
  for( int i = beg; i <end ; ++i ) {
    //TR.Start();
    pool->Dispatch(readFiles, nullptr);
    pool->WaitForIdle();
    result = std::accumulate(vec.begin(), vec.end(), result);
    //TR.Stop();
    //TR.Report();
    ++fid;
  }
  //auto dataId= TString::Format( "data_%i", 0 ); 

   //auto data = domain->CreateLocal<cor::Data<int>>(domain->Idp(), "data_", result); //std::ref(result));

   data->AcquireWrite();
   auto value=data->Get();
   *value=result;
   data->ReleaseWrite();
  
  //std::cout << "Int Value: " << result << std::endl;
  //cor::ResourcePtr<cor::Data<int>> data;

   auto meta_domain = domain->GetLocalResource<cor::Domain>(cor::MetaDomain);
   auto member_list = meta_domain->GetMemberList();
   auto total_members = meta_domain->GetTotalMembers();

    //std::cout << rank << "\t" << domain->Idp() << "\t" << domain->GetTotalMembers() << std::endl;

   Int_t FinalValue=result;

   if (rank != MASTER) {
       cor::Message msg;
       agent->Send(comm->GetIdp(0), msg);
   } else {
       for (int i = 0; i < comm_size - 1; ++i)
            auto dummy = agent->Receive();
   }

//std::this_thread::sleep_for(std::chrono::seconds(2));

  if (rank==MASTER) {
    auto comm_size = GetSize<cor::Communicator>();
/*
    for (int i = 0; i < total_members; ++i)
      std::cout << member_list[i] << "\n";
*/
    // std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
// for (auto x: member_list) std::cout << x << " .";
// std::cout <<'n';
    // for (auto j=1 ; j < comm_size ; ++j) {
    //   auto msg = agent->Receive();
    //   auto pool_id = msg.Get<idp_t>();
    //   std::cout << "Pool_ID: " << pool_id<< std::endl;
    // }

    std::remove(member_list.begin(), member_list.end(), cor::MetaDomain);
    std::remove(member_list.begin(), member_list.end(), domain->Idp());

    //std::cout << "DEBUG -> " << member_list.size() << std::endl;

   for (auto j=0 ; j < member_list.size() - 2; ++j)
     {
        //std::cout << "RANK <" << j << "> DOMAIN <" << member_list[j] << ">" << std::endl;

       //std::cout << "Loop Value: " << j<< std::endl;
       auto dataId=TString::Format( "data_%i", j);
       //std::cout << "dataId: " << '\n';
       auto domain_distributed = domain->CreateReference<cor::Domain>(member_list[j], domain->Idp(), "");

       //std::cout << domain_distributed->Idp() << "\t" << domain_distributed->GetTotalMembers() << std::endl;

       //std::cout << "domain_distributed: " << '\n';
       auto data_idp = domain_distributed->GetIdp("data_");
       //std::cout << "data_idp: " << '\n';
       auto data = domain->CreateReference<cor::Data<int>>(data_idp, domain->Idp(), dataId.Data());
       //std::cout << "data: " << '\n';

       data->AcquireRead();
       auto value = data->Get();
       std::cout << FinalValue << "\t" << *value << "\n";
       FinalValue+= *value;
       data->ReleaseRead();
     }

   std::cout << "Final Value: " << FinalValue << std::endl;
  }
  else { 
    
    //cor::Message msg;
    //msg.SetType(0);
    //msg.Add<idp_t> ( agent_idp); 
    //auto master_idp= comm->GetIdp(MASTER);
  
    //agent->Send(master_idp, msg);

  }

  //std::this_thread::sleep_for(std::chrono::seconds(2));

  //return FinalValue;
}

/* 
// Start some file reading threads:
    std::vector< std::thread > threads;
    for( Int_t i = 0; i < 8; ++i ) {
        threads.emplace_back( readFiles, i );
    }

    // Wait for all of them to finish:
    for( std::thread& thread : threads ) {
        thread.join();
    }
*/
