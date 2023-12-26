#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "sim_proc.h"
#include <list> 
#include <vector>
#include <iostream>
#include <iomanip>
using namespace std;
#define DBG 1


///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//                                  DEBUGGING SET OF FUNCTIONS
//TO DO: (Add list to do later, here)
//1> To copy between lists: First_List.assign(Second_List.begin(),Second_list.end())
//2> My fetching bundle might be a cycle late, check it later in debug.
//3> int m = decode_bundle.back().dst; {To read data from list of structs}
//4> 
//5>
//6>
//7>
///////////////////////////////////////////////////////////////////////////////////////////////////////
void printing_bundle(list<bundle> bnd){

    cout << " PC in bundle      " << hex << bnd.front().pc      << std::endl;
    cout << " optcode in bundle "        << bnd.front().op_type << endl;
    cout << " dst in bundle     "        << bnd.front().dst     << endl;
    cout << " src1 in bundle    "        << bnd.front().src1    << endl;
    cout << " src2 in bundle    "        << bnd.front().src2    << endl;
    
};


void printstruct (bundle print_bundle){
    cout << " Seq No :          " << fixed  << print_bundle.seq     << endl;
    cout << " PC in bundle      " << hex    << print_bundle.pc      << endl;
    cout << " optcode in bundle "           << print_bundle.op_type << endl;
    cout << " dst in bundle     "           << print_bundle.dst     << endl;
    cout << " src1 in bundle    "           << print_bundle.src1    << endl;
    cout << " src2 in bundle    "           << print_bundle.src2    << endl;
    cout << "___________________"           << "___________________"<< endl;
};

void print(list<bundle> list)
{ //auto const&
        for ( auto const& i: list) {
        printstruct(i);
    }

};



///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//                                 Implementing Functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////////


//LOOP_CHECK
//////////////////////////////////////////////////////////////////////////////////////////////////////

bool loop_check(bool &end_processor){
	return end_processor;
}




// FETCH_STAGE
///////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Check for fetch stalling [CRITICAL]
void Fetch(struct proc_params* params,FILE *FP,list<bundle> &decode_bundle,list<bundle> &rename_bundle,uint &seq,int &clk,bool &fetch_end,uint &final_pc){
    bundle bnd;
    uint64_t pc;
    int op_type;
    int dest;
    int src1;
    int src2;
	if(decode_bundle.size()!=0) return;
    if(fscanf(FP, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) == EOF){
		fetch_end = true;
        // ONE CORNER CASE EXISTS AS FINAL PC DOESNT EXSIST
		return;
	}
    else{

        bnd.pc      =      pc;
        bnd.op_type = op_type;
        bnd.dst     =    dest;
        bnd.src1    =    src1;
        bnd.src2    =    src2;
        bnd.seq     =     seq;
        bnd.cyc     =       0;
        bnd.src_p1  =    src1;
        bnd.src_p2  =    src2;
        bnd.dst_p   =    dest;
        seq++                ;
        bnd.fetch_entry = clk;
        bnd.fetch_stay  =   1;
        bnd.decode_entry = clk + 1;
        final_pc = bnd.seq;
        decode_bundle.push_back(bnd);
        for(unsigned long int i=0;i<params->width-1;i++){
            if(fscanf(FP, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2)!=EOF){
                bnd.pc      =      pc;
                bnd.op_type = op_type;
                bnd.dst     =    dest;
                bnd.src1    =    src1;
                bnd.src2    =    src2;
                bnd.src_p1  =    src1;
                bnd.src_p2  =    src2;
                bnd.dst_p   =    dest;
                bnd.seq     =     seq;
                bnd.fetch_entry = clk;
                bnd.fetch_stay  =   1;
                final_pc = bnd.seq;
                bnd.decode_entry = clk + 1;
                seq++                ;
                decode_bundle.push_back(bnd);
            } else {
                fetch_end = true;
                final_pc = bnd.seq;
                //cout << final_pc << endl;
            }
        }
    }

};

//DECODE_STAGE
///////////////////////////////////////////////////////////////////////////////////////////////////////

void decode(list<bundle> &rename_bundle,list<bundle> &decode_bundle,int &clk){
    if(decode_bundle.size()!=0){
        if(rename_bundle.size() == 0) {
            for(list<bundle>::iterator itr=decode_bundle.begin();itr!=decode_bundle.end();itr++){
                itr->rename_entry = clk + 1;
            }
            rename_bundle.assign(decode_bundle.begin(),decode_bundle.end());
            decode_bundle.clear();
        }
        else{ 
       
            return;
        }
    }
};

//  RENAME_STAGE
///////////////////////////////////////////////////////////////////////////////////////////////////////
void rename(list<bundle> &rename_bundle,list<bundle> &regread_bundle,vector<robuffer> &ROB,vector<rmt> &RMT,uint &tail,int width,int &clk){
    int empty_slot = ROB.size();
    bool rob_full;
    for(uint i=0;i<ROB.size();i++){if(ROB.at(i).valid == true) empty_slot--;}
    if(empty_slot < width) rob_full =  true;
    else                   rob_full = false;
    if(regread_bundle.size() != 0 || rob_full == true) return;
    else if(regread_bundle.size() == 0 && rob_full == false){
        for (list<bundle>::iterator itr=rename_bundle.begin(); itr!=rename_bundle.end(); itr++) {
            itr->regread_entry = clk+1;
            ROB.at(tail).valid = true;
            ROB.at(tail).ready = false;
            ROB.at(tail).instruction = *itr;
            if(itr->src1 != -1){
                if(RMT.at(itr->src1).valid_bit == true) itr->src1 = RMT.at(itr->src1).rob_tag;
                else itr->src1 = -1;}
            if(itr->src2 != -1){
                if(RMT.at(itr->src2).valid_bit == true) itr->src2 = RMT.at(itr->src2).rob_tag;
                else itr->src2 = -1;}
            ROB.at(tail).dest  = itr->dst;
            if(itr->dst != -1){
                RMT.at(itr->dst).valid_bit = true;
                RMT.at(itr->dst).rob_tag   = tail;}
            itr->dst = tail;
            tail++;
            if(tail == ROB.size()) tail = 0;
        }
        regread_bundle.assign(rename_bundle.begin(),rename_bundle.end());
        rename_bundle.clear();
    }
    return;
};


//  REGISTER_READ_STAGE
//  Regread bundle should get the bypass at the last cycle of execution
///////////////////////////////////////////////////////////////////////////////////////////////////////

void regread(list<bundle> &regread_bundle,list<bundle> &dispatch_bundle,vector<robuffer> &ROB,int &clk){
    if(regread_bundle.size()!=0){
        if(dispatch_bundle.size()!=0){
            return;
        }
        else{
            for(list<bundle>::iterator itr = regread_bundle.begin();itr!=regread_bundle.end();itr++){
                if(itr->src1 != -1){if(ROB.at(itr->src1).ready == true) itr->src1 = -1;}
                if(itr->src2 != -1){if(ROB.at(itr->src2).ready == true) itr->src2 = -1;}
                itr->dispatch_entry=clk+1;
            }
        
           dispatch_bundle.assign(regread_bundle.begin(),regread_bundle.end());
           regread_bundle.clear(); 
        }
    } else return;
};


//DISPATCH_STAGE
///////////////////////////////////////////////////////////////////////////////////////////////////////


void dispatch(int &clk,list<bundle> &dispatch_bundle,vector<iq_entry> &IQ,unsigned long int iq_size,vector<robuffer> &ROB,uint width){

    list<bundle>::iterator itr;


    //Checking for IQ space
    uint empty_slot = IQ.size();
    bool iq_full = false;
    for(uint i=0;i<IQ.size();i++){
        if(IQ.at(i).exist == true) empty_slot--;
    }
    if(empty_slot < width)                  iq_full =  true;
    else                                    iq_full = false;
    if(!dispatch_bundle.empty()){
        if(iq_full == false){
            for(itr = dispatch_bundle.begin(); itr!=dispatch_bundle.end(); itr++) {
                itr->issue_entry=clk+1; // REMIND NIS !!! CRITICAL: His mistake
                for(uint i=0;i<iq_size;i++){
                    if(IQ.at(i).exist == true) continue;
                    IQ.at(i).valid = false;
                    IQ.at(i).instruction = *itr;
                    IQ.at(i).exist=true;
                    break;
                }
                
            }
            dispatch_bundle.clear();
        }
    }
};


//ISSUE_STAGE
///////////////////////////////////////////////////////////////////////////////////////////////////////
// LOGIC //////////////
//step  1: Temp bundle;
//step  2: Integer of Timer
//step  3: for loop 0 to width.
//step  4: issue up till width
//step  5: Issue if you have a ready instruction

void issue(int &clk,uint width,vector<iq_entry> &IQ,vector<execute_list> &execute_list_bundle){
    execute_list temp_exe;
    int max = INT32_MAX;  // Increase this for latter traces
    int iq_index = -1;
    uint empty_check = 0;
    for(uint i = 0; i < IQ.size();i++){
        if(IQ.at(i).exist == false) empty_check++;
    }
    if(empty_check == IQ.size()) return;
    for(uint i=0;i<width;i++){
        max = INT32_MAX;
        iq_index = -1;
        if( execute_list_bundle.size() == width*5) return;
        
        for(uint j =0; j<IQ.size();j++){
            if(IQ.at(j).instruction.src1 == -1 && IQ.at(j).instruction.src2 == -1) IQ.at(j).valid = true;
            if(IQ.at(j).valid == false || IQ.at(j).exist == false) continue;
            if(IQ.at(j).instruction.src1 != -1 || IQ.at(j).instruction.src2 != -1) continue;
        
            if(IQ.at(j).exist == true && IQ.at(j).valid == true && (IQ.at(j).instruction.seq < max)){
                max = IQ.at(j).instruction.seq;
                iq_index = j;
            }
        }
        if(iq_index == -1) return;
        IQ.at(iq_index).exist = false;
        IQ.at(iq_index).instruction.execute_entry = clk+1;
        if(IQ.at(iq_index).instruction.op_type == 0)      temp_exe.cycle = 1;
        else if(IQ.at(iq_index).instruction.op_type == 1) temp_exe.cycle = 2;
        else if(IQ.at(iq_index).instruction.op_type == 2) temp_exe.cycle = 5;
        temp_exe.exist = true;
        temp_exe.instruction = IQ.at(iq_index).instruction;
        
        execute_list_bundle.push_back(temp_exe);
    }
    return;
};


//EXECUTE_STAGE
///////////////////////////////////////////////////////////////////////////////////////////////////////

void execute(int &clk,vector<execute_list> &execute_list_bundle,list<bundle> &writeback,list<bundle> &dispatch_bundle, list<bundle> &regread_bundle,vector<iq_entry> &IQ){
    int comp = 1;

    if(execute_list_bundle.size()==0) return;
    for(vector<execute_list>::iterator itr = execute_list_bundle.begin();itr!=execute_list_bundle.end();itr++){
        if (itr->cycle == comp) {
            // Waking up the registers in Issue Queue
            for(uint i =0; i < IQ.size();i++){
                if(IQ.at(i).exist == true){
                    if(IQ.at(i).instruction.src1 == itr->instruction.dst) IQ.at(i).instruction.src1 = -1;
                    if(IQ.at(i).instruction.src2 == itr->instruction.dst) IQ.at(i).instruction.src2 = -1;
                }            
            }

            // Waking up in Dispatch bundle
            for(list<bundle>::iterator itr1 = dispatch_bundle.begin();itr1!=dispatch_bundle.end();itr1++){
                if(itr1->src1 == itr->instruction.dst) itr1->src1 = -1;
                if(itr1->src2 == itr->instruction.dst) itr1->src2 = -1;
            }

            // Waking up in RegisterRead
            for(list<bundle>::iterator itr1 = regread_bundle.begin();itr1!=regread_bundle.end();itr1++){
                if(itr1->src1 == itr->instruction.dst) itr1->src1 = -1;
                if(itr1->src2 == itr->instruction.dst) itr1->src2 = -1;
            }
            itr->instruction.writeback_entry = clk + 1;
            writeback.push_back(itr->instruction);
            execute_list_bundle.erase(itr);
            itr--;
        } else {
            itr->cycle--;
        }
            
    }
};

//WRITE_BACK STAGE
///////////////////////////////////////////////////////////////////////////////////////////////////////

void Writeback(list<bundle> &writeback,vector<robuffer> &ROB,int &clk){
    for(list<bundle>::iterator itr = writeback.begin();itr!=writeback.end();itr++){
        itr->retire_entry = clk+1;
        for(uint i = 0;i<ROB.size();i++){
            if(ROB.at(i).instruction.seq == itr->seq){
                ROB.at(i).instruction = *itr;
                ROB.at(i).ready = true;}
        }
    }
    writeback.clear();
};

//RETIRE STAGE
///////////////////////////////////////////////////////////////////////////////////////////////////////

void retire(struct stats *statistics,vector<robuffer> &ROB,vector<rmt> &RMT,uint &head,uint &tail,int bundle_width,int &clk,bool &fetch_end,uint &final_check,bool &end_processor){
    robuffer head_entry;
    bundle bnd;
    uint finalising;
    if((head == tail) && !ROB.at(head).valid){
        return;
    }else{
        for(int i = 0;i<bundle_width;i++){
            head_entry = ROB.at(head);
            bnd = head_entry.instruction;
            finalising = bnd.seq;
            if(head_entry.ready == false || head_entry.valid == false) return;

            int dest = ROB.at(head).dest;
            if(dest!= -1){
                if(RMT.at(dest).rob_tag == (int) head){
                    RMT.at(dest).valid_bit = false;

                } 
            }           


            cout << bnd.seq <<" fu{"<< bnd.op_type << "} src{"<< bnd.src_p1 <<","<< bnd.src_p2 <<"} dst{"<< bnd.dst_p;
            cout << "} FE{" << bnd.fetch_entry      << "," << bnd.decode_entry - bnd.fetch_entry ;
            cout << "} DE{" << bnd.decode_entry     << "," << bnd.rename_entry - bnd.decode_entry;
            cout << "} RN{" << bnd.rename_entry     << "," << bnd.regread_entry - bnd.rename_entry; 
            cout << "} RR{" << bnd.regread_entry    << "," << bnd.dispatch_entry - bnd.regread_entry;
            cout << "} DI{" << bnd.dispatch_entry   << "," << bnd.issue_entry - bnd.dispatch_entry;
            cout << "} IS{" << bnd.issue_entry      << "," << bnd.execute_entry - bnd.issue_entry;
            cout << "} EX{" << bnd.execute_entry    << "," << bnd.writeback_entry - bnd.execute_entry;
            cout << "} WB{" << bnd.writeback_entry  << "," << bnd.retire_entry - bnd.writeback_entry;
            cout << "} RT{" << bnd.retire_entry     << "," << clk - bnd.retire_entry + 1 <<"}"<< endl;

            if(fetch_end == true){
                if(final_check == finalising) end_processor = true;
            }

            statistics->Dynamic_instruction++;
            ROB.at(head).valid = false;
            head++;
            if(head == ROB.size()) head = 0;
        }
    }
};


//////////////////////////////////////////////// MAIN CODE ////////////////////////////////////

int main (int argc, char* argv[])
{
    FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;
    proc_params params;       // look at sim_bp.h header file for the the definition of struct proc_params
    //int op_type, dest, src1, src2;  // Variables are read from trace file
    //uint64_t pc; // Variable holds the pc read from input file
    

    // Processor register variables

    list<bundle>    decode_bundle;                               // decode register stage
    list<bundle>    rename_bundle;                               // rename register stage
    list<bundle>   regread_bundle;
    list<bundle>  dispatch_bundle;
    list<bundle>   writeback_list;
    stats statistics;

    // Processor buffers
    vector<robuffer>              ROB;
    vector<rmt>               RMT(67);
    vector<iq_entry>               IQ;
    vector<execute_list> execute_list;

    // ROB locator pointers
    uint tail                 = 0;
    uint head                 = 0;



    if (argc != 5)
    {
        printf("Error: Wrong number of inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }
    
    params.rob_size     = strtoul(argv[1], NULL, 10);
    params.iq_size      = strtoul(argv[2], NULL, 10);
    params.width        = strtoul(argv[3], NULL, 10);
    trace_file          = argv[4];
    // printf("rob_size:%lu "
    //         "iq_size:%lu "
    //         "width:%lu "
    //         "tracefile:%s\n", params.rob_size, params.iq_size, params.width, trace_file);
    // Open trace_file in read mode
    FP = fopen(trace_file, "r");
    if(FP == NULL)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }
    
    // Resizing ROB and RMT
    ROB.resize(params.rob_size);
    IQ.resize(params.iq_size)  ;

    //Initialising Sequence Number for Instructions
    uint seq = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////
//
// The following loop just tests reading the trace and echoing it back to the screen.
//
// Replace this loop with the "do { } while (Advance_Cycle());" loop indicated in the Project 3 spec.
// Note: fscanf() calls -- to obtain a fetch bundle worth of instructions from the trace -- should be
// inside the Fetch() function.
//  while(fscanf(FP, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) != EOF)
//      printf("%lx %d %d %d %d\n", pc, op_type, dest, src1, src2); //Print to check if inputs have been read correctly
//
///////////////////////////////////////////////////////////////////////////////////////////////////////
   
   for(uint i = 0; i < IQ.size(); i++){
    IQ.at(i).exist = false;
    IQ.at(i).valid = false;
   }
   statistics.Dynamic_instruction = 0;
   uint final_check;
   int clk = 0;
   bool fetch_end = false;
   bool end_processor = false;
do{
	retire(&statistics,ROB,RMT,head,tail,params.width,clk,fetch_end,final_check,end_processor);
    Writeback(writeback_list,ROB,clk);
    execute(clk,execute_list,writeback_list,dispatch_bundle,regread_bundle,IQ);
    issue(clk,params.width,IQ,execute_list);
    dispatch(clk,dispatch_bundle,IQ,params.iq_size,ROB,params.width);
    regread(regread_bundle,dispatch_bundle,ROB,clk);
    rename(rename_bundle,regread_bundle,ROB,RMT,tail,params.width,clk);
    decode(rename_bundle,decode_bundle,clk);
    Fetch(&params,FP,decode_bundle,rename_bundle,seq,clk,fetch_end,final_check);
    cout << "  Rob head :" << head << " ROB Tail  :" <<tail<<endl;
    clk++;
	}while(end_processor == false);

    cout << "# === Simulator Command =========" << endl;
    cout << "# "<<argv[0]<<" "<<argv[1]<<" "<<argv[2]<<" "<<argv[3]<<" "<<argv[4]<< endl;
    cout << "# === Processor Configuration ===" << endl;
    cout << "# ROB_SIZE = " <<argv[1]                 << endl;
    cout << "# IQ_SIZE  = " <<argv[2]                    << endl;
    cout << "# WIDTH    = "   <<argv[3]                    << endl;
    cout << "# === Simulation Results ========" << endl;
    cout << "# Dynamic Instruction Count    = "<< statistics.Dynamic_instruction << endl;
    cout << "# Cycles                       = "<< clk << endl;
    cout << "# Instructions Per Cycle (IPC) = " << fixed<<setprecision(2) << (float)statistics.Dynamic_instruction/(float)clk << endl;














// for(uint i = 0;i<ROB.size();i++){
// 			uint idx = i + head;
// 			if(idx >= ROB.size()) idx = idx - ROB.size();
// 			bundle bnd = ROB.at(idx).instruction;
// 			cout << bnd.seq <<" fu{"<< bnd.op_type << "} src{"<< bnd.src_p1 <<","<< bnd.src_p2 <<"} dst{"<< bnd.dst_p;
//             cout << "} FE{" << bnd.fetch_entry      << "," << bnd.decode_entry - bnd.fetch_entry ;
//             cout << "} DE{" << bnd.decode_entry     << "," << bnd.rename_entry - bnd.decode_entry;
//             cout << "} RN{" << bnd.rename_entry     << "," << bnd.regread_entry - bnd.rename_entry; 
//             cout << "} RR{" << bnd.regread_entry    << "," << bnd.dispatch_entry - bnd.regread_entry;
//             cout << "} DI{" << bnd.dispatch_entry   << "," << bnd.issue_entry - bnd.dispatch_entry;
//             cout << "} IS{" << bnd.issue_entry      << "," << bnd.execute_entry - bnd.issue_entry;
//             cout << "} EX{" << bnd.execute_entry    << "," << bnd.writeback_entry - bnd.execute_entry;
//             cout << "} WB{" << bnd.writeback_entry  << "," << bnd.retire_entry - bnd.writeback_entry;
//             cout << "} RT{" << bnd.retire_entry     << "," << clk - bnd.retire_entry + 1 <<"}"<< endl;


// 	}
  return 0;
}

/////////////////////// DEBUG NOTES /////////////////////////////////
// FETCH  :: WORKING [Fixed the Seq]
// DECODE :: 
//
//
//
/////////////////////////////////////////////////////////////////////
