/** \copyright
 * Copyright (c) 2012, Stuart W Baker
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are  permitted provided that the following conditions are met:
 * 
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \file gc_format.cxx
 * Implementations of GridConnect packet parser and formatter routines.
 *
 * @author Balazs Racz
 * @date 20 May 2013
 */

//#define LOGLEVEL VERBOSE

#include <stdint.h>
#include <gc_format.h>

extern "C" {

/** Build an ASCII character representation of a nibble value (uppercase hex).
 * @param nibble to convert
 * @return converted value
 */
static char nibble_to_ascii(int nibble)
{
    nibble &= 0xf;

    if (nibble < 10)
    {
        return ('0' + nibble);
    }
    
    return ('A' + (nibble - 10));
}

/** Tries to parse a hex character to a nibble. Understands both upper and
    lowercase hex.
    @param c is the character to convert.
    @return a converted value, or -1 if an invalid character was encountered.
*/
static int ascii_to_nibble(const char c)
{
    if ('0' <= c && '9' >= c)
    {
        return c - '0';
    }
    else if ('A' <= c && 'F' >= c)
    {
        return c - 'A' + 10;
    }
    else if ('a' <= c && 'f' >= c)
    {
        return c - 'a' + 10;
    }
    return -1;
}

/** from JMRI class def
009 * Class for GridConnect messages for a CAN hardware adapter.
010 * <p>
011 * The GridConnect protocol encodes messages as an ASCII string of up to 24
012 * characters of the form: :ShhhhNd0d1d2d3d4d5d6d7;
013 * <p>
014 * The S indicates a standard
015 * CAN frame :XhhhhhhhhNd0d1d2d3d4d5d6d7; The X indicates an extended CAN frame
016 * hhhh is the two byte header N or R indicates a normal or remote frame, in
017 * position 6 or 10 d0 - d7 are the (up to) 8 data bytes
018 * <p>
019 *
020 * @author Andrew Crosland Copyright (C) 2008
021 */


int gc_format_parse_olcb(olcbMsg* thisMsg, lnReceiveBuffer* buf)
{
	CAN_frame_t thisFrame;
	if (gc_format_parse_rx(buf, &thisFrame) >= 0)
	{
		thisMsg->priority = (thisFrame.MsgID & 0x10000000) >> 28;
		thisMsg->frameType = (thisFrame.MsgID & 0x08000000) >> 27;
		thisMsg->canFrameType = (thisFrame.MsgID & 0x07000000) >> 24;
		thisMsg->MTI = (thisFrame.MsgID & 0x00FFF000) >> 12;
		thisMsg->srcAlias = thisFrame.MsgID & 0xFFF;
//		thisMsg->dstAlias = 
		thisMsg->dlc = thisFrame.FIR.B.DLC;
		for (int i = 0; i < thisMsg->dlc; i++)
			thisMsg->olcbData.u8[i] = thisFrame.data.u8[i];
		return 0;
	}
	else
		return -1;
}

int gc_format_parse_rx(lnReceiveBuffer* buf, CAN_frame_t* can_frame)
{
	lnTransmitMsg bufMsg;
	bufMsg.msgType = buf->msgType;
	bufMsg.lnMsgSize = buf->lnMsgSize;
	bufMsg.reqID = buf->reqID;
	bufMsg.reqRecTime = buf->reqRecTime;
	memcpy(bufMsg.lnData, buf->lnData, lnMaxMsgSize);
	return gc_format_parse_tx(&bufMsg, can_frame);
}


int gc_format_parse_tx(lnTransmitMsg* buf, CAN_frame_t* can_frame)
{
	uint8_t bufIndex = 1; //leading : is ignored
	
//    CLR_CAN_FRAME_ERR(*can_frame);
    if (buf->lnData[bufIndex] == 'X')
    {
//        SET_CAN_FRAME_EFF(*can_frame); 
		can_frame->FIR.B.FF = CAN_frame_ext;
    }
    else if (buf->lnData[bufIndex] == 'S') 
    {
//        CLR_CAN_FRAME_EFF(*can_frame);
		can_frame->FIR.B.FF = CAN_frame_std;
    } else
    {
        // Unknown packet type.
//        SET_CAN_FRAME_ERR(*can_frame);
        return -1;
    }
    bufIndex++;
    uint32_t id = 0;
    while (1)
    {
        int nibble = ascii_to_nibble(buf->lnData[bufIndex]);
        if (nibble >= 0)
        {
            id <<= 4;
            id |= nibble;
            ++bufIndex;
        }
        else if (buf->lnData[bufIndex] == 'N')
        {
            // end of ID, frame is coming.
 //           CLR_CAN_FRAME_RTR(*can_frame);
			can_frame->FIR.B.RTR = CAN_no_RTR;
            ++bufIndex;
            break;
        }
        else if (buf->lnData[bufIndex] == 'R')
        {
            // end of ID, remote frame is coming.
 //           SET_CAN_FRAME_RTR(*can_frame);
			can_frame->FIR.B.RTR = CAN_RTR;
            ++bufIndex;
            break;
        }
        else
        {
            // This character should not happen here.
//            SET_CAN_FRAME_ERR(*can_frame);
            return -1;
        }
    } // while parsing ID

//    if (IS_CAN_FRAME_EFF(*can_frame))
    if (can_frame->FIR.B.FF)
    {
//        SET_CAN_FRAME_ID_EFF(*can_frame, id);
        can_frame->MsgID = id & 0x1FFFFFFFU;
    }
    else
    { 
//        SET_CAN_FRAME_ID(*can_frame, id);
        can_frame->MsgID = id & 0x7ffU;
    }
    int index = 0;
    while (char(buf->lnData[bufIndex]) != ';')
    {
        int nh = ascii_to_nibble(buf->lnData[bufIndex++]);
        int nl = ascii_to_nibble(buf->lnData[bufIndex++]);
        if (nh < 0 || nl < 0)
        {
//            SET_CAN_FRAME_ERR(*can_frame);
            return -1;
        }
        can_frame->data.u8[index++] = (nh << 4) | nl;
    } // while parsing data
    can_frame->FIR.B.DLC = index;
//    CLR_CAN_FRAME_ERR(*can_frame);
    return 0;
}

/// Helper function for appending to a buffer ONCE.
///
/// @param dst buffer to append data to
/// @param value one character to append to.
///
static void output_single(uint8_t*& dst, char value)
{
    *dst++ = (uint8_t)value;
    
}

/// Helper function for appending to a buffer TWICE. Used in the implementation
/// of the double-byte gridconnect output protocol.
///
/// @param dst buffer to append data to
/// @param value one character to append to.
///
static void output_double(uint8_t*& dst, char value)
{
    *dst++ = (uint8_t)value;
    *dst++ = (uint8_t)value;
}

/** Formats a can frame in the GridConnect protocol.

    If requested, it can create the double protocol with leading !!, trailing ;;
    and all the characters doubled.

    If the input frame is an error frame, then does not output anything and
    returns the input pointer.

    @param can_frame is the input frame.

    @param buf is the output buffer. The caller must ensure this is big enough
    to hold the resulting frame (28 or 56 bytes).

    @param double_format if non-zero, the doubling format will be generated.

    @return the pointer to the buffer character after the formatted can frame.
*/
int gc_format_generate(CAN_frame_t* can_frame, lnReceiveBuffer* buf, bool double_format)
{
	uint8_t * wrPtr = &(buf->lnData[0]);
    void (*output)(uint8_t*& dst, char value);
    if (double_format)
    {
        output = output_double;
        output(wrPtr, '!');
    }
    else
    {
        output = output_single;
        output(wrPtr, ':');
    }
    uint32_t id;
    int offset;
    if (can_frame->FIR.B.FF == CAN_frame_ext) 
//    if (IS_CAN_FRAME_EFF(*can_frame))
    {
//        id = GET_CAN_FRAME_ID_EFF(*can_frame);
        id = can_frame->MsgID;
        output(wrPtr, 'X');
        offset = 28;
    }
    else
    {
//        id = GET_CAN_FRAME_ID(*can_frame);
        id = can_frame->MsgID;
        output(wrPtr, 'S');
        offset = 8;
    }
    for (;offset >= 0; offset -= 4)
    {
        output(wrPtr, nibble_to_ascii((id >> offset) & 0xf));
    }
    /* handle remote or normal */
    
//    if (IS_CAN_FRAME_RTR(*can_frame))
    if (can_frame->FIR.B.RTR == CAN_RTR) 
    {
        output(wrPtr, 'R');
    }
    else
    {
        output(wrPtr, 'N');
    }
    for (offset = 0; offset < can_frame->FIR.B.DLC; ++offset)
    {
        output(wrPtr, nibble_to_ascii(can_frame->data.u8[offset] >> 4));
        output(wrPtr, nibble_to_ascii(can_frame->data.u8[offset] & 0xf));
    }
    output(wrPtr, ';');
    
    if (gc_generate_newlines > 0) 
    {
//        output(wrPtr, ord('\n'));
    }
    return 1;
    
}

} //extern C

