//establish the MQTT connection to the broker
void   establishMQTTConnection()
{
  if (lnMQTT)
    lnMQTT->connectToBroker();
}
