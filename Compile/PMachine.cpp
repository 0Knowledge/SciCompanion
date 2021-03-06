#include "stdafx.h"
#include "PMachine.h"

KernelInfo KrnlInfo[TOTAL_KERNELS] = {
	{"number","Abs","number value"},
	{"void","AddAfter","heapPtr pList, heapPtr afterNode, heapPtr pNode"},
	{"void","AddMenu","string caption, string itemCaptions"},
	{"void","AddToEnd","heapPtr pList, heapPtr pNode"},
	{"void","AddToFront","heapPtr pList, heapPtr pNode"},
	{"void","AddToPic","heapPtr viewList"},
	{"void","Animate","[heapPtr cast, bool cycle]"},
	{"void","BaseSetter","heapPtr viewObj"},
	{"bool","CanBeHere","heapPtr viewObj[, heapPtr pList]"},
	{"number","CelHigh","number view, number loop, number cel"},
	{"number","CelWide","number view, number loop, number cel"},
	{"bool","CheckFreeSpace","string directory"},
	{"bool","CheckSaveGame","string gameName, number slotNumber[, string gameVersion]"},
	{"heapPtr","Clone","heapPtr object"},
	{"number","CoordPri","number y"},
	{"number","CosDiv","number angle, number factor"},
	{"number","CosMult","number angle, number factor"},
	{"void","DeleteKey","heapPtr pList, number key"},
	{"heapPtr","DeviceInfo","diGET_DEVICE, string thePath, string strBuf"},
	{"void","DirLoop","heapPtr viewObj, number angle"},
	{"[heapPtr]","Display","string text[, params]"},
	{"void","DisposeClone","heapPtr object"},
	{"void","DisposeList","heapPtr pList"},
	{"void","DisposeScript","number scriptNum"},
	{"void","DisposeWindow","heapPtr Window"},
	{"void","DoAvoider","heapPtr moverObj"},
	{"bool","DoBresen","heapPtr moverObj"},
	{"void","DoSound","number subFunction, ..."},
	{"void","DrawCel","number view, number loop, number cel, point position, number priority"},
	{"void","DrawControl","heapPtr control"},
	{"void","DrawMenuBar","bool mode"},
	{"void","DrawPic","number picNum, [number animation, bool clear, number defaultPalette]"},
	{"void","DrawStatus","string text"},
	{"void","EditControl","heapPtr control, heapPtr event"},
	{"bool","EmptyList","heapPtr pList"},
	{"void","FClose","number handle"},
	{"number","FGets","string buffer, number max, number handle"},
	{"heapPtr","FindKey","heapPtr pList, number key"},
	{"heapPtr","FirstNode","heapPtr pList"},
	{"void","FlushResources",""},
	{"number","FOpen","string name, number mode"},
	{"string","Format","string destString, string fmtString[, parameters]"},
	{"void","FPuts","number handle, string buffer"},
	{"bool","GameIsRestarting",""},
	{"number","GetAngle","point origin, point dest"},
	{"string","GetCWD","string strBuf"},
	{"number","GetDistance","point origin, point dest"},
	{"bool","GetEvent","number types, heapPtr event"},
	{"string","GetFarText","number rsNumber, number index, string aString"},
	{"*","GetMenu","number menuItem, number subfunction, param"},
	{"void","GetPort","heapPtr newPort"},
	{"string","GetSaveDir",""},
	{"number","GetSaveFiles","string gameName, strings descBuffer, @number descSlots[]"},
	{"number","GetTime","[bool fTIME_OF_DAY]"},
	{"void","GlobalToLocal","heapPtr event"},
	{"*","Graph","number subFunction, ..."},
	{"bool","HaveMouse",""},
	{"void","HiliteControl","heapPtr control"},
	{"void","InitBresen","heapPtr moverObj[, number stepMult]"},
	{"void","InspectObj","heapPtr pObj"},
	{"bool","IsObject","heapPtr obj"},
	{"number","Joystick","number subfunction, number state"},
	{"heapPtr","LastNode","heapPtr pList"},
	{"heapPtr","Load","number type, number resNum"},
	{"void","LocalToGlobal","heapPtr event"},
	{"heapPtr","MapKeyToDir","heapPtr event"},
	{"number","MemoryInfo","number type"},
	{"number","MenuSelect","heapPtr event"},
	{"heapPtr","NewList",""},
	{"heapPtr","NewNode","number value, number key"},
	{"heapPtr","NewWindow","rect bounds, string title, number flags, number priority, number forecolour, number backcolour"},
	{"heapPtr","NextNode","heapPtr pNode"},
	{"number","NodeValue","heapPtr pNode"},
	{"number","NumCels","heapPtr obj"},
	{"number","NumLoops","heapPtr obj"},
	{"number","OnControl","number map, point xy or rect aRect "},
	{"bool","Parse","heapPtr event, string text"},
	{"number","PicNotValid","[number newFlag]"},
	{"heapPtr","PrevNode","heapPtr pNode"},
	{"void","Profiler",""},
	{"number","Random","number min, number max"},
	{"number","ReadNumber","string aString"},
	{"bool","RespondsTo","heapPtr anObject, selector aSelector"},
	{"*","RestartGame",""},
	{"void","RestoreGame","string gameName, number slotNumber, string gameVersion"},
	{"bool","Said","said saidString"},
	{"bool","SaveGame","string gameName, number slotNumber, string description, string gameVersion"},
	{"heapPtr","ScriptID","number scriptNum, number Index"},
	{"void","SetCursor","number resNumber, bool visible[,point coords]"},
	{"void","SetDebug",""},
	{"void","SetJump","heapPtr moverObj, number dx, number dy, number gy"},
	{"void","SetMenu","number menuItem, [number subfunction, params]"},
	{"void","SetNowSeen","heapPtr viewObj"},
	{"void","SetPort","heapPtr newPort"},
	{"void","SetSynonyms","heapPtr scrListObj"},
	{"void","ShakeScreen","number shakeCount, [number direction]"},
	{"void","Show","[number screen]"},
	{"void","ShowFree",""},
	{"void","ShowObjs",""},
	{"void","ShowSends",""},
	{"number","SinDiv","number angle, number factor"},
	{"number","SinMult","number angle, number factor"},
	{"number","Sqrt","number value"},
	{"number","StackUsage",""},
	{"char","StrAt","string aString, number index[, char replacement]"},
	{"string","StrCat","string destString, string srcString"},
	{"number","StrCmp","string aString, string bString[, number max]"},
	{"string","StrCpy","string destString, string srcString[, number max]"},
	{"string","StrEnd","string aString"},
	{"number","StrLen","string aString"},
	{"void","TextSize","@rect sizeRect, string text, number font[, number maxWidth]"},
	{"void","UnLoad","number type, number resNum"},
	{"bool","ValidPath","string aDirectory"},
	{"number","Wait","number ticks"}
};


BYTE OpArgs[256] = {
	/*bnot*/     0,0,
	/*add*/      0,0,
	/*sub*/      0,0,
	/*mul*/      0,0,
	/*div*/      0,0,
	/*mod*/      0,0,
	/*shr*/      0,0,
	/*shl*/      0,0,
	/*xor*/      0,0,
	/*and*/      0,0,
/*10*/
	/*or*/       0,0,
	/*neg*/      0,0,
	/*not*/      0,0,
	/*eq?*/      0,0,
	/*ne?*/      0,0,
	/*gt?*/      0,0,
	/*ge?*/      0,0,
	/*lt?*/      0,0,
	/*le?*/      0,0,
	/*ugt?*/     0,0,
/*20*/
	/*uge?*/     0,0,
	/*ult?*/     0,0,
	/*ule?*/     0,0,
	/*bt*/       2,1,
	/*bnt*/      2,1,
	/*jmp*/      2,1,
	/*ldi*/      2,1,
	/*push*/     0,0,
	/*pushi*/    2,1,
	/*toss*/     0,0,
/*30*/
	/*dup*/      0,0,
	/*link*/     2,1,
	/*call*/     3,2,
	/*callk*/    3,2,
	/*callb*/    3,2,
	/*calle*/    4,3,
	/*ret*/      0,0,
	/*send*/     1,1,
	/**/         0,0,
	/**/         0,0,
/*40*/
	/*class*/    2,1,
	/**/         0,0,
	/*self*/     0,0,
	/*super*/    3,2,
	/*rest*/    2,1,
	/*lea*/      2,1,
	/*selfID*/   0,0,
	/**/         0,0,
	/*pprev*/    0,0,
	/*pToa*/     2,1,
/*50*/
	/*aTop*/     2,1,
	/*pTos*/     2,1,
	/*sTop*/     2,1,
	/*ipToa*/    2,1,
	/*dpToa*/    2,1,
	/*ipTos*/    2,1,
	/*dpTos*/    2,1,
	/*lofsa*/    2,1,
	/*lofss*/    2,1,
	/*push0*/    0,0,
/*60*/
	/*push1*/    0,0,
	/*push2*/    0,0,
	/*pushSelf*/ 0,0,
	/**/         0,0,
	/*lag*/      2,1,
	/*lal*/      2,1,
	/*lat*/      2,1,
	/*lap*/      2,1,
	/*lsg*/      2,1,
	/*lsl*/      2,1,
/*70*/
	/*lst*/      2,1,
	/*lsp*/      2,1,
	/*lagi*/     2,1,
	/*lali*/     2,1,
	/*lati*/     2,1,
	/*lapi*/     2,1,
	/*lsgi*/     2,1,
	/*lsli*/     2,1,
	/*lsti*/     2,1,
	/*lspi*/     2,1,
/*80*/
	/*sag*/      2,1,
	/*sal*/      2,1,
	/*sat*/      2,1,
	/*sap*/      2,1,
	/*ssg*/      2,1,
	/*ssl*/      2,1,
	/*sst*/      2,1,
	/*ssp*/      2,1,
	/*sagi*/     2,1,
	/*sali*/     2,1,
/*90*/
	/*sati*/     2,1,
	/*sapi*/     2,1,
	/*ssgi*/     2,1,
	/*ssli*/     2,1,
	/*ssti*/     2,1,
	/*sspi*/     2,1,
	/*+ag*/      2,1,
	/*+al*/      2,1,
	/*+at*/      2,1,
	/*+ap*/      2,1,
/*100*/
	/*+sg*/      2,1,
	/*+sl*/      2,1,
	/*+st*/      2,1,
	/*+sp*/      2,1,
	/*+agi*/     2,1,
	/*+ali*/     2,1,
	/*+ati*/     2,1,
	/*+api*/     2,1,
	/*+sgi*/     2,1,
	/*+sli*/     2,1,
/*110*/
	/*+sti*/     2,1,
	/*+spi*/     2,1,
	/*-ag*/      2,1,
	/*-al*/      2,1,
	/*-at*/      2,1,
	/*-ap*/      2,1,
	/*-sg*/      2,1,
	/*-sl*/      2,1,
	/*-st*/      2,1,
	/*-sp*/      2,1,
/*120*/
	/*-agi*/     2,1,
	/*-ali*/     2,1,
	/*-ati*/     2,1,
	/*-api*/     2,1,
	/*-sgi*/     2,1,
	/*-sli*/     2,1,
	/*-sti*/     2,1,
	/*-spi*/     2,1
};

/******************************************************************************/
OperandType OpArgTypes[TOTAL_OPCODES][3] = {
	/*bnot*/     {otEMPTY,otEMPTY,otEMPTY},
	/*add*/      {otEMPTY,otEMPTY,otEMPTY},
	/*sub*/      {otEMPTY,otEMPTY,otEMPTY},
	/*mul*/      {otEMPTY,otEMPTY,otEMPTY},
	/*div*/      {otEMPTY,otEMPTY,otEMPTY},
	/*mod*/      {otEMPTY,otEMPTY,otEMPTY},
	/*shr*/      {otEMPTY,otEMPTY,otEMPTY},
	/*shl*/      {otEMPTY,otEMPTY,otEMPTY},
	/*xor*/      {otEMPTY,otEMPTY,otEMPTY},
	/*and*/      {otEMPTY,otEMPTY,otEMPTY},
/*10*/
	/*or*/       {otEMPTY,otEMPTY,otEMPTY},
	/*neg*/      {otEMPTY,otEMPTY,otEMPTY},
	/*not*/      {otEMPTY,otEMPTY,otEMPTY},
	/*eq?*/      {otEMPTY,otEMPTY,otEMPTY},
	/*ne?*/      {otEMPTY,otEMPTY,otEMPTY},
	/*gt?*/      {otEMPTY,otEMPTY,otEMPTY},
	/*ge?*/      {otEMPTY,otEMPTY,otEMPTY},
	/*lt?*/      {otEMPTY,otEMPTY,otEMPTY},
	/*le?*/      {otEMPTY,otEMPTY,otEMPTY},
	/*ugt?*/     {otEMPTY,otEMPTY,otEMPTY},
/*20*/
	/*uge?*/     {otEMPTY,otEMPTY,otEMPTY},
	/*ult?*/     {otEMPTY,otEMPTY,otEMPTY},
	/*ule?*/     {otEMPTY,otEMPTY,otEMPTY},
	/*bt*/       {otLABEL,otEMPTY,otEMPTY},
	/*bnt*/      {otLABEL,otEMPTY,otEMPTY},
	/*jmp*/      {otLABEL,otEMPTY,otEMPTY},
	/*ldi*/      {otINT,otEMPTY,otEMPTY},
	/*push*/     {otEMPTY,otEMPTY,otEMPTY},
	/*pushi*/    {otINT,otEMPTY,otEMPTY},
	/*toss*/     {otEMPTY,otEMPTY,otEMPTY},
/*30*/
	/*dup*/      {otEMPTY,otEMPTY,otEMPTY},
	/*link*/     {otINT,otEMPTY,otEMPTY},
    /*call*/     {otLABEL,otINT8,otEMPTY},
	/*callk*/    {otKERNEL,otINT8,otEMPTY},
	/*callb*/    {otPUBPROC,otINT8,otEMPTY},
	/*calle*/    {otINT,otPUBPROC,otINT8},
	/*ret*/      {otEMPTY,otEMPTY,otEMPTY},
	/*send*/     {otINT8,otEMPTY,otEMPTY},
	/**/         {otEMPTY,otEMPTY,otEMPTY},
	/**/         {otEMPTY,otEMPTY,otEMPTY},
/*40*/
	/*class*/    {otCLASS,otEMPTY,otEMPTY},
	/**/         {otEMPTY,otEMPTY,otEMPTY},
	/*self*/     {otINT8,otEMPTY,otEMPTY},
	/*super*/    {otCLASS,otINT8,otEMPTY},
	/*rest*/    {otPVAR,otEMPTY,otEMPTY},
	/*lea*/      {otINT,otINT,otEMPTY}, // REVIEW: is this correct?
	/*selfID*/   {otEMPTY,otEMPTY,otEMPTY},
	/**/         {otEMPTY,otEMPTY,otEMPTY},
	/*pprev*/    {otEMPTY,otEMPTY,otEMPTY},
	/*pToa*/     {otPROP,otEMPTY,otEMPTY},
/*50*/
	/*aTop*/     {otPROP,otEMPTY,otEMPTY},
	/*pTos*/     {otPROP,otEMPTY,otEMPTY},
	/*sTop*/     {otPROP,otEMPTY,otEMPTY},
	/*ipToa*/    {otPROP,otEMPTY,otEMPTY},
	/*dpToa*/    {otPROP,otEMPTY,otEMPTY},
	/*ipTos*/    {otPROP,otEMPTY,otEMPTY},
	/*dpTos*/    {otPROP,otEMPTY,otEMPTY},
	/*lofsa*/    {otOFFS,otEMPTY,otEMPTY},
	/*lofss*/    {otOFFS,otEMPTY,otEMPTY},
	/*push0*/    {otEMPTY,otEMPTY,otEMPTY},
/*60*/
	/*push1*/    {otEMPTY,otEMPTY,otEMPTY},
	/*push2*/    {otEMPTY,otEMPTY,otEMPTY},
	/*pushSelf*/ {otEMPTY,otEMPTY,otEMPTY},
	/**/         {otEMPTY,otEMPTY,otEMPTY},
	/*lag*/      {otVAR,otEMPTY,otEMPTY},
	/*lal*/      {otVAR,otEMPTY,otEMPTY},
	/*lat*/      {otVAR,otEMPTY,otEMPTY},
	/*lap*/      {otVAR,otEMPTY,otEMPTY},
	/*lsg*/      {otVAR,otEMPTY,otEMPTY},
	/*lsl*/      {otVAR,otEMPTY,otEMPTY},
/*70*/
	/*lst*/      {otVAR,otEMPTY,otEMPTY},
	/*lsp*/      {otVAR,otEMPTY,otEMPTY},
	/*lagi*/     {otVAR,otEMPTY,otEMPTY},
	/*lali*/     {otVAR,otEMPTY,otEMPTY},
	/*lati*/     {otVAR,otEMPTY,otEMPTY},
	/*lapi*/     {otVAR,otEMPTY,otEMPTY},
	/*lsgi*/     {otVAR,otEMPTY,otEMPTY},
	/*lsli*/     {otVAR,otEMPTY,otEMPTY},
	/*lsti*/     {otVAR,otEMPTY,otEMPTY},
	/*lspi*/     {otVAR,otEMPTY,otEMPTY},
/*80*/
	/*sag*/      {otVAR,otEMPTY,otEMPTY},
	/*sal*/      {otVAR,otEMPTY,otEMPTY},
	/*sat*/      {otVAR,otEMPTY,otEMPTY},
	/*sap*/      {otVAR,otEMPTY,otEMPTY},
	/*ssg*/      {otVAR,otEMPTY,otEMPTY},
	/*ssl*/      {otVAR,otEMPTY,otEMPTY},
	/*sst*/      {otVAR,otEMPTY,otEMPTY},
	/*ssp*/      {otVAR,otEMPTY,otEMPTY},
	/*sagi*/     {otVAR,otEMPTY,otEMPTY},
	/*sali*/     {otVAR,otEMPTY,otEMPTY},
/*90*/
	/*sati*/     {otVAR,otEMPTY,otEMPTY},
	/*sapi*/     {otVAR,otEMPTY,otEMPTY},
	/*ssgi*/     {otVAR,otEMPTY,otEMPTY},
	/*ssli*/     {otVAR,otEMPTY,otEMPTY},
	/*ssti*/     {otVAR,otEMPTY,otEMPTY},
	/*sspi*/     {otVAR,otEMPTY,otEMPTY},
	/*+ag*/      {otVAR,otEMPTY,otEMPTY},
	/*+al*/      {otVAR,otEMPTY,otEMPTY},
	/*+at*/      {otVAR,otEMPTY,otEMPTY},
	/*+ap*/      {otVAR,otEMPTY,otEMPTY},
/*100*/
	/*+sg*/      {otVAR,otEMPTY,otEMPTY},
	/*+sl*/      {otVAR,otEMPTY,otEMPTY},
	/*+st*/      {otVAR,otEMPTY,otEMPTY},
	/*+sp*/      {otVAR,otEMPTY,otEMPTY},
	/*+agi*/     {otVAR,otEMPTY,otEMPTY},
	/*+ali*/     {otVAR,otEMPTY,otEMPTY},
	/*+ati*/     {otVAR,otEMPTY,otEMPTY},
	/*+api*/     {otVAR,otEMPTY,otEMPTY},
	/*+sgi*/     {otVAR,otEMPTY,otEMPTY},
	/*+sli*/     {otVAR,otEMPTY,otEMPTY},
/*110*/
	/*+sti*/     {otVAR,otEMPTY,otEMPTY},
	/*+spi*/     {otVAR,otEMPTY,otEMPTY},
	/*-ag*/      {otVAR,otEMPTY,otEMPTY},
	/*-al*/      {otVAR,otEMPTY,otEMPTY},
	/*-at*/      {otVAR,otEMPTY,otEMPTY},
	/*-ap*/      {otVAR,otEMPTY,otEMPTY},
	/*-sg*/      {otVAR,otEMPTY,otEMPTY},
	/*-sl*/      {otVAR,otEMPTY,otEMPTY},
	/*-st*/      {otVAR,otEMPTY,otEMPTY},
	/*-sp*/      {otVAR,otEMPTY,otEMPTY},
/*120*/
	/*-agi*/     {otVAR,otEMPTY,otEMPTY},
	/*-ali*/     {otVAR,otEMPTY,otEMPTY},
	/*-ati*/     {otVAR,otEMPTY,otEMPTY},
	/*-api*/     {otVAR,otEMPTY,otEMPTY},
	/*-sgi*/     {otVAR,otEMPTY,otEMPTY},
	/*-sli*/     {otVAR,otEMPTY,otEMPTY},
	/*-sti*/     {otVAR,otEMPTY,otEMPTY},
	/*-spi*/     {otVAR,otEMPTY,otEMPTY},
};


char *OpcodeNames[]={
	"bnot",
	"add",
	"sub",
	"mul",
	"div",
	"mod",
	"shr",
	"shl",
	"xor",
	"and",
	"or",
	"neg",
	"not",
	"eq?",
	"ne?",
	"gt?",
	"ge?",
	"lt?",
	"le?",
	"ugt?",
	"uge?",
	"ult?",
	"ule?",
	"bt",
	"bnt",
	"jmp",
	"ldi",
	"push",
	"pushi",
	"toss",
	"dup",
	"link",
	"call",
	"callk",
	"callb",
	"calle",
	"ret",
	"send",
    "INVALID",
    "INVALID",
	"class",
    "INVALID",
	"self",
	"super",
	"&rest",
	"lea",
	"selfID",  
    "INVALID",
	"pprev",
	"pToa",
	"aTop",
	"pTos",
	"sTop",
	"ipToa",
	"dpToa",
	"ipTos",
	"dpTos",
	"lofsa",
	"lofss",
	"push0",
	"push1",
	"push2",
	"pushSelf",  
    "INVALID",
	"lag",
	"lal",
	"lat",
	"lap",
	"lsg",
	"lsl",
	"lst",
	"lsp",
	"lagi",
	"lali",
	"lati",
	"lapi",
	"lsgi",
	"lsli",
	"lsti",
	"lspi",
	"sag",
	"sal",
	"sat",
	"sap",
	"ssg",
	"ssl",
	"sst",
	"ssp",
	"sagi",
	"sali",
	"sati",
	"sapi",
	"ssgi",
	"ssli",
	"ssti",
	"sspi",
	"+ag",
	"+al",
	"+at",
	"+ap",
/*100*/
	"+sg",
	"+sl",
	"+st",
	"+sp",
	"+agi",
	"+ali",
	"+ati",
	"+api",
	"+sgi",
	"+sli",
/*110*/
	"+sti",
	"+spi",
	"-ag",
	"-al",
	"-at",
	"-ap",
	"-sg",
	"-sl",
	"-st",
	"-sp",
/*120*/
	"-agi",
	"-ali",
	"-ati",
	"-api",
	"-sgi",
	"-sli",
	"-sti",
	"-spi"
};


SaidToken SaidArgs[] = {
    {',', 0xF0}, /*"OR". Used to specify alternatives to words, such as "take , get".*/
    {'&', 0xF1}, /*Unknown. Probably used for debugging.*/
    {'/', 0xF2}, /*Sentence part separator. Only two of these tokens may be used, since sentences are split into a maximum of three parts.*/
    {'(', 0xF3}, /*Used together with ')' for grouping*/
    {')', 0xF4}, /*See '('*/
    {'[', 0xF5}, /*Used together with '[' for optional grouping. "[ a ]" means "either a or nothing" */
    {']', 0xF6}, /*See '['.*/
    {'#', 0xF7}, /*Unknown. Assumed to have been used exclusively for debugging, if at all.*/
    {'<', 0xF8}, /*Semantic reference operator (as in "get < up").*/
    {'>', 0xF9}  /*Instructs Said() not to claim the event passed to the previous Parse() call on a match. Used for successive matching.*/
};