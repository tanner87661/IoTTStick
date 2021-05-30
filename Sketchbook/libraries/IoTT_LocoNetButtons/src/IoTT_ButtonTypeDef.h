#ifndef IoTT_ButtonTypeDef_h
#define IoTT_ButtonTypeDef_h

//used by greenhat, eventhandler, ledchain
enum sourceType : uint8_t {evt_button=0, evt_analogvalue=1, evt_trackswitch=2, evt_signalmastdcc=3, evt_signalmastdyn=4, evt_blockdetector=5, evt_transponder=6, evt_powerstat=7, evt_alwayson=8, evt_analogscaler=9, evt_randomgen=10, evt_nosource=255};
enum enableType : uint8_t {ent_alwayson = 0, ent_alwaysoff=1, ent_button=2, ent_switch=3, ent_block=4};

//enum buttonType : uint8_t {btnoff=0, digitalAct=2, analog=9};
//enum eventSource : uint8_t {buttonevt=0, analogevt=1, switchevt=2, signalevt=3, blockdetevt=4, transponderevt=5, nosource=255};
enum buttonType : uint8_t {btnoff=0, autodetect=1, digitalAct=2, touch=3, sensor=4, swireport=5, swireq=6, swiack=7, analog=9}; //analog=64
enum buttonEvent : byte {onbtndown=0, onbtnup=1, onbtnclick=2, onbtndblclick=4, onbtnhold=3, noevent=255};
enum buttonCommType : byte {btnI2C=0, btnSerComm=2};

enum outputType : byte {blockdet=0, dccswitchreq=1, dccswitchrep=9, dccswitchack=10, dccsignaldyn=2, dccsignalstat=3, dccsignalnmra=4, svbutton=5, analoginp=6, powerstat=7, constantled=8, unknown=255};
enum ctrlTypeType : byte {thrown=0, closed=1, toggle=2, nochange=3, input=4};
enum ctrlValueType : byte {offVal=0, onVal=1, idleVal=2, toggleVal=3};

//sourceType getSourceTypeByName(String transName);
//actionType getActionTypeByName(String actionName);

/*
sourceType getSourceTypeByName(String transName)
{
  if (transName == "button") return button;
  if (transName == "analogvalue") return analogvalue;
  if (transName == "switch") return trackswitch;
  if (transName == "dccsignal") return signalmastdcc;
  if (transName == "dynsignal") return signalmastdyn;
  if (transName == "blockdetector") return blockdetector;
  if (transName == "transponder") return transponder;
  if (transName == "power") return transponder;
  return trackswitch;
};

actionType getActionTypeByName(String actionName)
{ //blockdet, dccswitch, dccsignal, svbutton, analoginp, powerstat
  if (actionName == "block") return blockdet; 
  if (actionName == "switch") return dccswitchreq;
  if (actionName == "switchack") return dccswitchack;
  if (actionName == "switchrep") return dccswitchrep;
  if (actionName == "signal") return dccsignalnmra;
  if (actionName == "button") return svbutton;
  if (actionName == "analog") return analoginp;
  if (actionName == "power") return powerstat;
  return unknown; 
}
*/


#endif
