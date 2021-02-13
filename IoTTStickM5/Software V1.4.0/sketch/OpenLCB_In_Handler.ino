//OpenLCB messages coming in from Communication module side, e.g. LN Driver, MQTT, or Gateway

/*
void callbackOpenLCBMessage(lnReceiveBuffer * newData) //this is the landing point for incoming OpenLCB messages
                                                       //from OpenLCB, MQTT, or Gateway
{

//add code here for interpretation

//  Serial.printf("Process OLCB App Callback %i\n", newData->reqID & 0xC000);
  int index = 0;
  if (useM5Viewer == 3)
    processOLCBtoM5(newData);
  if (globalClient != NULL)
    processDataToOLCBWebClient("OLCB", newData);

  if ((newData->reqID & 0xC000) > 0) //original message not from App, so send to Hat
  {
    if (usbSerial) //BrownHat active
      usbSerial->lnWriteMsg(*newData);
  }
}

void processDataToOLCBWebClient(String thisCmd, lnReceiveBuffer * newData) //if a web browser is conneted, all LN messages are sent via Websockets
                                                     //this is the hook for a web based OLCB viewer
{
    DynamicJsonDocument doc(1200);
    char myMqttMsg[400];
    doc["Cmd"] = thisCmd;
    olcbMsg thisMsg;
    if (gc_format_parse_olcb(&thisMsg, newData) >= 0)
    {ndler
      doc["MTI"] = thisMsg.MTI;
      doc["priority"] = thisMsg.priority;
      doc["frameType"] = thisMsg.frameType;
      doc["canFrameType"] = thisMsg.canFrameType;
      doc["srcAlias"] = thisMsg.srcAlias;
      doc["dstAlias"] = thisMsg.dstAlias;
      doc["dlc"] = thisMsg.dlc;
      JsonArray data = doc.createNestedArray("Data");
      for (byte i=0; i < thisMsg.dlc; i++)
        data.add(thisMsg.olcbData.u8[i]);
    }
    serializeJson(doc, myMqttMsg);
    globalClient->text(myMqttMsg);
    lastWifiUse = millis();
}

*/
