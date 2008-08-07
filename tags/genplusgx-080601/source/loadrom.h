/***************************************************************************************
 *  Genesis Plus 1.2a
 *
 *  Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003  Charles Mac Donald (original code)
 *  Copyright (C) 2006,2007,2008 Eke-Eke (GC/Wii port, compatibility fixes & additional code)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  ROM Loading Support
 ****************************************************************************************/
#ifndef _LOADROM_H_
#define _LOADROM_H_

#define MAXCOMPANY 64

typedef struct
{
  char consoletype[18];		/* Genesis or Mega Drive */
  char copyright[18];		/* Copyright message */
  char domestic[50];		/* Domestic name of ROM */
  char international[50];	/* International name of ROM */
  char ROMType[4];			/* Educational or Game */
  char product[14];			/* Product serial number */
  unsigned short checksum;	/* Checksum */
  char io_support[18];		/* Actually 16 chars :) */
  unsigned int romstart;
  unsigned int romend;
  char RAMInfo[14];
  unsigned int ramstart;
  unsigned int ramend;
  char modem[14];
  char memo[50];
  char country[18];
} ROMINFO;

typedef struct
{
  char companyid[6];
  char company[30];
} COMPANYINFO;

typedef struct
{
  char pID[2];
  char pName[21];
} PERIPHERALINFO;

/* Global variables */
extern ROMINFO rominfo;
extern COMPANYINFO companyinfo[MAXCOMPANY];
extern PERIPHERALINFO peripheralinfo[14];
extern uint16 realchecksum;

/* Function prototypes */
extern int load_rom(char *filename);

#endif /* _LOADROM_H_ */

