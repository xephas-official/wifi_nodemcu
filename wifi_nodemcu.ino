// wifi client
#include <Arduino.h>
#include "Upload.h"

// empty arduino project
void setup(){
 initializeUploader();
}

void loop(){
  uploadToCloud();
}
