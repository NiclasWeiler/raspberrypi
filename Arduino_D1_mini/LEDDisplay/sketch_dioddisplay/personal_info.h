const int nrOfWiFi = 2; // maximum 8 WiFi networks
const int nrOfMqtt = 1;

typedef struct
{
  char ssid[20];
  char password[20];
} personalWiFi; 


typedef struct
{
  char     serverAdress[30];
  int      port;
  char     user[10];
  char     password[20];
} mqttServer;

personalWiFi wiFiList[nrOfWiFi] = 
{
	{"Network1", "password1"},          // WiFi 1
	{"Network2", "password2"},          // WiFi 2
//  {"Network3", "password3"},          // WiFi 3
//	{"Network4", "password4"},          // WiFi 4
//	{"Network5", "password5"},          // WiFi 5
//	{"Network6", "password6"},          // WiFi 6
//	{"Network7", "password7"},          // WiFi 7
//	{"Network8", "password8"},          // WiFi 8
};

mqttServer mqttServerList[nrOfMqtt] =
{
	{"serverAdress1", port1, "user1", "password1"},
//	{"serverAdress2", port2, "user2", "password2"},
};


