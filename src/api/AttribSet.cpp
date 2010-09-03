// AttribSet.cpp written by Gabriel Landau (night100@hotmail.com)

#include "AttribSet.h"

AttribSet AttribSet::STANDARD_P(AS_STD_P);     // Position only
AttribSet AttribSet::STANDARD_PN(AS_STD_PN);    // Position and Normal
AttribSet AttribSet::STANDARD_PT(AS_STD_PT);    // Position and TexCoord
AttribSet AttribSet::STANDARD_PC(AS_STD_PC);    // Position and Color
AttribSet AttribSet::STANDARD_PCN(AS_STD_PCN);   // Position, Color, and Normal
AttribSet AttribSet::STANDARD_PCT(AS_STD_PCT);   // Position, Color, and TexCoord
AttribSet AttribSet::STANDARD_PNT(AS_STD_PNT);   // Position, Normal, TexCoord
AttribSet AttribSet::STANDARD_PCNT(AS_STD_PCNT);  // Position, Color, Normal, and TexCoord
