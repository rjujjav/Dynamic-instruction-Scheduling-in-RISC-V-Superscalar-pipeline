#ifndef SIM_PROC_H
#define SIM_PROC_H

typedef struct proc_params{
    unsigned long int rob_size;
    unsigned long int iq_size;
    unsigned long int width;
}proc_params;

typedef struct bundle
{
    uint64_t  pc;
    int      dst;
    int  op_type;
    int     src1;
    int     src2;
    int      seq;
    int      cyc;

    // TIMERS
    int fetch_entry;
    int  fetch_stay;

    int  decode_entry;
    int  rename_entry;
    int regread_entry;
    int dispatch_entry;
    int issue_entry;
    int execute_entry;
    int writeback_entry;
    int retire_entry;
    int dst_p;
    int src_p1;
    int src_p2;
}bundle;

typedef struct stage_ready{
    bool        start_ready;
    bool        decod_ready;
    bool        renam_ready;
    bool        robuf_full_;
    bool        fetch_ready;
}stage_ready;

typedef struct robuffer
{
    bundle instruction;
    int           dest;
    bool         ready;
    bool         valid;


}robuffer;

typedef struct rmt
{
    bool valid_bit;
    int    rob_tag;
}rmt;

typedef struct iq_entry
{
    bool         valid;
    bundle instruction;
    bool         exist; // this is for knowing my vector empty or full

    // uint64_t  pc;
    // int      dst;
    // int  op_type;
    // int     src1;
    // int     src2;
    // int      seq;
    // int           src1;
    // bool      src1_rdy;
    // int           src2;
    // bool      src2_rdy;
    // int           dest;
    // bool      dest_rdy;
}iq_entry;

typedef struct execute_list{
    int          cycle;
    bundle instruction;
    bool         exist;
}execute_list;

typedef struct stats{
    int Dynamic_instruction;
}stats;



// Put additional data structures here as per your requirement

#endif
