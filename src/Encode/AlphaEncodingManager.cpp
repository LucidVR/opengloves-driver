#include <Encode/AlphaEncodingManager.h>

#include <sstream>
#include <vector>
#include "DriverLog.h"


/* Alpha encoding uses the wasted data in the delimiter from legacy to allow for optional arguments and redundancy over smaller packets
*Alpha Encoding Manager Arguments:
* A - Pinky Finger Position
* B - Ring Finger Position
* C - Middle Finger Position
* D - Index Finger Position
* E - Thumb Finger Position
* F - Joystick X
* G - Joystick Y
* H - Joystick click
* I - Trigger button
* J - A button
* K - B button
* L - Grab button
* M - Pinch button
* - Calibration Reset button
* 
*/

std::string getArgumentSubstring(std::string str, char del) { 
    int start = str.find(del);
    if (start == std::string::npos)
        return NULL;
    int end = str.find_first_of(alphabet, start + 1); //characters may not necessarily be in order, so end at any letter
    return str.substr(start, end - start);
}

bool argValid(std::string str, char del) { return str.find(del) == std::string::npos; }

VRCommData_t AlphaEncodingManager::Decode(std::string input) {

    std::array<float, 5> flexion;
    std::array<float, 5> splay;

    if (argValid(input, 'A'))
      flexion[0] = stof(getArgumentSubstring(input, 'A')) / m_maxAnalogValue;
    if (argValid(input, 'B'))
      flexion[1] = stof(getArgumentSubstring(input, 'B')) / m_maxAnalogValue;
    if (argValid(input, 'C'))
      flexion[2] = stof(getArgumentSubstring(input, 'C')) / m_maxAnalogValue;
    if (argValid(input, 'D'))
      flexion[3] = stof(getArgumentSubstring(input, 'D')) / m_maxAnalogValue;
    if (argValid(input, 'E'))
      flexion[4] = stof(getArgumentSubstring(input, 'E')) / m_maxAnalogValue;

    float joyX;
    float joyY;

    if (argValid(input, 'F'))
      joyX = stof(getArgumentSubstring(input, 'E')) / m_maxAnalogValue;
    if (argValid(input, 'G'))
      joyY = stof(getArgumentSubstring(input, 'G')) / m_maxAnalogValue;

    VRCommData_t commData(
        flexion,
        splay,
        joyX,
        joyY,
        argValid(input, 'H'), //joystick click
        argValid(input, 'I'), //trigger
        argValid(input, 'J'), //A button
        argValid(input, 'K'), //B button
        argValid(input, 'L'), //grab
        argValid(input, 'M') //pinch
    );

    return commData;
}