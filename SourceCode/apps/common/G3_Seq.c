#include "G3_Seq.h"
#include <stdint.h>
#include <stdbool.h>

uint16_t getSendSeq(uint16_t *sendSeq)
{
    if (sendSeq)
    {
        uint16_t seq = *sendSeq;
        if (seq == 0xFFFF)
        {
            *sendSeq = 0;
        }
        else
        {
            *sendSeq = *sendSeq + 1;
        }
        return seq;
    }
    return 0;
}

bool checkRecvSeqIsValid(uint16_t *oldSeq, uint16_t seq)
{
    if (seq == *oldSeq || (seq == 0 && *oldSeq == 0xFFFF))
    {
        *oldSeq = *oldSeq + 1;
        return true;
    }
    return false;
}

void resetSendSeq(uint16_t *sendSeq)
{
    if (sendSeq)
    {
        *sendSeq = 0;
    }
}

void resetRecvSeq(uint16_t *recvSeq)
{
    if (recvSeq)
    {
        *recvSeq = 0;
    }
}
