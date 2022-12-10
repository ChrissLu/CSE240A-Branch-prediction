//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include <string.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "NAME";
const char *studentID   = "PID";
const char *email       = "EMAIL";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//
uint32_t ghistory;
uint8_t * gBHR;

uint8_t counter;    //for choosing predictor in TOURNAMENT, 2-bit
uint8_t * lBHR;
uint32_t *lPHT;
uint8_t *choiceT;
uint8_t lres;
uint8_t gres;


uint32_t hindex;
uint32_t pindex;
uint32_t choiceIndex;

// used for custom predictor
int8_t * weight_table;
int threshold = 5;
int y_pred;
  
//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

void printWeight(uint32_t index){
  int8_t* weight = &weight_table[index * (ghistoryBits+1)];
  printf("%d: [", index);
  for(int i=0; i<(ghistoryBits+1);++i){
    printf("%d, ", weight[i]);
  }
  printf("]. y_pred = %d\n", y_pred);
}

// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //
  ghistory = 0;
  gBHR = (uint8_t *)malloc((1U << ghistoryBits) * sizeof(uint8_t));
  memset(gBHR,0,(1U << ghistoryBits) * sizeof(uint8_t));

  lBHR = (uint8_t *)malloc((1U << lhistoryBits) * sizeof(uint8_t));
  memset(lBHR,0,(1U << ghistoryBits) * sizeof(uint8_t));
  lPHT = (uint32_t *)malloc((1U << pcIndexBits) * sizeof(uint32_t));
  memset(lPHT,0,(1U << ghistoryBits) * sizeof(uint32_t));
  choiceT = (uint8_t *)malloc((1U << lhistoryBits) * sizeof(uint8_t));
  memset(choiceT,0,(1U << ghistoryBits) * sizeof(uint8_t));



  // Init custom branch predictor
  // size of custom branch predictor is weight_table + ghistory
  // size of weight_table is (2^pcIndexBits * (ghistoryBits+1)) Bytes
  // If size of weight_table is 64K bits, then  pcIndexBits = 10, ghistoryBits = 7
  //                                            pcIndexBits = 9, ghistoryBits = 15
  //                                            pcIndexBits = 8, ghistoryBits = 31
  weight_table = (int8_t *)malloc((1U << pcIndexBits) * (ghistoryBits+1) * sizeof(int8_t));
  memset(weight_table, 0, (1U << pcIndexBits) * (ghistoryBits+1) * sizeof(int8_t));
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      hindex = (pc ^ ghistory)&((1U << ghistoryBits) - 1);
      if(gBHR[hindex]== WT || gBHR[hindex]== ST) return TAKEN;
      else return NOTTAKEN;
    case TOURNAMENT:
      choiceIndex = ghistory &((1U << ghistoryBits) - 1);
      counter = choiceT[choiceIndex];
  
        pindex = pc & ((1U << pcIndexBits) - 1);
        hindex = lPHT[pindex]&((1U << lhistoryBits) - 1);
        if(lBHR[hindex]== WT || lBHR[hindex]== ST) lres = TAKEN;
        else lres = NOTTAKEN;
      
        hindex = pc &((1U << ghistoryBits) - 1);
        if(gBHR[hindex]== WT || gBHR[hindex]== ST) gres = TAKEN;
        else gres = NOTTAKEN;
      

      return counter<2 ? lres:gres;
    case CUSTOM:
    {
      uint32_t weight_select_index = pc & ((1U << pcIndexBits) - 1);
      int8_t* weight = &weight_table[weight_select_index * (ghistoryBits+1)];
      y_pred = 0;
      for(int i=0; i<ghistoryBits; ++i){
        if((ghistory >> i) & 0x1){
          y_pred += weight[i];
        } else{
          y_pred -= weight[i];
        }
      }
      y_pred += weight[ghistoryBits]; // constant term
      //printWeight(weight_select_index);
      return y_pred >= 0? TAKEN: NOTTAKEN;
    }
      
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//



void
train_predictor(uint32_t pc, uint8_t outcome)
{
  switch (bpType) {
    case STATIC:
      break;
    case GSHARE:
      hindex = (pc ^ ghistory) &((1U << ghistoryBits) - 1);
      ghistory = (ghistory<<1) | outcome;
      if(gBHR[hindex]==ST && outcome==TAKEN)  gBHR[hindex]=ST;
      else if(gBHR[hindex]==SN && outcome==NOTTAKEN) gBHR[hindex]=SN;
      else gBHR[hindex] += outcome==TAKEN?1:-1;
      break;
    case TOURNAMENT:
      choiceIndex = ghistory &((1U << ghistoryBits) - 1);
      counter = choiceT[choiceIndex];

        if(outcome==lres && outcome!=gres && counter!=0) choiceT[choiceIndex]--;
        if(outcome==gres && outcome!=lres && counter!=3) choiceT[choiceIndex]++;

      pindex = pc & ((1U << pcIndexBits) - 1);
      hindex = lPHT[pindex]&((1U << lhistoryBits) - 1);
      if(lBHR[hindex]==ST && outcome==TAKEN)  lBHR[hindex]=ST;
      else if(lBHR[hindex]==SN && outcome==NOTTAKEN) lBHR[hindex]=SN;
      else lBHR[hindex] += outcome==TAKEN?1:-1;
      lPHT[pindex] = (lPHT[pindex]<<1) | outcome;

      hindex = pc &((1U << ghistoryBits) - 1);
      if(gBHR[hindex]==ST && outcome==TAKEN)  gBHR[hindex]=ST;
      else if(gBHR[hindex]==SN && outcome==NOTTAKEN) gBHR[hindex]=SN;
      else gBHR[hindex] += outcome==TAKEN?1:-1;
      ghistory = (ghistory<<1) | outcome;
      break;
    case CUSTOM:
    {
      if((y_pred >= 0 && outcome == NOTTAKEN) || (y_pred < 0 && outcome == TAKEN) || abs(y_pred) <= threshold) {
        uint32_t weight_select_index = pc & ((1U << pcIndexBits) - 1);
        int8_t* weight = &weight_table[weight_select_index * (ghistoryBits+1)];
        int t = outcome==TAKEN? 1: -1;
        for(int i=0; i<ghistoryBits; ++i) {
          int x = ((ghistory >> i) & 0x1) == 1? 1: -1;
          //if(abs(weight[i]) < threshold)
            weight[i] += t*x;
        }
        weight[ghistoryBits] += t;
        //printf("Update: ");
        //printWeight(weight_select_index);
      }
      ghistory = (ghistory<<1) | outcome;
      break;
    }
    default:
      break;
  }
}

