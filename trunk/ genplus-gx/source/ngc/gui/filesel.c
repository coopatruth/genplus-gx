/****************************************************************************
 *  Genesis Plus 1.2a
 *
 *  Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003  Charles Mac Donald
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
 * File Selection
 ***************************************************************************/ 
#include "shared.h"
#include "dvd.h"
#include "iso9660.h"
#include "font.h"
#include "unzip.h"
#include "diskio.h"
#include "vfat.h"


#define PAGESIZE 12
#define PADCAL 70

static int maxfiles;
u8 havedir   = 0;
u8 haveSDdir = 0;
u8 UseSDCARD = 0;
char rootSDdir[SDCARD_MAX_PATH_LEN];
int LoadFile (unsigned char *buffer);
int offset = 0;
int selection = 0;
int old_selection = 0;
int old_offset = 0;
VFATFS fs;
FSDIRENTRY f;


extern void reloadrom ();

/***************************************************************************
 * Showfile screen
 ***************************************************************************/ 
static void ShowFiles (int offset, int selection) 
{
  int i, j;
  char text[MAX_LONG_NAME+2];

  ClearScreen ();
  j = 0;

  for (i = offset; i < (offset + PAGESIZE) && (i < maxfiles); i++)
  {
	  memset(text,0,MAX_LONG_NAME+2);
	  if (filelist[i].flags) sprintf(text, "[%s]", filelist[i].displayname + filelist[i].filename_offset);
	  else sprintf (text, "%s", filelist[i].displayname + filelist[i].filename_offset);

      if (j == (selection - offset)) WriteCentre_HL ((j * fheight) + 120, text);
      else WriteCentre ((j * fheight) + 120, text);
	  j++;
  }
  SetScreen ();
}

/***************************************************************************
 * Update SDCARD curent directory name 
 ***************************************************************************/ 
int updateSDdirname()
{
  int size=0;
  char *test;
  char temp[1024];

   
   /* go up to parent directory */
   if (strcmp(filelist[selection].filename,"..") == 0) 
   {
     /* determine last subdirectory namelength */
     sprintf(temp,"%s",rootSDdir);
     test= strtok(temp,"/");
     while (test != NULL)
     { 
       size = strlen(test);
       test = strtok(NULL,"/");
     }
  
     /* remove last subdirectory name */
     size = strlen(rootSDdir) - size - 1;
     rootSDdir[size] = 0;

     return 1;
   }
   else
   {
       /* update current directory name */
       sprintf(rootSDdir, "%s/%s",rootSDdir, filelist[selection].filename);
       return 1;
   } 
}

/***************************************************************************
 * Browse SDCARD subdirectories 
 ***************************************************************************/ 
int parseSDdirectory()
{
  int nbfiles = 0;
  FSDIRENTRY fsdir;

  /* Get a list of files from the actual root directory */ 
  int res = VFAT_opendir(0, &fsdir, rootSDdir);
  
  if (res == FS_SUCCESS)
  {
	  while ( VFAT_readdir(&fsdir) == FS_SUCCESS )
      {
		  memset (&filelist[nbfiles], 0, sizeof (FILEENTRIES));
		  strcpy(filelist[nbfiles].displayname, fsdir.longname);
		  strcpy(filelist[nbfiles].filename, fsdir.shortname);
		  filelist[nbfiles].length = fsdir.fsize;
		  filelist[nbfiles].flags = fsdir.dirent.attribute & ATTR_DIRECTORY;
          nbfiles++;
	  }

	  VFAT_closedir(&fsdir);
  }
  
  return nbfiles;
}

/****************************************************************************
 * FileSelector
 *
 * Let user select a file from the File listing
 ****************************************************************************/ 
void FileSelector () 
{
	short p;
    signed char a,b;
	int haverom = 0;
	int redraw = 1;
	int go_up = 0;
	int i,size;
  
	while (haverom == 0)
	{
		if (redraw) ShowFiles (offset, selection);
		redraw = 0;
		p = PAD_ButtonsDown (0);
		a = PAD_StickY (0);
		b = PAD_StickX (0);

		/*
		 * check selection screen changes
		 */		

		/* scroll displayed filename */
  		if ((p & PAD_BUTTON_LEFT) || (b < -PADCAL))
		{
			if (filelist[selection].filename_offset > 0)
			{
				filelist[selection].filename_offset --;
				redraw = 1;
			}
		}
		else if ((p & PAD_BUTTON_RIGHT) || (b > PADCAL))
		{
			size = 0;
			for (i=filelist[selection].filename_offset; i<strlen(filelist[selection].filename); i++)
				size += font_size[(int)filelist[selection].filename[i]];
		  
			if (size > back_framewidth)
			{
				filelist[selection].filename_offset ++;
				redraw = 1;
			}
		}

		/* highlight next item */
		else if ((p & PAD_BUTTON_DOWN) || (a < -PADCAL))
		{
			filelist[selection].filename_offset = 0;
			selection++;
			if (selection == maxfiles) selection = offset = 0;
			if ((selection - offset) >= PAGESIZE) offset += PAGESIZE;
			redraw = 1;
		}

		/* highlight previous item */
		else if ((p & PAD_BUTTON_UP) || (a > PADCAL))
		{
			filelist[selection].filename_offset = 0;
			selection--;
			if (selection < 0)
			{
				selection = maxfiles - 1;
				offset = selection - PAGESIZE + 1;
			}
			if (selection < offset) offset -= PAGESIZE;
			if (offset < 0) offset = 0;
			redraw = 1;
		}
      
		/* go back one page */
		else if (p & PAD_TRIGGER_L)
		{
			filelist[selection].filename_offset = 0;
			selection -= PAGESIZE;
			if (selection < 0)
			{
				selection = maxfiles - 1;
				offset = selection - PAGESIZE + 1;
			}
			if (selection < offset) offset -= PAGESIZE;
			if (offset < 0) offset = 0;
			redraw = 1;
		}

		/* go forward one page */
		else if (p & PAD_TRIGGER_R)
		{
			filelist[selection].filename_offset = 0;
			selection += PAGESIZE;
			if (selection > maxfiles - 1) selection = offset = 0;
			if ((selection - offset) >= PAGESIZE) offset += PAGESIZE;
			redraw = 1;
		}
		
		/*
		 * Check pressed key
		 */
		 
		/* go up one directory or quit */
		if (p & PAD_BUTTON_B)
		{
			filelist[selection].filename_offset = 0;
			if (((!UseSDCARD) && (basedir == rootdir)) ||
				(UseSDCARD && strcmp(rootSDdir,"/") == 0)) return;
			go_up = 1;
		}

		/* quit */
		if (p & PAD_TRIGGER_Z)
		{
			filelist[selection].filename_offset = 0;
			return;
		}

		/* open selected file or directory */
		if ((p & PAD_BUTTON_A) || go_up)
		{
			filelist[selection].filename_offset = 0;
			if (go_up)
			{
				go_up = 0;
				selection = UseSDCARD ? 0 : 1;
			}

			/*** This is directory ***/
			if (filelist[selection].flags)
			{
				/* SDCARD directory handler */
				if (UseSDCARD)
				{
					/* update current directory */
					int status = updateSDdirname();

					/* move to new directory */
					if (status == 1)
					{
						/* reinit selector (previous value is saved for one level) */
						if (selection == 1)
						{
							selection = old_selection;
							offset = old_offset;
							old_selection = 0;
							old_offset = 0;
						}
						else
						{
							/* save current selector value */
							old_selection = selection;
							old_offset = offset;
							selection = 0;
							offset = 0;
						}

						/* set new entry list */
						maxfiles = parseSDdirectory();
						if (!maxfiles)
						{
							/* quit */
							WaitPrompt ("Error reading directory !");
							haverom   = 1;
							haveSDdir = 0;
						}
					}
					else if (status == -1)
					{
						/* quit */
						haverom   = 1;
						haveSDdir = 0;
					}
				}
				/* DVD directory handler */
				else 
				{
					/* move to a new directory */
					if (selection != 0)
					{
						/* update current directory */
						rootdir = filelist[selection].offset;
						rootdirlength = filelist[selection].length;
					 
						/* reinit selector (previous value is saved for one level) */
						if (selection == 1)
						{
							selection = old_selection;
							offset = old_offset;
							old_selection = 0;
							old_offset = 0;
						}
						else
						{
							/* save current selector value */
							old_selection = selection;
							old_offset = offset;
							selection = 0;
							offset = 0;
						}

						/* get new entry list */
						maxfiles = parsedirectory ();
					}
				}
			}
			else /*** This is a file ***/
			{
				rootdir = filelist[selection].offset;
				rootdirlength = filelist[selection].length;
				genromsize = LoadFile (cart_rom);
				reloadrom ();
				haverom = 1;
			}
			redraw = 1;
		}
	}
}

/****************************************************************************
 * OpenDVD
 *
 * Function to load a DVD directory and display to user.
 ****************************************************************************/ 
void OpenDVD () 
{
  UseSDCARD = 0;
  if (!getpvd())
  {
	ShowAction("Mounting DVD ... Wait");
	DVD_Mount();
	havedir = 0;
	if (!getpvd())
	{
		WaitPrompt ("Failed to mount DVD");
        return;
	}
  }
  
  if (havedir == 0)
  {
     /* don't mess with SD entries */
	 haveSDdir = 0;
	 
	 /* reinit selector */
	 rootdir = basedir;
	 old_selection = selection = offset = old_offset = 0;
	 
	 if ((maxfiles = parsedirectory ()))
	 {
       FileSelector ();
	   havedir = 1;
	 }
  }
  else FileSelector ();
}
 
/****************************************************************************
 * OpenSD updated to use the new libogc.  Written by softdev and pasted
 * into this code by Drack.
 * Modified for subdirectory browing & quick filelist recovery
 * Enjoy!
*****************************************************************************/
int OpenSD () 
{
  UseSDCARD = 1;
  char msg[20];
  
  if (haveSDdir == 0)
  {
     /* don't mess with DVD entries */
	 havedir = 0;
 
     /* reinit selector */
	 old_selection = selection = offset = old_offset = 0;
	
	 /* Reset SDCARD root directory */
     sprintf(rootSDdir,"/genplus/roms");
 
	 /* Parse initial root directory and get entries list */
     ShowAction("Reading Directory ...");
	 int res = VFAT_mount(FS_SLOTA, &fs);
	 if ( res != FS_TYPE_FAT16 )
	 {
		 sprintf(msg,"Error mounting SDCARD: %d", res);
		 WaitPrompt (msg);
		 return 0;
	 }

	 if ((maxfiles = parseSDdirectory ()))
	 {
       /* Select an entry */
	   FileSelector ();
       
       /* memorize last entries list, actual root directory and selection for next access */
	   haveSDdir = 1;
	 }
	 else
	 {
		/* no entries found */
		WaitPrompt ("Error reading /genplus/roms");
		return 0;
	 }
  }
  /* Retrieve previous entries list and made a new selection */
  else  FileSelector ();

  return 1;
}
  
/****************************************************************************
 * SDCard Get Info
 ****************************************************************************/ 
void GetSDInfo () 
{
  char fname[SDCARD_MAX_PATH_LEN];
  rootdirlength = 0;
 
  /* Check filename length */
  sprintf(fname, "%s/%s",rootSDdir,filelist[selection].filename); 

  int res = VFAT_fopen(0, &f, fname, FS_READ);
  if (res != FS_SUCCESS )
  {
	  WaitPrompt ("Unable to open file!");
      return;
  }
}

/****************************************************************************
 * LoadFile
 *
 * This function will load a file from DVD or SDCARD, in BIN, SMD or ZIP format.
 * The values for offset and length are inherited from rootdir and 
 * rootdirlength.
 *
 * The buffer parameter should re-use the initial ROM buffer.
 ****************************************************************************/ 
int LoadFile (unsigned char *buffer) 
{
  int offset;
  int blocks;
  int i;
  u64 discoffset;
  char readbuffer[2048];
  
  /* SDCard Addition */ 
  if (UseSDCARD) GetSDInfo ();
  
  /* How many 2k blocks to read */ 
  if (rootdirlength == 0) return 0;
  blocks = rootdirlength / 2048;

  offset = 0;
  discoffset = rootdir;
  ShowAction ("Loading ... Wait");
  
  if (UseSDCARD) VFAT_fread(&f, readbuffer, 2048);
  else dvd_read (&readbuffer, 2048, discoffset);
  
  if (!IsZipFile ((char *) readbuffer))
  {
      if (UseSDCARD) VFAT_fseek(&f, 0, SEEK_SET);

      for (i = 0; i < blocks; i++)
      {
	      if (UseSDCARD) VFAT_fread (&f, readbuffer, 2048);
	  	  else dvd_read(readbuffer, 2048, discoffset);
	      memcpy (buffer + offset, readbuffer, 2048);
	      offset += 2048;
	      discoffset += 2048;
	  }
      
	  /* And final cleanup */ 
	  if (rootdirlength % 2048)
	  {
	     i = rootdirlength % 2048;
	     if (UseSDCARD) VFAT_fread (&f, readbuffer,  i);
	     else dvd_read (readbuffer, 2048, discoffset);
	     memcpy (buffer + offset, readbuffer, i);
	  }
  }
  else return UnZipBuffer (buffer, discoffset, rootdirlength);
  
  if (UseSDCARD) VFAT_fclose(&f);

  return rootdirlength;
}
