/*
 * mp3acm.h -- the modern build's MP3 decoder in SndDrv.dll (MODERN.md X-2).
 *
 * Tron 2.0's speech, most of its sound effects that are not PCM, and all of
 * its music are MP3 in WAV files (WAVE_FORMAT_MPEGLAYER3, 0x55: 3,404 of the
 * 3,843 WAV files in Sound.rez; VOICE/ 2,084, MUSIC/ 1,317, SND/ 3).
 * Retail decodes them through Windows' Audio Compression Manager (ACM)
 * with the MP3 codec of the system: Fraunhofer's L3CODECA.ACM, or under Wine
 * winemp3.acm. SndDrv.dll does the speech and the effects; DirectMusic
 * (dswave.dll) does the music waves of its segments, in the same process.
 * Without an MP3 codec, SndDrv's Startup fails and the game has no sound.
 *
 * The modern build puts its own ACM codec into SndDrv.dll: a driver
 * procedure (Mp3Acm_DriverProc) on minimp3 (minimp3/, CC0), which
 * SndDrv adds to ACM as a local driver of the process (acmDriverAdd,
 * ACM_DRIVERADDF_FUNCTION | ACM_DRIVERADDF_LOCAL), as retail adds
 * L3CODECA.ACM itself. SndDrv uses it for every MP3 sound. ACM gives a
 * local driver priority over the drivers of the system, so DirectMusic's
 * own ACM calls in the process also find it. No system codec is necessary.
 *
 * The codec converts MP3 to 16-bit PCM at the rate and with the channels
 * of the MP3 (acmFormatSuggest gives that format). It does not encode.
 *
 * mp3acm.cpp is #included at the end of s_dx8.cpp in a TRON20SRC_MODERN
 * block: snddrv.dsp is the retail project file, and minimp3's macros stay
 * out of the retail code.
 *
 * Modern build only (TRON20SRC_MODERN).
 */
#ifndef TRON20SRC_MP3ACM_H
#define TRON20SRC_MP3ACM_H

// Adds the codec to ACM for this process (once; later calls give the same
// id). NULL if ACM refuses it.
HACMDRIVERID Mp3Acm_Register( void );

#endif
