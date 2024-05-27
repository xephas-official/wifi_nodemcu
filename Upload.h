
/**
 * SYNTAX:
 *
 * String Firestore::Documents::createDocument(<AsyncClient>, <Firestore::Parent>, <documentPath>, <DocumentMask>, <Document>);
 *
 * String Firestore::Documents::createDocument(<AsyncClient>, <Firestore::Parent>, <collectionId>, <documentId>, <DocumentMask>, <Document>);
 *
 * <AsyncClient> - The async client.
 * <Firestore::Parent> - The Firestore::Parent object included project Id and database Id in its constructor.
 * <documentPath> - The relative path of document to create in the collection.
 * <DocumentMask> - The fields to return. If not set, returns all fields. Use comma (,) to separate between the field names.
 * <collectionId> - The document id of document to be created.
 * <documentId> - The relative path of document collection id to create the document.
 * <Document> - The Firestore document.
 *
 * The Firebase project Id should be only the name without the firebaseio.com.
 * The Firestore database id should be (default) or empty "".
 * 
 * The complete usage guidelines, please visit https://github.com/mobizt/FirebaseClient
 */

// DOCs: https://github.com/mobizt/FirebaseClient/blob/main/examples/FirestoreDatabase/Documents/Sync/CreateDocument/CreateDocument.ino


// wifi client
#include <WiFiClientSecure.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FirebaseClient.h>
#include "AppTimes.h"
#include <ctime>
#include <iomanip>
#include <sstream>

// Your WiFi credentials.
#define WIFI_SSID "xeph★s"
#define WIFI_PASSWORD "xephas096z"

//   SSID:	xeph★s
// Protocol:	Wi-Fi 4 (802.11n)
// Security type:	WPA2-Personal
// Manufacturer:	Intel Corporation
// Description:	Intel(R) Dual Band Wireless-AC 8265
// Driver version:	20.70.30.1
// Network band:	2.4 GHz
// Network channel:	9
// Link speed (Receive/Transmit):	72/72 (Mbps)
// Link-local IPv6 address:	fe80::6703:d220:755d:9d7b%21
// IPv4 address:	192.168.155.63
// IPv4 DNS servers:	192.168.155.22 (Unencrypted)
// Physical address (MAC):	28-C6-3F-60-BF-F3

// The API key can be obtained from Firebase console > Project Overview > Project settings.
#define API_KEY "AIzaSyAH-JWnXtIktk7dHVTOYXR8c6weZtYirps"

// User Email and password that already registerd or added in your project.
#define USER_EMAIL "test@wcmcs.com"
#define USER_PASSWORD "Test@WCMCS096"
// #define DATABASE_URL "URL"

// Define the project ID
#define FIREBASE_PROJECT_ID "mukfpl"

// define the functions
//  and variables
void authHandler();

void printResult(AsyncResult &aResult);
void printError(int code, const String &msg);

DefaultNetwork network; // initilize with boolean parameter to enable/disable network reconnection

UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD);

FirebaseApp app;

WiFiClientSecure ssl_client;

using AsyncClient = AsyncClientClass;

AsyncClient aClient(ssl_client, getNetwork(network));

Firestore::Documents Docs;

AsyncResult aResult_no_callback;

bool taskCompleted = false;

// sections db path
String sectionsCollection = "sections";

// current section e.g kitchen, bathroom, living room etc
String currentSection = "kitchen";

// a function to get todays date and format it as
// todays date in format dd-mm-yyyy eg. 27-05-2024
// String todaysDate = "27-06-2024";
String todaysDate = getTodayFormattedString();

int count = 0;

void initializeUploader()
{
  // * Wifi SetUp
  Serial.begin(9600);
  // Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // * Firebase SetUp

  Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);

  Serial.println("Initializing app...");

  ssl_client.setInsecure();
  ssl_client.setBufferSizes(4096, 1024);

  initializeApp(aClient, app, getAuth(user_auth), aResult_no_callback);

  authHandler();

  app.getApp<Firestore::Documents>(Docs);

  // In case setting the external async result to the sync task (optional)
  // To unset, use unsetAsyncResult().
  aClient.setAsyncResult(aResult_no_callback);
}

void uploadToCloud()
{
  authHandler();

  Docs.loop();

  if (app.ready() && !taskCompleted)
  {
    taskCompleted = true; // set this to false later on when we want to upload again

    // We will create the document in the parent path "sectionsCollection/currentSection"
    // "sectionsCollection" is the collection id, "currentSection" is the document id in collection.
    // documents are given random IDs
    String documentPath = sectionsCollection + "/" + currentSection + "/" + todaysDate;

    // If the document path contains space e.g. "a b c/d e f"
    // It should encode the space as %20 then the path will be "a%20b%20c/d%20e%20f"

    // double to hold the water flow rate
    Values::DoubleValue dblV(11000 / 1000.0f);

    // timestamp
    Values::TimestampValue tsV(getCurrentTimestampString());

    // document to upload
    Document<Values::Value> doc("amount", Values::Value(dblV));
    doc.add("timestamp", Values::Value(tsV));

    // taskCompleted = false;
    // The value of Values::xxxValue, Values::Value and Document can be printed on Serial.

    Serial.println("Create document... ");

    String payload = Docs.createDocument(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), documentPath, DocumentMask(), doc);

    if (aClient.lastError().code() == 0)
      Serial.println(payload);
    else
      printError(aClient.lastError().code(), aClient.lastError().message());
  }
}

void authHandler()
{
  // Blocking authentication handler with timeout
  unsigned long ms = millis();
  while (app.isInitialized() && !app.ready() && millis() - ms < 120 * 1000)
  {
    // This JWT token process required for ServiceAuth and CustomAuth authentications
    JWT.loop(app.getAuth());
    printResult(aResult_no_callback);
  }
}

void printResult(AsyncResult &aResult)
{
  if (aResult.isEvent())
  {
    Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.appEvent().message().c_str(), aResult.appEvent().code());
  }

  if (aResult.isDebug())
  {
    Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());
  }

  if (aResult.isError())
  {
    Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
  }

  if (aResult.available())
  {
    Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
  }
}

void printError(int code, const String &msg)
{
  Firebase.printf("Error, msg: %s, code: %d\n", msg.c_str(), code);
}
