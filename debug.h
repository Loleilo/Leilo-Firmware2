#ifndef _DEBUG_H
#define _DEBUG_H
#define DEBUG
#ifdef DEBUG
#define dp(str) Serial.println(str)
#define dpnl(str) Serial.print(str)
#else
#define dp(str) 
#define dpnl(str) 
#endif
#endif
