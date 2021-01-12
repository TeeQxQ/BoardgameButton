#include "ArduinoJson.h"

/* This is the definition (as JSON) for the message.
 *  If this definition is changed later, the following tool
 *  can be used to calculate new value for msgCapacity
 *  https://arduinojson.org/v6/assistant/
 *  
 *  {
 *   "color": 0,
 *   "event": 2,
 *   "data": 100
 *  }
 *  
 */

namespace msg
{

const int msgCapacity = 96;
StaticJsonDocument<msgCapacity> msg;
DeserializationError err;

}
