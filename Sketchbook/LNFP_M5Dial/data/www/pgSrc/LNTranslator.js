var sigOffset = 2;

function getPlainMsgText(lnData)
{
    var retMsg = "";
    var p1, p2, p3;
    switch(lnData[0])
    {
        case 0x81 : retMsg = "System busy, please wait"; break;
        case 0x82 : retMsg = "Switch track power on MAIN to OFF"; break;
        case 0x83 : retMsg = "Switch track power on MAIN to ON"; break;
        case 0x85 : retMsg = "Set track power on MAIN to IDLE"; break;
        case 0xA0 : retMsg = "Set Loco speed for slot " + lnData[1].toString() + " to " + lnData[2].toString(); break;
        case 0xA1 : retMsg = "Set function group 1 for slot " + lnData[1].toString() + " to 0x" + ("00" + lnData[2].toString(16)).slice(-2); break;
        case 0xA2 : retMsg = "Set function group 2 for slot " + lnData[1].toString() + " to 0x" + ("00" + lnData[2].toString(16)).slice(-2); break;
        case 0xB0 : //REQ SWITCH
            p1 = ((lnData[1] & 0x7F) + ((lnData[2] & 0x0F)<<7));
            retMsg = "Set switch " + p1.toString()  + " to " + (((lnData[2] & 0x20)>0) ? "closed":"thrown") + " with coil " + (((lnData[2] & 0x10)>0) ? "ON":"OFF"); 
            break;
        case 0xB1 : //Turnout SENSOR state REPORT
            p1 = ((lnData[1] & 0x7F) + ((lnData[2] & 0x0F)<<7));
            switch (lnData[2] & 0x40)
            {
                case 0x00:
                    retMsg = "Report position of switch " + p1.toString()  + " Closed " + (((lnData[2] & 0x20)>0) ? "ON":"OFF") + " thrown " + (((lnData[2] & 0x10)>0) ? "ON":"OFF");
                    break;
                case 0x40:
                    retMsg = "Report position of switch " + p1.toString()  + " " + (((lnData[2] & 0x20)>0) ? "SWITCH ":"AUX ") + (((lnData[2] & 0x10)>0) ? "HIGH":"LOW");
                    break;
            }
            break;
        case 0xB2 : //OPC_INPUT_REP
            p1 = (((lnData[1] & 0x7F)<<1) + ((lnData[2] & 0x0F)<<8) + ((lnData[2] & 0x20)>>5));
            retMsg = "Report status of block detector " + p1.toString() + " as " + (((lnData[2] & 0x10)>0) ? "OCCUPIED":"FREE"); 
            break;
        case 0xB4 : //LACK Long acknowledge
            if (lnData[1] === 0)
            {
                retMsg = "Acknowledge of message failed with response 0x" + ("00" + lnData[2].toString(16).toUpperCase()).slice(-2); 
                break;
            }
            else
                if (lastLACKMsg.length > 0)
                switch (lnData[1] | 0x80)
                {
                    case 0xBA: //move slots
                        retMsg = (lnData[2] === 0) ? "Illegal move. Command not accepted": "Slot Move command accepted";
                        break;
                    case 0xBC: //Switch Status
                        p1 = ((lastLACKMsg[1] & 0x7F) + ((lastLACKMsg[2] & 0x0F)<<7));
                        retMsg = "Current position of switch " + p1.toString() + " is " + ((lnData[2] & 0x20) ? "closed":"thrown");
                        break;
                    case 0xBD: //OPC_SW_ACK
                        if (lnData[2] === 0)
                            retMsg = "FIFO full. Command not accepted";
                        else
                        {
                            p1 = ((lastLACKMsg[1] & 0x7F) + ((lastLACKMsg[2] & 0x0F)<<7));
                            retMsg = "Accepted force set switch " + p1.toString()  + " to " + (((lastLACKMsg[2] & 0x20)>0) ? "closed":"thrown") + " with coil " + (((lastLACKMsg[2] & 0x10)>0) ? "ON":"OFF"); 
                        }
                        break;
                    case 0xBF: 
                        retMsg = "Loco slot " + (lnData[2] === 0) ? "not": lnData[2].toString() + " assigned";
                        break;
                    case 0xED: //immediate command
                        retMsg = (lnData[2] === 0) ? "DCC Buffer full. Command failed": "DCC Packet accepted";
                        break;
                    case 0xEF:
                        switch (lastLACKMsg[2])
                        {
                            case 0x7C:
                                switch (lnData[2])
                                {
                                    case 0x7F: retMsg = "Function NOT implemented, no reply";
                                        break;
                                    case 0x00: retMsg = "Programmer BUSY , task aborted, no reply";
                                        break;
                                    case 0x01: retMsg = "Task accepted. Please wait for result";
                                        break;
                                    case 0x40: retMsg = "Task accepted blind. No result";
                                        break;
                                }
                                break;
                            case 0x7B: 
                                retMsg = "Confirm FastClock (Rate " + lastLACKMsg[3].toString() + ") to " + ("00" + (24 - 128 + lastLACKMsg[8]).toString()).slice(-2) + ":" + ("00" + (60 - 128 + lastLACKMsg[6]).toString()).slice(-2); 
                                break;
                            default:
                                retMsg = "Acknowledge of message " + ((lnData[1] === 0) ? "failed" : getOpCodeStr(lnData[1] | 0x80) + " with response 0x" + ("00" + lnData[2].toString(16).toUpperCase()).slice(-2)); 
                                break;
                        }
                        break;
                }
            break;
        case 0xB5 : retMsg = "Set slot " + lnData[1].toString() + " to status 0x" + ("00" + lnData[2].toString(16).toUpperCase()).slice(-2); break;
        case 0xB5 : retMsg = "Set function group 1 in consist slot " + lnData[1].toString() + " to 0x" + ("00" + lnData[2].toString(16).toUpperCase()).slice(-2); break;
        case 0xB8 : retMsg = "Unlink slot " + lnData[1].toString() + " from slot " + lnData[2].toString(); break;
        case 0xB9 : retMsg = "Link slot " + lnData[1].toString() + " into slot " + lnData[2].toString(); break;
        case 0xBA : 
            if (lnData[1] == lnData[2]) //NULL move
                retMsg = "Set slot " + lnData[1].toString() + " to \"In Use\"";
            else
                if (lnData[1] === 0) //null source
                    retMsg = "Move loco from dispatch slot to slot " + lnData[2].toString();
                else
                    if (lnData[2] === 0) //null destination
                        retMsg = "Move loco from slot " + lnData[1].toString() + " to dispatch slot";
                    else
                        retMsg = "Ooops! Invalid slot move attempt from " + lnData[1].toString() + " to " + lnData[2].toString();
            break;
        case 0xBB : retMsg = "Request information from slot " + lnData[1].toString(); break;
        case 0xBC : 
            p1 = ((lnData[1] & 0x7F) + ((lnData[2] & 0x0F)<<7));
            retMsg = "Request status of switch " + p1.toString();
            break;
        case 0xBD : 
            p1 = ((lnData[1] & 0x7F) + ((lnData[2] & 0x0F)<<7));
            retMsg = "Force set switch " + p1.toString()  + " to " + (((lnData[2] & 0x20)>0) ? "closed":"thrown") + " with coil " + (((lnData[2] & 0x10)>0) ? "ON":"OFF"); 
            break;
        case 0xBF : retMsg = "Request slot information for loco addr " + ((lnData[1]<<7) + lnData[2]).toString(); break;
        case 0xD0 : retMsg = decodeMultiSenseMsg(lnData); break;
        case 0xD4 : retMsg = "OPC_UHLI-FUN"; break;
        case 0xD7 : retMsg = "OPC_PANEL_RESPONSE"; break;
        case 0xDF : retMsg = "OPC_PANEL_QUERY"; break;
        case 0xE0 : switch (lnData[1])
					{
						case 0x09: retMsg = decodeMultiSenseLongMsg(lnData); break; //long
						default: "Unknown message: " + lnData;
							break;
					} 
					break;
        case 0xE4 : p1 = ((lnData[4] & 0x7F) + ((lnData[3] & 0x1F)<<7)); //sensor id
					p2 = ((lnData[6] & 0x7F) + ((lnData[5] & 0x7F)<<7)); //loco address
					p3 = (lnData[3] & 0x20) ? "south" : "north";
					switch (lnData[2])
                    {
						case 0x00: retMsg = "Lissy IR Report for Sensor " + p1.toString() + " - Loco " + p2.toString() + " going " + p3; 
							break;
						case 0x40: retMsg = "Lissy Wheel Count Report for Sensor " + p1.toString() + " - " + p2.toString() + " axles going " + ((lnData[3] & 0x20) > 0) ? "north" : "south"; 
							break;
						case 0x41: retMsg = "Lissy RFID x Report for Sensor " + p1.toString(); 
							break;
						default: "Unknown Lissy Sensor Report " + lnData;
							break;
					}
					break;
        case 0xE5 : retMsg = decodePeerMsg(lnData); break;
        case 0xE6 : retMsg = "OPC_ALM_READ"; break;
        case 0xE7 : switch (lnData[2])
                    {
                        case 0x7B: retMsg = "Slot Data for FastClock slot "; break;
                        case 0x7C: 
                            if (lnData[4] === 0) { retMsg = "Read CV " + (lnData[9]+1).toString() + " Value " + (lnData[10] + ((lnData[8] & 0x02) << 6)).toString(); break;} 
                            if (lnData[4] & 0x01) { retMsg = "Service Mode track empty"; break; }
                            if (lnData[4] & 0x02) { retMsg = "No Write acknowledge response from decoder"; break; }
                            if (lnData[4] & 0x04) { retMsg = "Failed to detect READ response from decoder"; break; }
                            if (lnData[4] & 0x08) { retMsg = "User Aborted this command"; break; }
                            retMsg = "Unknown response from Service track";
                            break;
                        default: retMsg = "Slot data for slot " + lnData[2].toString();
                    }
                    break;
        case 0xED : 
            var dccData = [];
            var bitMask = 0x01;
            for (var i = 0; i < (lnData[3] & 0x70)>>4; i++)
            {
                dccData.push(lnData[5+i] | ((lnData[4] & bitMask) << (7-i)));
                bitMask = bitMask << 1;
            }
            if (dccData.length >= 2)
                retMsg = decodeDCCPacket(dccData);
            else
                retMsg = "Trying to send invalid DCC packet";
            break;
        case 0xEE : retMsg = "OPC_WR_LIM_DATA"; break;
        case 0xEF : switch (lnData[2])
                    {
                        case 0x7B: retMsg = "Set FastClock (Rate " + lnData[3].toString() + ") to " + ("00" + (24 - 128 + lnData[8]).toString()).slice(-2) + ":" + ("00" + (60 - 128 + lnData[6]).toString()).slice(-2); break;
                        case 0x7C: 
                            var decAddr = (lnData[5] << 7) + lnData[6];
                            retMsg = (((lnData[3] & 0x04) > 0) ? ("OpsMode Addr " + decAddr.toString())  : "Service Mode") + (((lnData[3] & 0x40) > 0) ? " write" : " read")  + " task CV " + (lnData[9]+1).toString() + " Val " + (lnData[10] + ((lnData[8] & 0x02) << 6)).toString(); break;
                        default: retMsg = "Update slot information of slot " + lnData[2].toString();
                    }
                    break;
    }
    return retMsg;
}

function decodeDCCPacket(packetData)
{
    //identify packet type
    if (packetData[0] === 0) //Broadcast address
    {
        return "Broadcast Data";
    }
    if ((packetData[0] > 0) && (packetData[0] <= 0x7F)) //Multi-Function decoders with 7 bit addresses
    {
        return "Multi-Function decoders with 7 bit address " + packetData[0].toString();
    }
    if ((packetData[0] > 0x7F) && (packetData[0] <= 0xBF)) //Basic Accessory Decoders with 9 bit and Extended Accessory 
                                                            //Decoders with 11-bit addresses
    {
        if (packetData.length == 2)
           return "Basic Accessory Decoder";
        else
        {
            var baseAddr = (((~packetData[1]) & 0x70) << 2) | ( packetData[0] & 0x3F);
            var groupAddr = (packetData[1] & 0x06) >> 1;
            var pairAddr = packetData[1] & 0x01;
            var accStatus = (packetData[1] & 0x08)>>3;
            var mastAddr = (4 * baseAddr) + groupAddr - sigOffset;
            return "DCC command to Signal Mast " + mastAddr.toString() + " aspect " + (packetData[2] & 0x1F).toString();
        }
    }                                                        
    if ((packetData[0] > 0xBF) && (packetData[0] <= 0xE7)) //Multi Function Decoders with 14 bit addresses
    {
		var locoAddr = ((packetData[0] & 0x3F) << 8) + packetData[1];
		switch (packetData[2] & 0xE0)
		{
			case 0xA0:
				return "MF Decoder 14bit Addr " + locoAddr.toString() + " Fct Group 9-12 Value " + (packetData[2] & 0x0F).toString(16).toUpperCase();
				break;
			case 0xC0:
				switch (packetData[2] & 0x1F)
				{
					case 0x1E: 
						return "MF Decoder 14bit Addr " + locoAddr.toString() + " Fct Group 13-20 Value " + packetData[3].toString(16).toUpperCase();
						break;
					case 0x1F:
						return "MF Decoder 14bit Addr " + locoAddr.toString() + " Fct Group 21-28 Value " + packetData[3].toString(16).toUpperCase();
						break;
				}
				break;
			default: return "Multi Function Decoders with 14 bit addresses";
		}
    }
    if ((packetData[0] > 0xE7) && (packetData[0] <= 0xFE)) //Reserved for Future Use
    {
        return "Reserved for Future Use";
    }
    if (packetData[0] == 0xFF) //Idle Packet 
    {
        return "Idle Packet ";
    }
    return "Invalid DCC Command";
}
    
function decodePeerMsg(msgData)
{
    if (msgData[1] == 0x10)
    {
        switch (msgData[4])
        {
            case 0x01:
                return "SV programming format 1 message";
                break;
            case 0x02:
                switch (msgData[3])
                {
                    case 0x71: //
                        switch (msgData[11])
                        {
                            case 0x01: 
                                var btnAddr = (msgData[8] & 0x3F) + ((msgData[9] & 0x3F)<<6); 
                                var btnVal = (msgData[12] & 0x3F); 
                                var btnEvt = btnVal.toString();
                                switch (btnVal)
                                {
                                    case 0: btnEvt = "Btn Down"; break;
                                    case 1: btnEvt = "Btn Up"; break;
                                    case 2: btnEvt = "Btn Click"; break;
                                    case 3: btnEvt = "Btn Hold"; break;
                                    case 4: btnEvt = "Btn Double Click"; break;
                                }
                                return "Button " + btnAddr.toString() + " Event " + btnEvt; 
                            case 0x00: 
                                var analogAddr = (msgData[8] & 0x3F) + ((msgData[9] & 0x3F)<<6); 
                                var analogVal = (msgData[12] & 0x3F) + ((msgData[13] & 0x3F)<<6); 
                                return "Analog Input " + analogAddr.toString() + " Value " + analogVal.toString(); 
                                break;
                            default: "Invalid Universal Input Data"; break;
                        }
                        break;
                    default:
                        return "SV programming format 2 message";
                        break;
                }
            default:
                return "invalid SV message format " + msgData[3].toString();
                break;
        }
    }
    else
        return "Peer to peer message. Meaning depending on device";
}

function decodeMultiSenseLongMsg(sensData)
{
	var zoneAddr = 0;
	var locoAddr = 0;
	
	zoneAddr = ((sensData[2] & 0x1F) << 7) + sensData[3];
	var locoDetect = (sensData[2] & 0x20);
	locoAddr = (sensData[4] << 7) + sensData[5];
	var trackDir = (sensData[6] & 0x40);
	var thisDir = trackDir ? " forward" : " backward";
	if (locoDetect)
		return "Detected locomotive " + locoAddr.toString() + " in block " + zoneAddr.toString() + thisDir;
	else
        return "Release locomotive " + locoAddr.toString() + " from block " + zoneAddr.toString() + thisDir;
}

function decodeMultiSenseMsg(sensData)
{
    switch (sensData[1] & 0x60)
    {
        case 0x00: //0xD0, 0x00, 0x04, 0x07, 0x0E, 0x22
            var zoneAddr = (((sensData[1] & 0x1F) << 7) + (sensData[2] & 0x7F));
            var locoAddr;
            if (sensData[3] == 0x7E)
                locoAddr = sensData[4] & 0x7F;
            else
                locoAddr = (sensData[3] << 7) + (sensData[4] & 0x7F);
            return "Release locomotive " + locoAddr.toString() + " from block " + zoneAddr.toString();
            break;
        case 0x20: 
            var zoneAddr = (((sensData[1] & 0x1F) << 7) + (sensData[2] & 0x7F));
            var locoAddr;
            if (sensData[3] == 0x7E)
                locoAddr = sensData[4] & 0x7F;
            else
                locoAddr = (sensData[3] << 7) + (sensData[4] & 0x7F);
            return "Detected locomotive " + locoAddr.toString() + " in block " + zoneAddr.toString();
            break;
        case 0x60:
            switch ((sensData[1] & 0x0E) >> 1)
            {
                case 0x01:
                    var pmAddr = ((sensData[1] & 0x01) << 7) + sensData[2];
                    return "Power Manager " + pmAddr.toString() + " Message ";
                    break;
                case 0x02:
                    return "Barcode reader message";
                    break;
                default:
                    return "Invalid OPC_MULTI_SENSE Device Message";
                    break;
            }
        default:
            return "unspecified OPC_MULTI_SENSE message";
    }
   
}
