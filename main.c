/*******************************
*
*  Project Name: Wave Goodbye
*  Description: write a program can do reverse, change speed, flip channel, fade in, fade out, volume, echo to a audio file
*  File names: main.c wave.c wave.h
*  Date: March 23, 2018
*  Authors: Anh Nguyen 
*
*******************************/
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "wave.h"

void reverse(short *data, int size);
short *changeSpeed(short *data, double factor, int size);
void fadeIn(short *data, double seconds, int OriginalLength);
void fadeOut(short *data, double seconds, int OriginalLength);
void volume(short *data, double rate, int size);
short *echo(short *data, double delay, double factor, int size);
short getShort();
short limit(double n);
void putShort(short n);

int main(int argc, char **argv)
{	
	WaveHeader header;
	//READING A FILE
	readHeader (&header);
	unsigned int size = header.dataChunk.size/4;//trun size from byte to numbers of int
	short *leftchanel = malloc(size * sizeof(short));
	short *rightchanel = malloc(size * sizeof(short));
	
	if ( *leftchannel == NULL || *rightchanel == NULL )// error case for not stereo
	{
		fprintf(stderr,"File is not stereo\n");
		exit(-1);
	}
	
	int i = 0;
	for(i = 0; i<size ; i++)//store data of left and right chanel into different array
	{
		leftchanel[i] = getShort();
		rightchanel[i] = getShort();
	}
	
	//line argument reading
	
	double factor;
	short *temp;
	double seconds;
	double rate;
	double delay;
	
	for( i = 0; i < argc; ++i )//go through whole line argument
	{
		//if line argument contains any following comand, edit the file as comanded
		if(strcmp(argv[i] , "-r") == 0)//reverse
		{
			reverse(leftchanel, size);
			reverse(rightchanel, size);
		}
		else if(strcmp(argv[i] , "-s") == 0)//speed change
		{
			
			factor = atof(argv[i+1]);
			leftchanel = changeSpeed(leftchanel, factor, size);
			rightchanel = changeSpeed(rightchanel, factor, size);
			size /= factor;//update size
			i++;//update index
		}
	
		if(strcmp(argv[i] , "-f") == 0)//flip channel
		{
			temp = leftchanel;
			leftchanel = rightchanel;
			rightchanel = temp;
		}
		if(strcmp(argv[i] , "-o") == 0)//fade out
		{
			seconds = atof(argv[i+1]);
			fadeOut(leftchanel,seconds,size);
			fadeOut(rightchanel,seconds,size);
			i++;//update index
		}
		if(strcmp(argv[i] , "-i") == 0)//fade in
		{
			seconds = atof(argv[i+1]);
			fadeIn(leftchanel,seconds,size);
			fadeIn(rightchanel,seconds,size);
			i++;//update index
		}
		if(strcmp(argv[i] , "-v") == 0)//change volume
		{
			rate = atof(argv[i+1]);
			volume(leftchanel, rate, size);
			volume(rightchanel, rate, size);
			i++;//update index
		}

		if(strcmp(argv[i] , "-e") == 0)//add echo
		{
			delay = atof(argv[i+1]);
			factor = atof(argv[i+2]);
			leftchanel = echo(leftchanel, delay, factor, size);
			rightchanel = echo(rightchanel, delay, factor, size);
			size =  size+delay*44100;//update size
			i += 2;//update index
		}
		else // error case for command lines
		{
		fprintf(stderr, "Usage: wave [[-r][-s factor][-f][-o delay][-i delay][-v scale][-e delay scale] < input > output\n");
		exit(-1);
		}
	}

	//writing
	header.dataChunk.size = size*4;//change size back to bytes
	header.size = header.dataChunk.size + 36;//update the size of whole file
	
	writeHeader(&header);
	
	for(i = 0; i<size; i++)//write in left and right channel
	{
		putShort(leftchanel[i]);
		putShort(rightchanel[i]);
	}

	//free memory
	free(leftchanel);
	free(rightchanel);
	return 0;
}

void reverse(short *data, int size)//reverse method
{
	short temp = 0;
	int i = 0;
	
	for(i = 0; i < size/2 ; i++)//loop through array and swaping
	{
		temp = data[i];
		data[i] = data[size-i-1];
		data[size-i-1] = temp;
	}
}

short *changeSpeed(short *OriginalData, double factor, int size)//speed changing method
{
	short  *data = malloc(sizeof(short)*(size/factor));//create a new data array to store changed information
	int i = 0;
	
	for(i = 0; i < size/factor; i++)//loop through new data array and fill it with corresponding elements
	{
		data[i] = limit(OriginalData[(int)(i*factor)]);
	}
	
	if( factor < 0 || factor >='A' || factor <= 'Z')//error case 
	{
		fprintf(stderr, "A positive number must be supplied for the speed change\n");
		exit(-1);
	}
	
	free(OriginalData);
	return data;
}

void fadeIn(short *data, double seconds, int OriginalLength)//fade in method
{
	double sampleNum = 44100*seconds;//seconds times sample rate, 44100, equals numbers of sample

	int i = 0;
	
	for(i = 0; i < sampleNum && i < OriginalLength; i++)
	{
		data[i] = limit(data[i]*(i/sampleNum)*(i/sampleNum));//multiply 0.0 to 1.0 to every element in array;
	}
	
	if( seconds < 0 || seconds >='A' || seconds <= 'Z')//error case 
	{
		fprintf(stderr, "A positive number must be supplied for the fade in time\n");
		exit(-1);
	}
}

void fadeOut(short *data, double seconds, int OriginalLength)//fate out method
{
	double sampleNum = 44100*seconds;//seconds times sample rate, 44100, equals numbers of sample
	
	int i = sampleNum;
	
	if(sampleNum >= OriginalLength)
		i = OriginalLength -1;
		
	while(i >= 0)
	{
		data[i] = limit(data[i]*(i/sampleNum)*(i/sampleNum));//multiply 1.0 to 0.0 to every element in array;
		i--;
	}
	if( seconds < 0 || seconds >='A' || seconds <= 'Z')//error case 
	{
		fprintf(stderr, "A positive number must be supplied for the fade out time\n");
		exit(-1);
	}
}

void volume(short *data, double rate, int size)//volume change method
{
	int i = 0;
	
	for(i = 0; i < size; i++)
	{
		data[i] = limit(data[i] * rate);
	}
	if( rate < 0 || rate >='A' || rate <= 'Z')//error case 
	{
		fprintf(stderr, "A positive number must be supplied for the volume scale\n");
		exit(-1);
	}
}

short *echo(short *data, double delay, double factor, int size)//echo method
{
	short *NewData = calloc((size+delay*44100), sizeof(short));//create new data array with calloc to make sure everthing zeros out
	
	int i = 0;

	for(i = 0; i < size; i++)//copying data into new data array
		NewData[i] = data[i];
		
	for(i = 0; i < size; i++)//add echo into array
		NewData[(int)(i+ delay*44100)] = limit(NewData[(int)(i+ delay*44100)]+data[i]*factor);
		
	if( (delay < 0 && scale < 0) || (delay >='A' && scale >='A') || (delay <= 'Z' && scale <'Z'))//error case 
	{
		fprintf(stderr, "Positive numbers must be supplied for the echo delay and scale parameters\n");
		exit(-1);
	}	
		
	free(data);
	return NewData;	
	
}

short getShort()//turing char value we read into short
{
	int a = getchar();
	int b = getchar();
	
	return (short)(a|(b<<8));
}

short limit(double n)//make sure values in data array will not go out of max and min of a short value
{
	if(n>SHRT_MAX)
		return SHRT_MAX;
	if(n<SHRT_MIN)
		return SHRT_MIN;
	return (short)n;
}

void putShort(short n)//turning short into binary code and write into file
{
	unsigned int a = n & 0xFF;
	unsigned int b = n>>8;
	putchar(a);
	putchar(b);
}


















