/* Configure library by modifying this file */

#ifndef SMS_NTSC_CONFIG_H
#define SMS_NTSC_CONFIG_H

/* Format of source pixels */
#ifdef USE_15BPP_RENDERING
#define SMS_NTSC_IN_FORMAT SMS_NTSC_RGB15
#else
#define SMS_NTSC_IN_FORMAT SMS_NTSC_RGB16
#endif

/* Original CRAM format */
/* #define SMS_NTSC_IN_FORMAT SMS_NTSC_BGR12 */

/* The following affect the built-in blitter only; a custom blitter can
handle things however it wants. */

/* Type of input pixel values */
#define SMS_NTSC_IN_T unsigned short

/* Each raw pixel input value is passed through this. You might want to mask
the pixel index if you use the high bits as flags, etc. */
#define SMS_NTSC_ADJ_IN( in ) in

/* For each pixel, this is the basic operation:
output_color = SMS_NTSC_ADJ_IN( SMS_NTSC_IN_T ) */

#endif
