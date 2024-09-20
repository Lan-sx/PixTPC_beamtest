/*********************************************************************
 * Author           : Lan-sx
 * Email            : shexin@ihep.ac.cn
 * Last modified    : 2024-07-05 21:02
 * Filename         : LinkDef.h
 * Description      : LinkDef head file, Data structure  
 *                    for ROOT I/O, generate dict
 * Update           : 2024-07-05 21:02: created and add PixelTPCdata;
 *                    2024-09-20 15:32: add MCTrackdata;
 * ******************************************************************/
#include "PixelTPCdata.h"
#include "MCTrackdata.h"
#ifdef __CLING__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ nestedclasses;
#pragma link C++ class PixelTPCdata+;
#pragma link C++ class MCTrackdata+;

#endif
