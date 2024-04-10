#include "URLParser.h"

void setup() {

    Serial.begin(9600);

    while(!Serial);

    Serial.println("starting");

    ParsedUrl url(
        "https://www.google.com/search?q=arduino"
    );

    Serial.print("parsed URL schema: \"");
    Serial.print(url.schema());
    Serial.print("\"\nparsed URL host: \"");
    Serial.print(url.host());
    Serial.print("\"\nparsed URL path: \"");
    Serial.print(url.path());
    Serial.print("\"\nparsed URL query: \"");
    Serial.print(url.query());
    Serial.print("\"\nparsed URL userinfo: \"");
    Serial.print(url.userinfo());
    Serial.println("\"");

}

void loop() { }