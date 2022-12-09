//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
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

uint8_t counter;    //for choosing predictor in TOURNAMENT
uint32_t lhistory;
uint8_t * lBHR;


uint32_t index;
  
//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //
  ghistory = 0;
  gBHR = malloc((1 << ghistoryBits) * sizeof(uint8_t));

  counter = 0;
  lhistory = 0;
  lBHR = malloc((1 << ghistoryBits) * sizeof(uint8_t));


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
      index = (pc ^ ghistory)&((1 << ghistoryBits) - 1);
      if(gBHR[index]== WT || gBHR[index]== ST) return TAKEN;
    case TOURNAMENT:
      if (counter>=2){
        index = pc & ((1 << lhistoryBits) - 1);
        return lBHR[index];
      }
      else{
        index = pc &((1 << ghistoryBits) - 1);
        return gBHR[index];
      }
    case CUSTOM:
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


// uint8_t bitCount(uint32_t n)
// {
//     uint8_t c =0 ; 
//     for (c =0; n; n >>=1) 
//         c += n &1 ; 
//     return c ;
// }



void
train_predictor(uint32_t pc, uint8_t outcome)
{
  switch (bpType) {
    case STATIC:
      break;
    case GSHARE:
      index = (pc ^ ghistory) &((1 << ghistoryBits) - 1);
      ghistory = (ghistory<<1) | outcome;
      if(gBHR[index]==ST && outcome==TAKEN)  gBHR[index]=ST;
      else if(gBHR[index]==SN && outcome==NOTTAKEN) gBHR[index]=SN;
      else gBHR[index] += outcome==TAKEN?1:-1;

    case TOURNAMENT:
      if (counter>=2){
        if(outcome==1 && counter==2) counter++;
        if(outcome==0) counter--; 
        index = pc & ((1 << lhistoryBits) - 1);
        lBHR[index] = outcome;
        lhistory = (lhistory<<1) | outcome;
      }
      else{
        if(outcome==1 && counter==1) counter--;
        if(outcome==0) counter++; 
        index = pc &((1 << ghistoryBits) - 1);
        gBHR[index] = outcome;
        ghistory = (ghistory<<1) | outcome;
      }
    case CUSTOM:
    default:
      break;
  }
}

