LocoNet Button Events library for the ESP32 Arduino platform

This library provides a button handler for LocoNet. It receives button event messages as well as analog messages and
issues action commands, e.g. switch requests, signal aspect commands, etc.

The button numbers to be listened to as well as the actions for each event can be specified in a JSON file and be loaded at startup.

enum actionType : byte {blockdet=0, dccswitch=1, dccsignal=2, svbutton=3, analoginp=4, powerstat=5, unknown=255};
enum ctrlTypeType : byte {thrown=0, closed=1, toggle=2, nochange=3, input=4};
enum ctrlValueType : byte {offVal=0, onVal=1, idleVal=2};

{
"ButtonHandler":
[
{
	"ButtonNr": 11,
	"CtrlCmd": 
	[
	{"EventType":"onbtndown", "CmdList":[{"CtrlTarget": "switch", "CtrlAddr": 0, "CtrlType":"thrown", "CtrlValue":"on"}]},
	{"EventType":"onbtnup", "CmdList": []},
	{"EventType":"onbtnclick", "CmdList":[]},
	{"EventType":"onbtnhold", "CmdList":[]},
	{"EventType":"onbtndlclick", "CmdList":[]},
	{"EventType":"onanalog", "CmdList":[]}
	]
},
{
	"ButtonNr": 12,
	"CtrlCmd": 
	[
	{"EventType":"onbtndown", "CmdList":[{"CtrlTarget": "switch", "CtrlAddr": 0, "CtrlType":"closed", "CtrlValue":"on"}]},
	{"EventType":"onbtnup", "CmdList": []},
	{"EventType":"onbtnclick", "CmdList":[]},
	{"EventType":"onbtnhold", "CmdList":[]},
	{"EventType":"onbtndlclick", "CmdList":[]},
	{"EventType":"onanalog", "CmdList":[]}
	]
}
]}

To initialize (needs ArduinoJSON)
    DynamicJsonDocument doc(docSize);
    DeserializationError error = deserializeJson(doc, jsonData);
    if (!error)
      buttonHandler->loadButtonCfgJSON(doc);  


To define button handler in Arduino application:

IoTT_LocoNetButtonList * buttonHandler = new(IoTT_LocoNetButtonList); //dynamic

or

IoTT_LocoNetButtonList buttonHandler(); //static

Handling incoming events:

First, decode LocoNet messages for incoming button and analog events:
        case 0xE5: //OPC_PEER_XFER (button input)
        {
          //add code to interpret button and analog commands
          uint16_t btnAddr = (newData->lnData[8] & 0x3F) + ((newData->lnData[9] & 0x3F) << 6);
          uint8_t  inputType = newData->lnData[11];
          uint16_t inputValue = (newData->lnData[12] & 0x3F) + ((newData->lnData[13] & 0x3F) << 6);
          switch (inputType)
          {
            case 0: setAnalogValue(btnAddr, inputValue); //update buffer, if defined -> this is the preferred way for analog values as they are relatively static
                    handleAnalogValue(btnAddr, inputValue); //handle event
                    break;
            case 1: setButtonValue(btnAddr, inputValue & 0xFF); //update buffer, if defined
                    handleButtonValue(btnAddr, inputValue); //handle event -> this is the preferred way for button events as they are volatile
                    break;
          }
          break;
        }


When a new event is received from LocoNet, call the button handler:
  buttonHandler->processBtnEvent(btnAddr, (buttonEvent)inputValue);

For analog events, you typically will not call a handler, but simply update a status variable, from which the analiog value is polled by the application
