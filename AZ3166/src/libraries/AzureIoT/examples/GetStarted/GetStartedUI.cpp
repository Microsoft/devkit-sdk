// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
#include "Arduino.h"
#include "GetStartedUI.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Application title
static const unsigned char appIcon [] = 
{
    0x00,0x00,0x38,0xf0,0xe0,0xe0,0xc0,0xc0,0xf0,0xfc,0xfa,0xfa,0xd8,0xf0,0xe0,0x80,
    0x40,0x00,
    0x00,0x04,0x08,0x18,0x19,0x3b,0x3f,0x3f,0x3f,0x3f,0x3f,0x1f,0x1f,0x0f,0x07,0x01,
    0x01,0x00,
};

void DrawAppTitle(char* text)
{
    char sz[32];
    snprintf(sz, 32, "   %s", text);
    Screen.print(0, sz);
    Screen.draw(0, 0, 18, 2, (unsigned char*)appIcon);
}