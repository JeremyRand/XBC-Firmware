// waveutil.cpp : Defines the entry point for the console application.
//


//////////////////////////////////////////////////////////////////////////////
// wav2gba.cpp                                                              //
//////////////////////////////////////////////////////////////////////////////
/*
Converts a wave file to WaveData for use with GBA.

  by Dark Fader
  v1.01 - stereo to two output files, file padding
  v1.00 - initial version
  Rich LeGrand modified to output C arrays
*/

//////////////////////////////////////////////////////////////////////////////
// Pragma's                                                                 //
//////////////////////////////////////////////////////////////////////////////
#pragma pack(1)

//////////////////////////////////////////////////////////////////////////////
// Includes                                                                 //
//////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////////
// Defines                                                                  //
//////////////////////////////////////////////////////////////////////////////
#define VER				"1.11"
#define WAVE_STAT_LOOP  0xC000

//////////////////////////////////////////////////////////////////////////////
// Variables                                                                //
//////////////////////////////////////////////////////////////////////////////
int stat = 0;	//WAVE_STAT_LOOP;

//////////////////////////////////////////////////////////////////////////////
// typedefs                                                                 //
//////////////////////////////////////////////////////////////////////////////
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

typedef struct
	{
	char	riff_id[4]; // "RIFF"
	u32		riff_len;
	
	char	wave_id[4]; // "WAVE"
	
	char	fmt_id[4]; // "fmt "
	u32		fmt_len;
	u16		fmt_tag;
	u16		channels; // 1=mono, 2=stereo
	u32		samplerate;
	u32		samplerate2;
	u16		u8s_per_sample;
	u16		bits; // 8/16 bits
	
	char	data_id[4]; // "data"
	u32		data_len;
	
	//u8		padding;
	
	u8		wave_data[1];
	} WaveHdr;

//////////////////////////////////////////////////////////////////////////////
// DoChannel                                                                //
//////////////////////////////////////////////////////////////////////////////
int DoChannel(WaveHdr &hdr, FILE *fi, char *outfile, int channels, int channel)
	{
	FILE *fo = fopen(outfile, "wb");
	if (!fo) { printf("Error opening output file!\n"); return -1; }
	
	int type = 0;
	int freq = hdr.samplerate<<10;
	int loop = 0;
	int size = hdr.data_len / channels;
	
	// samples
	unsigned char *buf1;
	
	fseek(fi, (char *)&hdr.wave_data - (char *)&hdr + channel, SEEK_SET);
	buf1 = new unsigned char[hdr.data_len];
	fread(buf1, 1, hdr.data_len, fi);
	
	unsigned char *p1 = buf1;
	unsigned int i, j;
	unsigned long ltemp = 0;
	char var[256];

	// extract variable name
	for (i=0, j=0; outfile[i]!=0; i++)
		{
		if (j==0)
			{
			if (outfile[i]!='.' && outfile[i]!='\\' && outfile[i]!='/')
				var[j++] = outfile[i];
			}
		else // j!=0
			{
			if (outfile[i]=='.')
				{
				var[j++] = '\0';
				break;
				}
			else
				var[j++] = outfile[i];
			}
		}
			
	fprintf(fo, "const unsigned long %s[] =\n{\n", var);
	for (i=0; i<hdr.data_len/channels; i++)
		{
		ltemp |= (*p1 - 0x80)<<24;
		p1 += channels;
		if (i%4==3)
			{
			fprintf(fo, "0x%08x", ltemp);
			if (i!=(hdr.data_len/channels-1))
				fprintf(fo, ", ");
			if (i%32==31)
				fprintf(fo, "\n");
			}
		ltemp >>= 8;		
		}
	
	fprintf(fo, "\n};\n");
	
	fclose(fo);
	
	delete buf1;
	
	return 0;
	}

//////////////////////////////////////////////////////////////////////////////
// DoWave                                                                   //
//////////////////////////////////////////////////////////////////////////////
int DoWave(char *infile, char *outfiles[])
	{
	// input .wav
	FILE *fi = fopen(infile, "rb");
	if (!fi) { printf("Error opening input file!\n"); return -1; }
	
	// read header
	WaveHdr hdr;
	fread(&hdr, sizeof(hdr), 1, fi);
	if (strncmp(hdr.riff_id, "RIFF", 4) != 0) { printf("'WAVE' not found\n"); return -1; }
	if (strncmp(hdr.wave_id, "WAVE", 4) != 0) { printf("'WAVE' not found\n"); return -1; }
	if (strncmp(hdr.fmt_id, "fmt ", 4) != 0) { printf("'fmt ' not found\n"); return -1; }
	if (strncmp(hdr.data_id, "data", 4) != 0) { printf("'data' not found\n"); return -1; }
	if (hdr.bits != 8) { printf("8 bit required\n"); return -1; }
	
	// process channels
	int channels = hdr.channels;
	for (int channel=0; channel<channels; channel++)
		{
		if (DoChannel(hdr, fi, outfiles[channel], channels, channel)) return -1;
		}
	
	fclose(fi);
	return 0;
	}

//////////////////////////////////////////////////////////////////////////////
// main                                                                     //
//////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
	{
	char *infile=0, *outfiles[2]={0,0};
	
	if (argc < 3)
		{
		printf("wav2gba v"VER" by Rafael Vuijk (aka Dark Fader)\n");
		printf("Syntax: %s [-l] <input.wav> <output.inl> [<output2.inl>]\n", argv[0]);
		printf("Options:\n");
		printf("    -l       set loop flag\n");
		return -1;
		}
	
	for (int i=1; i<argc; i++)
		{
		if (argv[i][0] == '-')	// flag
			{
			for (int j=1; argv[i][j]; j++)
				{
				switch (argv[i][j])
					{
					case 'l':
						{
						stat = WAVE_STAT_LOOP;
						break;
						}
						
					default:
						{
						printf("Unknown flag: %c\n", argv[i][j]);
						break;
						}
					}
				}
			}
		else
			{
			if (!infile) infile = argv[i];
			else if (!outfiles[0]) outfiles[0] = argv[i];
			else if (!outfiles[1]) outfiles[1] = argv[i];
			else { printf("Too many arguments!\n"); return -1; }
			}
		}
	
	if (!infile || !outfiles[0])
		{
		printf("File arguments needed!\n");
		return -1;
		}
	
	if (DoWave(infile, outfiles)) return -1;
	
	return 0;
	}

