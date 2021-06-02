
#include <ESP8266WiFi.h>
#include "ESPAsyncTCP.h"
#include "AudioPlayer.h"

#include "ESPAsyncWebServer.h"

#include <Servo.h>

/* Put your SSID & Password */
const char *ssid = "TP-LINK-AC8266";
const char *password = "password";

/* Put IP Address details */
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

#define SOUND_BOOT 1
#define SOUND_COME_PLAY 2
#define SOUND_GIGGLE_1 3
#define SOUND_GIGGLE_2 4
#define SOUND_I_AM_WATCHING 5
#define SOUND_RING_AROUND_LALALA 6
#define SOUND_RAINING_POURING 7
#define SOUND_RING_AROUND 8
#define SOUND_CANT_FIND_FRIENDS 9
#define SOUND_MAN_AT_THE_STORE 10
#define SOUND_NEXT_TO_YOUR_BED 11
#define SOUND_YOU_DROPPED_ME_AND_I_BROKE 12

// HTML web page
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>

<head>
	<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
	<title>LED Control</title>
	<style>
	html {
		font-family: Helvetica;
		display: inline-block;
		margin: 0px auto;
		text-align: center;
	}
	
	body {
		margin-top: 50px;
	}
	
	h1 {
		color: #444444;
		margin: 50px auto 30px;
	}
	
	.button {
		display: block;
		/*width: 80px;*/
		border: none;
		color: white;
		padding: 13px 30px;
		text-decoration: none;
		font-size: 25px;
		margin: 0px auto 35px;
		cursor: pointer;
		border-radius: 4px;
	}
	
	.button-sfx:nth-child(odd) {
		background-color: #1abc9c;
	}
	
	.button-sfx:nth-child(even) {
		background-color: #138770;
	}
	
	.button-servo:nth-child(odd) {
		background-color: #34495e;
	}
	
	.button-servo:nth-child(even) {
		background-color: #1d2a36;
	}
	
	p {
		font-size: 14px;
		color: #888;
		margin-bottom: 10px;
	}

  .slider {
    -webkit-appearance: none;
    width: 100%;
    height: 15px;
    margin: 0px auto 35px;
    border-radius: 5px;  
    background: #d3d3d3;
    outline: none;
    opacity: 0.7;
    -webkit-transition: .2s;
    transition: opacity .2s;
}

.slider::-webkit-slider-thumb {
  -webkit-appearance: none;
  appearance: none;
  width: 25px;
  height: 25px;
  border-radius: 50%; 
  background: #34495e;
  cursor: pointer;
}

.slider::-moz-range-thumb {
  width: 25px;
  height: 25px;
  border-radius: 50%;
  background: #34495e;
  cursor: pointer;
}
	</style>
</head>

<body>
	<h1>Doll Web Server</h1>
  <a class="button button-sfx" href="javascript:send('sound/2');">SFX: Come Play</a>
  <a class="button button-sfx" href="javascript:send('sound/3');">SFX: Giggle 1</a>
  <a class="button button-sfx" href="javascript:send('sound/4');">SFX: Giggle 2</a>
  <a class="button button-sfx" href="javascript:send('sound/5');">SFX: I'm watching you</a>
  <a class="button button-sfx" href="javascript:send('sound/6');">SFX: Ring around la la la</a>
  <a class="button button-sfx" href="javascript:send('sound/7');">SFX: Raining pouring</a>
  <a class="button button-sfx" href="javascript:send('sound/8');">SFX: Ring around</a>
  <a class="button button-sfx" href="javascript:send('sound/9');">SFX: Can't find friends</a>
  <a class="button button-sfx" href="javascript:send('sound/10');">SFX: Man at the store</a>
  <a class="button button-sfx" href="javascript:send('sound/11');">SFX: Next to your bed</a>
  <a class="button button-sfx" href="javascript:send('sound/12');">SFX: Dropped me and I broke</a>
  <input type="range" min="0" max="180" value="90" class="slider" id="slider" onchange="setPos(this.value)">
  <a class="button button-servo" href="javascript:setPos2(0);">Servo: 0</a>
  <a class="button button-servo" href="javascript:setPos2(90);">Servo: 90</a>
  <a class="button button-servo" href="javascript:setPos2(180);">Servo: 180</a>
   <script>
   function send(x) {
     var xhr = new XMLHttpRequest();
     xhr.open("GET", "/" + x, true);
     xhr.send();
   }
   function setPos2(value) {
     document.getElementById("slider").value = value;
     setPos(value);
   }
   function setPos(value) {
      send("servo/set?pos=" + value);
   }
  </script>
  </body>
</html>)rawliteral";

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

AsyncWebServer server(80);

Servo servo;
int pos = 0;

bool shouldPlay = false;

void setup()
{
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);

  setupAudio(D3, D7);
  delay(300);

  servo.attach(D6); // attaches the servo
  servo.write(90);

  // Send web page to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", index_html); });

  server.on("/sound/2", HTTP_GET, [](AsyncWebServerRequest *request)
            { play(request, SOUND_COME_PLAY); });

  server.on("/sound/3", HTTP_GET, [](AsyncWebServerRequest *request)
            { play(request, SOUND_GIGGLE_1); });

  server.on("/sound/4", HTTP_GET, [](AsyncWebServerRequest *request)
            { play(request, SOUND_GIGGLE_2); });

  server.on("/sound/5", HTTP_GET, [](AsyncWebServerRequest *request)
            { play(request, SOUND_I_AM_WATCHING); });

  server.on("/sound/6", HTTP_GET, [](AsyncWebServerRequest *request)
            { play(request, SOUND_RING_AROUND_LALALA); });

  server.on("/sound/7", HTTP_GET, [](AsyncWebServerRequest *request)
            { play(request, SOUND_RAINING_POURING); });

  server.on("/sound/8", HTTP_GET, [](AsyncWebServerRequest *request)
            { play(request, SOUND_RING_AROUND); });

  server.on("/sound/9", HTTP_GET, [](AsyncWebServerRequest *request)
            { play(request, SOUND_CANT_FIND_FRIENDS); });

  server.on("/sound/10", HTTP_GET, [](AsyncWebServerRequest *request)
            { play(request, SOUND_MAN_AT_THE_STORE); });

  server.on("/sound/11", HTTP_GET, [](AsyncWebServerRequest *request)
            { play(request, SOUND_NEXT_TO_YOUR_BED); });

  server.on("/sound/12", HTTP_GET, [](AsyncWebServerRequest *request)
            { play(request, SOUND_YOU_DROPPED_ME_AND_I_BROKE); });

  server.on("/servo/set", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              AsyncWebParameter *p = request->getParam("pos");
              int pos = p->value().toInt();
              servo.write(pos);
            });

  server.onNotFound(notFound);
  server.begin();

  playAudio(SOUND_BOOT);
}

void play(AsyncWebServerRequest *request, int audio)
{
  request->send(200, "text/plain", "playing");
  playAudio(audio);
}

void loop()
{
}
