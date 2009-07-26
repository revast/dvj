/*
 *
 * logDrawer
 *
 * Copyright Chris Nelson, 2007
 *
 */

//log everything:
//70kB / frame
//300MB / minute
//18GB / hour (Yikes!)

//log visuals:
//1.5kB / frame
//4MB / minute
//280MB / hour

#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "LGL.module/LGL.h"
#include "FileInterface.h"
#include "Particle.h"

#include "../../../src/Common.cpp"	//This is so screwy, but it works.
#include "../../../src/ConfigFile.cpp"	//This is so screwy, but it works.
#include "../../../src/Config.cpp"	//This is so screwy, but it works.
#include "../../../src/Turntable.h"	//For NOISE_IMAGE_COUNT_256_64

#define	ENTIRE_WAVE_ARRAY_COUNT	(1920)

void
LoadWaveArrayData
(
	LGL_Sound*	sound,
	float*		entireWaveArrayMagnitudeAve,
	float*		entireWaveArrayMagnitudeMax,
	float*		entireWaveArrayFreqFactor
)
{
	if(sound==NULL)
	{
		return;
	}

	char waveArrayDataPath[1024];
	sprintf(waveArrayDataPath,"data/cache/waveArrayData/%s.dvj-wavearraydata-%i.bin",sound->GetPathShort(),ENTIRE_WAVE_ARRAY_COUNT);

	int expectedSize=sizeof(float)*ENTIRE_WAVE_ARRAY_COUNT*3;
	if
	(
		LGL_FileExists(waveArrayDataPath) &&
		expectedSize!=LGL_FileLengthBytes(waveArrayDataPath)
	)
	{
		//Bad data... Whatever.
		LGL_FileDelete(waveArrayDataPath);
		return;
	}

	FILE* fd=fopen(waveArrayDataPath,"rb");
	if(fd)
	{
		fread(entireWaveArrayMagnitudeAve,sizeof(float),ENTIRE_WAVE_ARRAY_COUNT,fd);
		fread(entireWaveArrayMagnitudeMax,sizeof(float),ENTIRE_WAVE_ARRAY_COUNT,fd);
		fread(entireWaveArrayFreqFactor,sizeof(float),ENTIRE_WAVE_ARRAY_COUNT,fd);
		fclose(fd);
	}
}

int main(int argc, char** argv)
{
	int ResX=1024;
	int ResY=768;
	bool visualsOnly=false;
	bool noPointers=false;
	bool noText=false;
	bool allFormats=false;

	float OmniFader=0.0f;

	int bitrateHQ = 4000;
	char codec[128];
	strcpy(codec,"mpeg4");

	int videoSkipFrames = 0;	//Sync Video + Audio. Without this, Video lags noticably.

	LGL_Video* vid = NULL;

	for(int a=0;a<argc;a++)
	{
		if(strcasecmp(argv[a],"--fullscreen")==0)
		{
			ResX=1920;
			ResY=1200;
		}
		if(strcasecmp(argv[a],"--projector")==0)
		{
			ResX=1600;
			ResY=1200;
		}
		if(strcasecmp(argv[a],"--1080p")==0)
		{
			ResX=1920;
			ResY=1080;
		}
		if(strcasecmp(argv[a],"--720p")==0)
		{
			ResX=1280;
			ResY=720;
		}
		if(strcasecmp(argv[a],"--480p")==0)
		{
			ResX=852;
			ResY=480;
		}
		if(strcasecmp(argv[a],"--240p")==0)
		{
			ResX=320;
			ResY=240;
		}
		if(strcasecmp(argv[a],"--resolution")==0)
		{
			if(a+2<argc)
			{
				ResX=atoi(argv[a+1]);
				ResY=atoi(argv[a+2]);
				a+=2;
			}
		}

		if(strcasecmp(argv[a],"--visualsOnly")==0)
		{
			visualsOnly=true;
		}

		if(strcasecmp(argv[a],"--bitrate")==0)
		{
			if(a+1<argc)
			{
				bitrateHQ = atoi(argv[a+1]);
				assert(bitrateHQ >= 32 && bitrateHQ <= 8000);
				a++;
			}
		}
		if(strcasecmp(argv[a],"--codec")==0)
		{
			if(a+1<argc)
			{
				strcpy(codec,argv[a+1]);
				a++;
			}
		}
		if(strcasecmp(argv[a],"--allFormats")==0)
		{
			allFormats=true;
		}
		if(strcasecmp(argv[a],"--noPointers")==0)
		{
			noPointers=true;
		}
		if(strcasecmp(argv[a],"--noText")==0)
		{
			noText=true;
		}
		if(strcasecmp(argv[a],"--help")==0)
		{
			printf("\n");
			printf("logDrawer arguments:\n");
			printf("\n");
			printf("\t--allFormats\n");
			printf("\t--codec mpeg4 (default) h264 NULL\n");
			printf("\t--bitrate 4000 (default)\n");
			printf("\t--visualsOnly\n");
			printf("\t--noPointers\n");
			printf("\t--noText\n");
			printf("\t--fullscreen (1920x1200)\n");
			printf("\t--1080p (1920x1080)\n");
			printf("\t--projector (1600x1200)\n");
			printf("\t--720p (1280x720)\n");
			printf("\t--480p (640x480)\n");
			printf("\t--240p (320x240)\n");
			printf("\t--resolution 1024 768\n");
			printf("\n");
			exit(0);
		}
	}

	bool nullCodec = strcasecmp(codec,"NULL")==0;

	LGL_Init
	(
		ResX,ResY,
		0,
		"Luminescence | logDrawer"
	);

	//gunzip first!

	if(LGL_FileExists("data/record/drawlog.txt.gz"))
	{
		system("gunzip -c data/record/drawlog.txt.gz > data/record/drawlog.txt");
	}

	FILE* fd=fopen64("data/record/drawlog.txt","r");
	if(fd==NULL)
	{
		printf("logDrawer: Error! Cannot fopen() data/record/drawlog.txt!\n");
		exit(-1);
	}

	//First pass: Figure out how many seconds of video and audio think we have
/*
	long fdLengthBytes=LGL_FileLengthBytes("data/record/drawlog.txt");
	double audioLengthSeconds=0.0;
	double videoLengthSeconds=0.0;
	double ratioVideoOverAudio=0.0;
	int frameCounter=0;
*/
	FileInterfaceObj fi;
	
	char audioFile[2048];

	LGL_Image* NoiseImage[NOISE_IMAGE_COUNT_256_64];
	for(int a=0;a<NOISE_IMAGE_COUNT_256_64;a++)
	{
		char path[1024];
		sprintf
		(
			path,
			"data/noise/256x64/%02i.png",
			a
		);
		assert(LGL_FileExists(path));
		NoiseImage[a] = new LGL_Image(path);
	}

	for(;;)
	{
		fi.ReadLine(fd);
		if(feof(fd))
		{
			break;
		}

		if(fi.Size()==0)
		{
			continue;
		}

		if(strcasecmp(fi[0],"!Luminescence::Record.mp3")==0)
		{
			assert(fi.Size()==2);
			if(LGL_FileExists("data/record/visualizer-audio-override.mp3"))
			{
				sprintf(audioFile,"data/record/visualizer-audio-override.mp3");
			}
			else
			{
				sprintf(audioFile,"%s",fi[1]);
			}
			break;
		}
		/*
		else if(strcasecmp(fi[0],"LGL_SwapBuffers")==0)
		{
			assert(fi.Size()==2);
			videoLengthSeconds=atof(fi[1]);

			frameCounter++;

			if(frameCounter>1000)
			{
				frameCounter=0;
				
				LGL_ProcessInput();
				if(LGL_KeyStroke(SDLK_ESCAPE))
				{
					exit(0);
				}
				double pct = ftell(fd)/(double)fdLengthBytes;
				LGL_DrawRectToScreen
				(
					0,pct,
					0,0.05f,
					.4f*pct,.2f*pct,0.5f+0.5f*pct,1.0f
				);
				LGL_GetFont().DrawString
				(
					0.49f,0.015f,0.02f,
					1,1,1,1,
					false,
					.75f,
					"%.0f%%",
					pct*100.0f
				);
				LGL_GetFont().DrawString
				(
					0.49f,0.065f,0.025f,
					1,1,1,1,
					true,
					.75f,
					"First Pass"
				);
				LGL_DelaySeconds(1.0f/120.0f-LGL_SecondsSinceThisFrame());
				LGL_SwapBuffers();
			}
		}
		else if(strcasecmp(fi[0],"!LuminescenceVideoLengthSeconds")==0)
		{
			assert(fi.Size()==2);
			videoLengthSeconds=atof(fi[1]);
			break;
		}
		*/
	}
/*
printf("Video len: %.2f\n",videoLengthSeconds);
	ratioVideoOverAudio=videoLengthSeconds/audioLengthSeconds;
printf("ratioVideoOverAudio: %f\n",ratioVideoOverAudio);
exit(0);
*/

	//Second pass: Render + Encode

	rewind(fd);

	if(visualsOnly)
	{
		LGL_ViewPortScreen(0.0f,0.5f,0.5f,1.0f);
	}

	char videoFile[2048];
	char noExtensionFile[2048];
	char noPathNoExtensionFile[2048];

	float crossFadeSliderLeft = 0.5f;
	float crossFadeSliderRight = 0.5f;

	LGL_Image* ParticleImage=NULL;
	ParticleSystemObj* ParticleSystem[8];
	bool ParticleSystemActive[8];
	bool ParticleSystemUpdatedThisFrame[8];
	for(int a=0;a<8;a++)
	{
		ParticleSystem[a]=NULL;
		ParticleSystemActive[a]=false;
		ParticleSystemUpdatedThisFrame[a]=false;
	}
	bool ttLines=true;
	bool noCE=false;

	double homePointSeconds[18];
	float homePointFlash[18];
	float homePointNoise[18];
	
	LGL_Image* fbImage=NULL;

	std::vector<LGL_Image*> imageList;
	std::vector<LGL_Video*> videoList;
	LGL_Sound* soundList[10];
	for(int a=0;a<10;a++)
	{
		soundList[a]=NULL;
	}
	LGL_DirTree dirTrees[2];
	dirTrees[0].SetPath("data/music");
	dirTrees[1].SetPath("data/music");

	bool firstWiimoteActive=false;

	double videoTimeNow=0.0;
	double videoTimeScalar=1.0;//0.999371752977849;	//Vid Length: 3320.350, Aud Length: 3318.264
	double videoTimeActual=0.0;
	double videoTimePrev=0.0;
	double videoTimeNext=0.0;
	double videoTimeStep=1.0/60.0;
	unsigned char* pixels=NULL;

	FILE* mencoderFD=NULL;

	int lineNum=0;

	for(;;)
	{
		if(videoList.size()>4)
		{
			for(unsigned int a=0;a<videoList.size();a++)
			{
				delete videoList[a];
				videoList[a]=NULL;
			}
			videoList.clear();
		}
		fi.ReadLine(fd);
		lineNum++;
		//printf("Line %i\n",lineNum);
		if(feof(fd))
		{
			break;
		}

		if(fi.Size()==0)
		{
			continue;
		}

		if(strcasecmp(fi[0],"dr")==0)
		{
			//LGL_DrawRectToScreen()

			assert(fi.Size()==10);
			LGL_DrawRectToScreen
			(
				atof(fi[1]),atof(fi[2]),				//left,right
				atof(fi[3]),atof(fi[4]),				//bottom,top
				atof(fi[5]),atof(fi[6]),atof(fi[7]),atof(fi[8]),	//r,g,b,a
				atof(fi[9])						//rotation
			);
		}
		else if(strcasecmp(fi[0],"f")==0)
		{
			//LGL_GetFont().DrawString()

			assert(fi.Size()==7 || fi.Size()==11);
			float R=1.0f;
			float G=1.0f;
			float B=1.0f;
			float A=1.0f;
			bool centered=false;
			float shadowAlpha=0.0f;
			char* str=NULL;
			if(fi.Size()==11)
			{
				R=atof(fi[4]);
				G=atof(fi[5]);
				B=atof(fi[6]);
				A=atof(fi[7]);
				centered=atoi(fi[8])!=0;
				shadowAlpha=atof(fi[9]);
				str=fi[10];
			}
			else
			{
				centered=atoi(fi[4])!=0;
				shadowAlpha=atof(fi[5]);
				str=fi[6];
			}
			if(noText==false)
			{
				LGL_GetFont().DrawString
				(
					atof(fi[1]),atof(fi[2]),atof(fi[3]),			//x,y,h
					R,G,B,A,						//r,g,b,a
					centered,						//centered
					shadowAlpha,						//shadow alpha
					str							//str
				);
			}
		}
		else if(strcasecmp(fi[0],"LGL_DrawLineToScreen")==0)
		{
			assert(fi.Size()==11);
			LGL_DrawLineToScreen
			(
				atof(fi[1]),atof(fi[2]),				//x1,y1
				atof(fi[3]),atof(fi[4]),				//x2,y2
				atof(fi[5]),atof(fi[6]),atof(fi[7]),atof(fi[8]),	//r,g,b,a
				atof(fi[9]),						//Thickness
				atoi(fi[10])						//Antialias
			);
		}
		else if(strcasecmp(fi[0],"LGL_Image::DrawToScreen")==0)
		{
			assert(fi.Size()==16);
			LGL_Image* image=NULL;
			for(unsigned int a=0;a<imageList.size();a++)
			{
				if(strcmp(imageList[a]->GetPath(),fi[1])==0)
				{
					image=imageList[a];
					break;
				}
			}
			bool imageLocked=false;
			if(image==NULL)
			{
				if(fi[1][0]=='!')
				{
					//It's a RAM Image! (Or a video image...)
					char imageName[1024];
					char imageTimeSeconds[1024];
					int secondChk=-1;
					for(unsigned int a=1;a<strlen(fi[1]);a++)
					{
						if(fi[1][a]=='!')
						{
							assert(secondChk==-1);
							secondChk=a;
						}
					}

					fi[1][secondChk]='\0';
					strcpy(imageName,&(fi[1][1]));
					strcpy(imageTimeSeconds,&(fi[1][secondChk+1]));
					vid=NULL;
					for(unsigned int a=0;a<videoList.size();a++)
					{
						assert(videoList[a]!=NULL);
						if(strcmp(videoList[a]->GetPath(),imageName)==0)
						{
							vid=videoList[a];
							break;
						}
					}
					
					if(vid==NULL)
					{
						printf("New Video: %s\n",imageName);
						vid = new LGL_Video(imageName);
						videoList.push_back(vid);
					}
					/*
					else
					{
						printf("Set Video: %s\n",imageName);
						vid->SetVideo(imageName);
					}
					*/
					vid->SetPrimaryDecoder();
					vid->SetTime(atof(imageTimeSeconds));
					for(int a=0;a<200;a++)
					{
						if(vid->ImageUpToDate())
						{
							break;
						}
					}
					image=vid->LockImage();
					imageLocked=true;
				}
				else
				{
					//It's an image from our filesystem
					image=new LGL_Image(fi[1]);
					imageList.push_back(image);
				}
			}

			if(image!=NULL)
			{
				if
				(
					noPointers==false ||
					strcmp("data/particle.png",image->GetPath())!=0
				)
				{
					image->DrawToScreen
					(
						atof(fi[3]),atof(fi[4]),				//left,right
						atof(fi[5]),atof(fi[6]),				//bottom,top
						atof(fi[7]),						//rotation
						atof(fi[8]),atof(fi[9]),atof(fi[10]),atof(fi[11]),	//r,g,b,a
						false,false,0.0f,0.0f,0.0f,				//TODO: Support fading
						atof(fi[12]),atof(fi[13]),atof(fi[14]),atof(fi[15])	//lrbt subimage
					);
				}
			}

			if(imageLocked && vid)
			{
				vid->UnlockImage(image);
			}
		}
		else if(strcasecmp(fi[0],"fbu")==0)
		{
			assert(fi.Size()==1);
			if(fbImage!=NULL)
			{
				for(int a=0;a<8;a++)
				{
					if
					(
						noPointers==false &&
						ParticleSystem[a]!=NULL
					)
					{
						ParticleSystem[a]->Draw(OmniFader);
					}
				}
				fbImage->FrameBufferUpdate();
			}
		}
		else if(strcasecmp(fi[0],"fbd")==0)
		{
			assert(fi.Size()==19);

			bool front=atoi(fi[1]);
			float fl=atof(fi[2]);
			float fb=atof(fi[3]);
			float fw=atof(fi[4]);
			float fh=atof(fi[5]);
			float fr=fl+fw;
			float ft=fb+fh;

			if(visualsOnly)
			{
				fl=0.0f;
				fb=0.0f;
				fw=1.0f;
				fh=1.0f;
				fr=1.0f;
				ft=1.0f;
			}

			if(fbImage==NULL)
			{
				fbImage=new LGL_Image
				(
					fl,fr,
					fb,ft,
					front
				);
			}
			else
			{
				fbImage->DrawToScreen
				(
					atof(fi[6]),atof(fi[7]),atof(fi[8]),atof(fi[9]),	//l,r,b,t
					atof(fi[10]),						//rotation
					atof(fi[11]),atof(fi[12]),atof(fi[13]),atof(fi[14]),	//r,g,b,a
					false,false,0.0f,0.0f,0.0f,				//TODO: Support fading
					atof(fi[15]),atof(fi[16]),atof(fi[17]),atof(fi[18])
				);
			}
		}
		else if(strcasecmp(fi[0],"vqcre")==0)
		{
			assert(fi.Size()==2);
			noCE=fi[1][0]=='T';
			if(visualsOnly==false && noCE)
			{
				LGL_ClipRectEnable
				(
					0.0f,0.5f,
					0.5f,1.0f
				);
			}
		}
		else if(strcasecmp(fi[0],"ce")==0)
		{
			assert(fi.Size()==5);
			if(visualsOnly==false && noCE==false)
			{
				LGL_ClipRectEnable
				(
					atof(fi[1]),atof(fi[2]),				//l,r
					atof(fi[3]),atof(fi[4])					//b,t
				);
			}
			else if(visualsOnly==false)
			{
				LGL_ClipRectEnable
				(
					0.0f,0.5f,
					0.5f,1.0f
				);
			}
		}
		else if(strcasecmp(fi[0],"cd")==0)
		{
			assert(fi.Size()==1);
			if(visualsOnly==false)
			{
				LGL_ClipRectDisable();
			}
		}
		else if(strcasecmp(fi[0],"LGL_DrawLineStripToScreen")==0)
		{
			if(fi.Size()!=8)
			{
				printf("Bad line! Expected fi.Size() to be 8, not %i!\n",fi.Size());
				printf("\t'%s'\n",fi.GetLine());
				assert(fi.Size()==8);
			}

			int pointCount=atoi(fi[1]);
			float r=atof(fi[2]);
			float g=atof(fi[3]);
			float b=atof(fi[4]);
			float a=atof(fi[5]);
			float thickness=atof(fi[6]);
			bool antialias=atoi(fi[7]);

			//Obtain Point Data

			fi.ReadLine(fd);
			assert(!feof(fd));
			assert(fi.Size()==2);
			assert(strcasecmp(fi[0],"D")==0);
			long dataPointsLen=atol(fi[1]);

			char* dataPoints=new char[dataPointsLen];
			fread(dataPoints,dataPointsLen,1,fd);
			float* dataPointsFloat=(float*)dataPoints;

			//Process Data

			LGL_DrawLineStripToScreen
			(
				dataPointsFloat,
				pointCount,
				r,g,b,a,
				thickness,
				antialias
			);
			
			//Clean Up

			delete dataPoints;
		}
		else if(strcasecmp(fi[0],"LGL_DrawLineStripToScreenColors")==0)
		{
			assert(fi.Size()==4);
			int pointCount=atoi(fi[1]);
			float thickness=atof(fi[2]);
			bool antialias=atoi(fi[3]);

			//Obtain Point Data

			fi.ReadLine(fd);
			assert(!feof(fd));
			assert(fi.Size()==2);
			assert(strcasecmp(fi[0],"D")==0);
			long dataPointsLen=atol(fi[1]);

			char* dataPoints=new char[dataPointsLen];
			fread(dataPoints,dataPointsLen,1,fd);
			float* dataPointsFloat=(float*)dataPoints;

			//Read blank line

			fi.ReadLine(fd);
			if(feof(fd))
			{
				//drawLog.txt can often end here.
				//It's not supposed to, but whatever.
				break;
			}
			assert(!feof(fd));
			assert(strlen(fi.GetLine())==0);
			
			//Obtain Color Data
			
			fi.ReadLine(fd);
			assert(!feof(fd));
			assert(fi.Size()==2);
			assert(strcasecmp(fi[0],"D")==0);
			long dataColorsLen=atol(fi[1]);

			char* dataColors=new char[dataColorsLen];
			fread(dataColors,dataColorsLen,1,fd);
			float* dataColorsFloat=(float*)dataColors;

			//Process Data

			LGL_DrawLineStripToScreen
			(
				dataPointsFloat,
				dataColorsFloat,
				pointCount,
				thickness,
				antialias
			);

			//Clean Up

			delete dataPoints;
			delete dataColors;
		}
		else if(strcasecmp(fi[0],"LGL_MouseCoords")==0)
		{
			//FIXME: Mouse ultimately doesn't register if a wiimote has been activated
			//We only care about this if there's no wiimotes. Otherwise, this is taken care of in "LGL_WiimoteMotion"
			if(firstWiimoteActive==false)
			{
				assert(fi.Size()==3);
				if
				(
					ParticleSystem[0]!=NULL &&
					ParticleSystemActive[0]
				)
				{
					ParticleSystem[0]->PosPrev=ParticleSystem[0]->Pos;
					ParticleSystem[0]->Pos.SetXY(atof(fi[1]),atof(fi[2]));
				}
			}
		}
		else if(strcasecmp(fi[0],"LGL_WiimotePointers")==0)
		{
			assert(fi.Size()==9);
			for(unsigned int a=0;a<8;a++)
			{
				ParticleSystemActive[a] = atoi(fi[a+1])==1;
				ParticleSystem[a]->ParticlesPerSecond = ParticleSystemActive[a] ? POINTER_PARTICLES_PER_SECOND : 0;
			}
		}
		else if(strcasecmp(fi[0],"LGL_WiimoteMotion")==0)
		{
			assert(fi.Size()>=5);
			int wiimoteIndex = atoi(fi[1]);
			float dTime = LGL_Max(1.0f/300.0f,atof(fi[2]));
			assert(((fi.Size()-3)%2)==0);
			unsigned int pointCount = (fi.Size()-3)/2;
			for(unsigned int p=0;p<pointCount;p++)
			{
				/*
				if(atof(fi[3+2*p+0]) < -5.0f)
				{
					//This pointer isn't pointing at the screen
					ParticleSystemActive[wiimoteIndex]=false;
					ParticleSystem[wiimoteIndex]->ParticlesPerSecond = 0;
				}
				else
				{
				*/
					ParticleSystemActive[wiimoteIndex]=true;
					firstWiimoteActive=true;
					ParticleSystem[wiimoteIndex]->PosPrev=ParticleSystem[wiimoteIndex]->Pos;
					ParticleSystem[wiimoteIndex]->Pos.SetXY(atof(fi[3+2*p+0]),atof(fi[3+2*p+1]));
					ParticleSystem[wiimoteIndex]->ParticlesPerSecond = ParticleSystemActive[wiimoteIndex] ? POINTER_PARTICLES_PER_SECOND : 0;
					ParticleSystemUpdatedThisFrame[wiimoteIndex]=true;
					ParticleSystem[wiimoteIndex]->NextFrame(dTime/pointCount);
				//}
			}
		}
		else if(strcasecmp(fi[0],"LGL_ViewPortScreen")==0)
		{
			assert(fi.Size()==5);
			if(visualsOnly==false && 0)
			{
				LGL_ViewPortScreen
				(
					atof(fi[1]),
					atof(fi[2]),
					atof(fi[3]),
					atof(fi[4])
				);
			}
		}
		else if(strcasecmp(fi[0],"LGL_SwapBuffers")==0)
		{
			assert(fi.Size()==2);
			videoTimePrev=videoTimeActual;
			videoTimeNow=atof(fi[1]);
			videoTimeActual = videoTimeNow*videoTimeScalar;

			double dTime=videoTimeActual-videoTimePrev;
			for(int a=0;a<8;a++)
			{
				if(ParticleSystem[a]!=NULL)
				{
					if(ParticleSystemUpdatedThisFrame[a]==false)
					{
						ParticleSystem[a]->NextFrame(dTime);
					}
					if(noPointers==false)
					{
						ParticleSystem[a]->Draw(OmniFader);
					}
					ParticleSystemUpdatedThisFrame[a]=false;
				}
			}

			//Take a screenshot before we swap buffers
			if(pixels!=NULL)
			{
				delete pixels;
			}
			pixels=(unsigned char*)malloc(3*ResX*ResY);
			if(pixels==NULL)
			{
				printf("LGL_ScreenShot(): Error! Unable to malloc() pixel buffer.\n");
				exit(-1);
			}

			glReadBuffer(GL_BACK_LEFT);
			glReadPixels
			(
				0, 0,
				ResX,ResY,
				GL_RGB, GL_UNSIGNED_BYTE,
				pixels
			);

			LGL_SwapBuffers();
			LGL_ProcessInput();
			if(LGL_KeyDown(SDLK_ESCAPE))
			{
				//Abort!
				exit(-1);
			}

			//Draw this frame however many times we need to catch up. Always draw at least once.
			bool frameDrawn=false;
			for(;;)
			{
				if
				(
					frameDrawn==false ||
					videoTimeActual>=videoTimeNext
				)
				{
					//Send this frame to mencoder. Maybe.
					if(mencoderFD)
					{
						videoSkipFrames--;
						if(videoSkipFrames<0)
						{
							videoSkipFrames=0;
							fwrite(pixels,3*ResX*ResY,1,mencoderFD);
						}
					}
					videoTimeNext+=videoTimeStep;
					frameDrawn=true;
				}
				else
				{
					break;
				}
			}
		}
		else if(strlen(fi[0])==0)
		{
			//Blank line
		}
		else if(strcasecmp(fi[0],"LGL_DrawLogStart")==0)
		{
			assert(fi.Size()==2);
			videoTimeNow=atof(fi[1]);
			videoTimeActual=atof(fi[1]);
		}
		else if(strcasecmp(fi[0],"!dvj::ParticleMouse")==0)
		{
			assert(fi.Size()==22);
			assert(ParticleSystem[0]==NULL);
			assert(ParticleImage==NULL);
			ParticleImage=new LGL_Image(fi[20]);
			for(int a=0;a<8;a++)
			{
				ParticleSystem[a]=new ParticleSystemObj
				(
					atof(fi[1]),atof(fi[2]),				//start x/y
					atof(fi[3]),atof(fi[4]),				//scale x/y
					atof(fi[5]),atof(fi[6]),				//radius begin / eng
					atof(fi[7]),atof(fi[8]),atof(fi[9]),atof(fi[10]),	//Begin Color
					atof(fi[11]),atof(fi[12]),atof(fi[13]),atof(fi[14]),	//End Color
					atof(fi[15]),atof(fi[16]),				//Life Min/Max
					atof(fi[17]),atof(fi[18]),				//Velocity Min/Max
					atof(fi[19]),						//Drag
					ParticleImage,
					atof(fi[21])						//Particles Per Second
				);
				ParticleSystem[a]->PosPrev.SetXY(0.5f,0.5f);
				ParticleSystem[a]->ParticlesPerSecond = 0;
			}
		}
		else if(strcasecmp(fi[0],"dttprehps")==0)
		{
			assert(fi.Size()==19);
			for(int a=0;a<18;a++)
			{
				homePointSeconds[a]=atof(fi[a+1]);
			}
		}
		else if(strcasecmp(fi[0],"dttpreflash")==0)
		{
			assert(fi.Size()==19);
			for(int a=0;a<18;a++)
			{
				homePointFlash[a]=atof(fi[a+1]);
			}
		}
		else if(strcasecmp(fi[0],"dttprenoise")==0)
		{
			assert(fi.Size()==19);
			for(int a=0;a<18;a++)
			{
				homePointNoise[a]=atof(fi[a+1]);
			}
		}
		else if(strcasecmp(fi[0],"dtt")==0)
		{
			assert(fi.Size()==38);
			
			int which = atoi(fi[1]);
			assert(which >=0 && which < 10);
			assert(soundList[which]);

			int entireWaveArrayCount=ENTIRE_WAVE_ARRAY_COUNT;
			float entireWaveArrayMagnitudeAve[ENTIRE_WAVE_ARRAY_COUNT];
			float entireWaveArrayMagnitudeMax[ENTIRE_WAVE_ARRAY_COUNT];
			float entireWaveArrayFreqFactor[ENTIRE_WAVE_ARRAY_COUNT];

			LoadWaveArrayData
			(
				soundList[which],
				entireWaveArrayMagnitudeAve,
				entireWaveArrayMagnitudeMax,
				entireWaveArrayFreqFactor
			);
			
			Turntable_DrawWaveform
			(
				soundList[which],		//01: sound
				fi[2][0] == 'T',		//02: loaded
				fi[3],				//03: video path short
				fi[4][0] == 'T',		//04: glitch
				atof(fi[5]),			//05: glitchBegin
				atof(fi[6]),			//06: glitchLength
				atof(fi[7]),			//07: soundPositionSamples
				atof(fi[8]),			//08: soundLengthSamples
				atof(fi[9]),			//09: soundSpeed
				atof(fi[10]),			//10: pitchBend
				atof(fi[11]),			//11: grainStreamCrossfader
				atof(fi[12]),			//12: grainStreamSourcePoint
				atof(fi[13]),			//13: grainStreamLength
				atof(fi[14]),			//14: grainStreamPitch
				atof(fi[15]),			//15: viewPortLeft
				atof(fi[16]),			//16: viewPortRight
				atof(fi[17]),			//17: viewPortBottom
				atof(fi[18]),			//18: viewPortTop
				atof(fi[19]),			//19: volumeMultiplierNow
				atof(fi[20]),			//20: centerX
				fi[21][0] == 'T',		//21: pause
				atof(fi[22]),			//22: nudge
				atof(fi[23]),			//23: Deprecated
				atof(fi[24]),			//24: SecondsSinceExecution
				homePointSeconds,		//25: homePointSeconds
				atoi(fi[25]),			//26: homePointIndex
				atoi(fi[26]),			//27: homePointIndexActual
				atol(fi[27]),			//28: homePointBitfield
				homePointNoise,			//29: homePointNoisePct
				homePointFlash,			//30: homePointFlashPct
				atof(fi[28]),			//31: bpm
				atof(fi[29]),			//32: bpmAdjusted
				atof(fi[30]),			//33: bpmFirstBeatSeconds
				atof(fi[31]),			//34: eq0
				atof(fi[32]),			//35: eq1
				atof(fi[33]),			//36: eq2
				false,//fi[34][0] == 'T'	//37: LowRez
				entireWaveArrayCount,		//38: entireWaveArrayFillIndex
				ENTIRE_WAVE_ARRAY_COUNT,	//39: ENTIRE_WAVE_ARRAY_COUNT
				entireWaveArrayMagnitudeAve,	//40: MagAve
				entireWaveArrayMagnitudeMax,	//41: MagMax
				entireWaveArrayFreqFactor,	//42: FreqFactor
				soundList[which]->GetLengthSeconds(),			//43: CachedLengthSeconds
				NoiseImage[rand()%NOISE_IMAGE_COUNT_256_64],		//44: NoiseImage
				atoi(fi[35]),			//45: VideoFrequencySensitiveMode
				atof(fi[36]),			//46: SoundWarpPointSecondsTrigger
				fi[37][0] == 'T'		//47: WaveformRecordHold
			);
		}
		else if(strcasecmp(fi[0],"MixF")==0)
		{
			if(fi.Size()!=5)
			{
				printf("MixF: fiSize() == %i != 5!\n",fi.Size());
				for(unsigned int a=0;a<fi.Size();a++)
				{
					printf("fi[%i]:\t'%s'\n",a,fi[a]);
				}
				assert(fi.Size()==5);
			}

			if(visualsOnly==false && ttLines)
			{
				crossFadeSliderLeft = atof(fi[1]);
				crossFadeSliderRight = atof(fi[2]);
				bool visualizerQuadrent = fi[3][0]=='T';
				float visualizerZoomOutPercent = atof(fi[4]);

				Mixer_DrawGlowLinesTurntables
				(
					videoTimeActual,
					crossFadeSliderLeft,
					crossFadeSliderRight,
					OmniFader,
					visualizerQuadrent,
					visualizerZoomOutPercent
				);
			}
		}
		else if(strcasecmp(fi[0],"MixL")==0)
		{
			assert(fi.Size()==7);
			Mixer_DrawLevels
			(
				0.0f,
				0.5f,
				atoi(fi[1]),
				atoi(fi[2]),
				atoi(fi[3]),
				atoi(fi[4]),
				fi[5][0]=='T',
				atof(fi[6])
			);
		}
		else if(strcasecmp(fi[0],"MixS")==0)
		{
			assert(fi.Size()==3);
			if(visualsOnly==false)
			{
				Mixer_DrawGlowLinesStatus
				(
					videoTimeActual,
					OmniFader,
					fi[1][0]=='T',
					atof(fi[2])
				);
			}
		}
		else if(strcasecmp(fi[0],"dvj::OmniFader")==0)
		{
			assert(fi.Size()==2);
			OmniFader = atof(fi[1]);
		}
		else if(strcasecmp(fi[0],"!dvj::NewSound")==0)
		{
			assert(fi.Size()==3);
			int which = atoi(fi[2]);
			assert(which >= 0 && which < 10);
			assert(soundList[which]==NULL);	//TODO: If this isn't so, just delete the old sound. It's okay.
printf("soundList[%i] = new LGL_Sound('%s')\n",which,fi[1]);
			soundList[which] = new LGL_Sound(fi[1]);
		}
		else if(strcasecmp(fi[0],"!dvj::DeleteSound")==0)
		{
			assert(fi.Size()==2);
			int which = atoi(fi[1]);
			assert(soundList[which]);
			soundList[which]->PrepareForDelete();
			for(;;)
			{
				if(soundList[which]->ReadyForDelete())
				{
					break;
				}
				LGL_DelayMS(1);
			}
			delete soundList[which];
			soundList[which]=NULL;
		}
		else if(strcasecmp(fi[0],"!dvj::NewVideo")==0)
		{
			assert(fi.Size()==2);

			//NOTE! Even if the video is already loaded, we must load a second one.
			//Why? Because the first DeleteVideo mustn't nuke all copies of the video from the videoList.

			printf("NewVideo: '%s'\n",fi[1]);
			if(fi[1][0]!='\0')
			{
				if(LGL_FileExists(fi[1])==false)
				{
					printf("NewVideo: Error! Video '%s' doesn't exist!\n",fi[1]);
					assert(LGL_FileExists(fi[1]));
				}

				if(vid==NULL)
				{
					vid = new LGL_Video(fi[1]);
				}
				else
				{
					vid->SetVideo(fi[1]);
				}
			}
		}
		else if(strcasecmp(fi[0],"!dvj::DeleteVideo")==0)
		{
			//Meh
			/*
			assert(fi.Size()==2);

			bool found=false;
			for(unsigned int a=0;a<videoList.size();a++)
			{
				if(strcmp(videoList[a]->GetPath(),fi[1])==0)
				{
					found=true;
					delete videoList[a];
					videoList[a]=NULL;
					videoList.erase
					(
						(std::vector<LGL_Video*>::iterator)(&(videoList[a]))
					);
					break;
				}
			}
			if(found==false)
			{
printf("!Luminescence::DeleteVideo|%s|ERROR\n",fi[1]);
				//assert(found);
			}
			*/
		}
		else if(strcasecmp(fi[0],"DirTreeDraw")==0)
		{
			assert(fi.Size()==20);

			float glow = atof(fi[1]);
			const char* filterText=fi[2];
			const char* filterDir=fi[3];
			const char* nameArray[5];
			for(int a=0;a<5;a++)
			{
				nameArray[a]=fi[4+a];
			}
			bool isDirBits[5];
			for(int a=0;a<5;a++)
			{
				isDirBits[a]=atoi(fi[9]) & (1<<a);
			}
			bool alreadyPlayedBits[5];
			for(int a=0;a<5;a++)
			{
				alreadyPlayedBits[a]=atoi(fi[10]) & (1<<a);
			}
			float bpm[5];
			for(int a=0;a<5;a++)
			{
				bpm[a]=atof(fi[16+a]);
			}

			Turntable_DrawDirTree
			(
				glow,			//01
				filterText,		//02
				filterDir,		//03
				nameArray,		//04-08
				isDirBits,		//09
				alreadyPlayedBits,	//10
				atoi(fi[11]),		//11
				atoi(fi[12]),		//12
				atof(fi[13]),		//13
				atof(fi[14]),		//14
				atof(fi[15]),		//15
				bpm			//16-19
			);
		}
		else if(strcasecmp(fi[0],"!DirTreePath")==0)
		{
			assert(fi.Size()==3);
			dirTrees[atoi(fi[1])].SetPath(NULL);
			dirTrees[atoi(fi[1])].SetPath(fi[2]);
		}
		else if(strcasecmp(fi[0],"!DirTreeFilter")==0)
		{
			assert(fi.Size()==3);
			dirTrees[atoi(fi[1])].SetFilterText(fi[2]);
		}
		else if(strcasecmp(fi[0],"!Luminescence::ParticlePointerActive")==0)
		{
			assert(fi.Size()==4);
			int which = atoi(fi[1]);
			if(ParticleSystemActive[which]==false)
			{
				ParticleSystemActive[which]=true;
				ParticleSystem[which]->Pos.SetXY(atof(fi[2]),atof(fi[3]));
				ParticleSystem[which]->PosPrev.SetXY(atof(fi[2]),atof(fi[3]));
			}
		}
		else if(strcasecmp(fi[0],"NoTTLines")==0)
		{
			assert(fi.Size()==1);
			ttLines=false;
		}
		else if(strcasecmp(fi[0],"dvj::MainDrawGlowLines")==0)
		{
			assert(fi.Size()==4);
			if(visualsOnly==false && ttLines)
			{
				bool visualsQuadrent = fi[1][0]=='T';
				float visualizerZoomOutPercent = atof(fi[2]);
				float visualizerRight = atof(fi[3]);
				Main_DrawGlowLines(videoTimeActual,OmniFader,visualsQuadrent,visualizerZoomOutPercent,visualizerRight);
			}
			ttLines=true;
		}
		else if(strcasecmp(fi[0],"!dvj::Record.mp3")==0)
		{
			assert(fi.Size()==2);
			if(LGL_FileExists("data/record/visualizer-audio-override.mp3"))
			{
				sprintf(audioFile,"data/record/visualizer-audio-override.mp3");
			}
			else
			{
				sprintf(audioFile,"%s",fi[1]);
			}
			char cmdInput[2048];
			sprintf(noExtensionFile,"%s",fi[1]);
			sprintf(noPathNoExtensionFile,"%s",&(fi[1][12]));
			noExtensionFile[strlen(noExtensionFile)-4]='\0';
			noPathNoExtensionFile[strlen(noPathNoExtensionFile)-4]='\0';
			if(strcasecmp(codec,"mpeg4")==0)
			{
				strcpy
				(
					cmdInput,
					"mencoder - -nosound -audiofile \"%s\" -oac copy -demuxer rawvideo -rawvideo fps=60:w=%i:h=%i:format=rgb24 -idx -flip -ovc lavc -lavcopts vcodec=mpeg4:mbd=2:trell=yes:v4mv=yes:vbitrate=%i:autoaspect=1:threads=1 -vf scale=%i:%i,harddup -noskip -of avi -o \"%s\""
				);
				sprintf(videoFile,"%s.avi",noExtensionFile);
			}
			else if(strcasecmp(codec,"h264")==0)
			{
				sprintf(videoFile,"%s.h264",noExtensionFile);
				strcpy
				(
					cmdInput,
					"mencoder - -nosound -audiofile \"%s\" -oac copy -demuxer rawvideo -rawvideo fps=60:w=%i:h=%i:format=rgb24 -idx -flip -ovc x264 -x264encopts subq=6:4x4mv:8x8dct:me=3:frameref=5:bframes=3:b_pyramid:weight_b:psnr:bitrate=%i -vf scale=%i:%i,harddup -noskip -of avi -o \"%s\""
				);
			}
			else if(strcasecmp(codec,"NULL")==0)
			{
				//Nothing to do here...
			}
			else
			{
				printf("Unknown codec '%s'! (Valid choices: 'mpeg4' (default) or 'h264')\n",codec);
				exit(0);
			}

			if(nullCodec==false)
			{
				//Fire up mencoder
				
				char cmd[1024];
				sprintf
				(
					cmd,
					cmdInput,
					audioFile,
					ResX,
					ResY,
					bitrateHQ,
					ResX,
					ResY,
					videoFile
				);
				mencoderFD=popen(cmd,"w");
				assert(mencoderFD);
			}

			sprintf(audioFile,"%s",fi[1]);
		}
		else if(strcasecmp(fi[0],"!LuminescenceTitleScreenAlpha")==0)
		{
			assert(fi.Size()==1);
			if(visualsOnly)
			{
				LGL_ViewPortScreen(0,1,0,1);
			}
		}
		else if(strcasecmp(fi[0],"!LuminescenceTitleScreenOmega")==0)
		{
			assert(fi.Size()==1);
			if(visualsOnly)
			{
				LGL_ViewPortScreen(0.0f,0.5f,0.5f,1.0f);
			}
		}
		else if(strcasecmp(fi[0],"!LuminescenceVideoLengthSeconds")==0)
		{
			assert(fi.Size()==2);
		}
		else if(strcasecmp(fi[0],"noop")==0)
		{
			//noop
		}
		else
		{
			printf("UNKNOWN!!\n");
			printf("\t'%s'\n",fi.GetLine());
			exit(-1);
		}
	}

	fclose(fd);

	if(LGL_FileExists("data/record/drawlog.txt.gz"))
	{
		LGL_FileDelete("data/record/drawlog.txt");
	}

	if(nullCodec)
	{
		printf("logDrawer: Success!\n");
		exit(0);
	}

	if(allFormats==false)
	{
		printf("logDrawer: Success!\n");
		exit(0);
	}

	LGL_ShutDown();

	char cmd[2048];
	char outputFile[2048];
	char target[2048];

	int minOutputFileSize = 1024 * 8;

	/*

	printf("Building index...\n");
	sprintf
	(
		cmd,
		"mencoder \"%s\" -o data/record/temp.avi -idx -ovc copy -oac copy",
		videoFile
	);
	printf("\t%s\n",cmd);
	system(cmd);
	printf("Checking results of command:\n\t%s\n",cmd);
	assert(LGL_FileExists("data/record/temp.avi") && LGL_FileLengthBytes("data/record/temp.avi") > minOutputFileSize);
	LGL_FileDelete(videoFile);
	LGL_FileDirMove("data/record/temp.avi",videoFile);
	printf("ogmmerge: Success! Created indexed output file: '%s'\n",videoFile);

#if 0
	printf("Running ogmmerge...\n");
	sprintf
	(
		cmd,
		"ogmmerge -o \"%s.ogm\" \"%s\" \"%s\"",
		noExtensionFile,
		videoFile,
		audioFile
	);
	printf("\t%s\n",cmd);
	system(cmd);
	printf("Checking results of command:\n\t%s\n",cmd);
	sprintf(outputFile,"%s.ogm",noExtensionFile);
	assert(LGL_FileExists(outputFile) && LGL_FileLengthBytes(outputFile) > minOutputFileSize);
	printf("ogmmerge: Success! Created output file: '%s.ogm'\n",noExtensionFile);
#endif
	printf("Running avimerge...\n");
	sprintf
	(
		cmd,
		"avimerge -o \"%s.avi\" -i \"%s\" -p \"%s\"",
		noExtensionFile,
		videoFile,
		audioFile
	);
	printf("\t%s\n",cmd);
	system(cmd);
	printf("Checking results of command:\n\t%s\n",cmd);
	sprintf(outputFile,"%s.avi",noExtensionFile);
	assert(LGL_FileExists(outputFile) && LGL_FileLengthBytes(outputFile) > minOutputFileSize);
	printf("avimerge: Success! Created output file: '%s.avi'\n",noExtensionFile);

	printf("Running lame to generate temporary lower bitrate mp3...\n");
	sprintf
	(
		cmd,
		"lame \"%s.mp3\" -h -b 192 \"%s.192.mp3\"",
		noExtensionFile,
		noExtensionFile
	);
	printf("\t%s\n",cmd);
	system(cmd);
	printf("Checking results of command:\n\t%s\n",cmd);
	sprintf(outputFile,"%s.192.mp3",noExtensionFile);
	assert(LGL_FileExists(outputFile) && LGL_FileLengthBytes(outputFile) > minOutputFileSize);
	printf("lame: Success! Created output file: '%s.192.mp3'\n",noExtensionFile);

	printf("Running avimerge to generate temporary avi with lower bitrate mp3...\n");
	sprintf
	(
		cmd,
		"avimerge -o \"%s.192.avi\" -i \"%s\" -p \"%s.192.mp3\"",
		noExtensionFile,
		videoFile,
		noExtensionFile
	);
	printf("\t%s\n",cmd);
	system(cmd);
	printf("Checking results of command:\n\t%s\n",cmd);
	sprintf(outputFile,"%s.192.avi",noExtensionFile);
	assert(LGL_FileExists(outputFile) && LGL_FileLengthBytes(outputFile) > minOutputFileSize);
	printf("avimerge: Success! Created output file: '%s.192.avi'\n",noExtensionFile);

	sprintf(target,"%s.192.mp3",noExtensionFile);
	LGL_FileDelete(target);
	*/

	if(allFormats)
	{

		if(LGL_DirectoryExists("data/record/dvd_root"))
		{
			LGL_DirectoryDelete("data/record/dvd_root");
		}
/*
		printf("Running ffmpeg to create temporary .mpeg4.avi file...\n");
		sprintf
		(
			cmd,
			"ffmpeg -i \"%s\" -vcodec mpeg4 -b %i \"%s.mpeg4.avi\"",
			videoFile,
			bitrateHQ,
			noExtensionFile
		);
		printf("\t%s\n",cmd);
		system(cmd);
		printf("Checking results of command:\n\t%s\n",cmd);
		sprintf(outputFile,"%s.mpeg4.avi",noExtensionFile);
		assert(LGL_FileExists(outputFile) && LGL_FileLengthBytes(outputFile) > minOutputFileSize);
		printf("ffmpeg: Success! Created output file: '%s.mpeg4.avi'\n",noExtensionFile);

		printf("Running mkvmerge...\n");
		sprintf
		(
			cmd,
			"mkvmerge -o \"%s.mkv\" \"%s.mpeg4.avi\" \"%s.mp3\"",
			noExtensionFile,
			noExtensionFile,
			noExtensionFile
		);
		printf("\t%s\n",cmd);
		system(cmd);
		printf("Checking results of command:\n\t%s\n",cmd);
		sprintf(outputFile,"%s.mkv",noExtensionFile);
		assert(LGL_FileExists(outputFile) && LGL_FileLengthBytes(outputFile) > minOutputFileSize);
		printf("mkvmerge: Success! Created output file: '%s.mkv'\n",noExtensionFile);

		sprintf(target,"%s.mpeg4.avi",noExtensionFile);
		LGL_FileDelete(target);
*/		

#if 0
		printf("Running ffmpeg for .mp4 conversion...\n");	//FIXME: We shouldn't be re-encoding video here...
		sprintf
		(
			cmd,
			"ffmpeg -i \"%s.avi\" -vcodec h264 -b %i -ab 320 \"%s.mp4\"",
			noExtensionFile,
			bitrateHQ,
			noExtensionFile
		);
		printf("\t%s\n",cmd);
		system(cmd);
		printf("Checking results of command:\n\t%s\n",cmd);
		sprintf(outputFile,"%s.mp4",noExtensionFile);
		assert(LGL_FileExists(outputFile) && LGL_FileLengthBytes(outputFile) > minOutputFileSize);
		printf("ffmpeg: Success! Created output file: '%s.mp4'\n",noExtensionFile);
#endif

#if 0	
		printf("Running ffmpeg to create .wmv file...\n");
		sprintf
		(
			cmd,
			"ffmpeg -i \"%s.192.avi\" -b %i \"%s.wmv\"",
			noExtensionFile,
			bitrateHQ,
			noExtensionFile
		);
		printf("\t%s\n",cmd);
		system(cmd);
		printf("Checking results of command:\n\t%s\n",cmd);
		sprintf(outputFile,"%s.wmv",noExtensionFile);
		assert(LGL_FileExists(outputFile) && LGL_FileLengthBytes(outputFile) > minOutputFileSize);
		printf("ffmpeg: Success! Created output file: '%s.wmv'\n",noExtensionFile);
#endif
		printf("Running ffmpeg -target ntsc-dvd...\n");
		sprintf
		(
			cmd,
			"ffmpeg -i \"%s.avi\" -target ntsc-dvd -b 8000 -maxrate 9000 -minrate 4000 -g 18 -aspect 16:9 \"%s.mpeg2\"",
			noExtensionFile,
			noExtensionFile
		);
		printf("\t%s\n",cmd);
		system(cmd);
		printf("Checking results of command:\n\t%s\n",cmd);
		sprintf(outputFile,"%s.mpeg2",noExtensionFile);
		assert(LGL_FileExists(outputFile) && LGL_FileLengthBytes(outputFile) > minOutputFileSize);
		printf("ffmpeg: Success! Created output file: '%s.mpeg2'\n",noExtensionFile);
		
		printf("Running dvdauthor...\n");
		sprintf
		(
			cmd,
			"dvdauthor -o data/record/dvd_root \"%s.mpeg2\" && dvdauthor -o data/record/dvd_root -T",
			noExtensionFile
		);
		printf("\t%s\n",cmd);
		system(cmd);
		printf("Checking results of command:\n\t%s\n",cmd);
		assert(LGL_DirectoryExists("data/record/dvd_root"));
		printf("dvdauthor: Success! Created interim directory: 'data/record/dvd_root'\n");

		printf("Running mkisofs...\n");
		sprintf
		(
			cmd,
			"mkisofs -dvd-video -V Luminescence -o \"%s.iso\" data/record/dvd_root",
			noExtensionFile
		);
		printf("\t%s\n",cmd);
		system(cmd);
		printf("Checking results of command:\n\t%s\n",cmd);
		sprintf(outputFile,"%s.iso",noExtensionFile);
		assert(LGL_FileExists(outputFile) && LGL_FileLengthBytes(outputFile) > minOutputFileSize);
		printf("mkisofs: Success! Created output file: '%s.iso'\n",noExtensionFile);

		if(LGL_DirectoryExists("data/record/dvd_root"))
		{
			LGL_DirectoryDelete("data/record/dvd_root");
		}

		//It would seem these .mov files don't work on Quicktime. So don't bother encoding them for now.
		/*
		printf("Running ffmpeg for .mov conversion...\n");	//FIXME: We shouldn't be re-encoding video here...
		sprintf
		(
			cmd,
			"ffmpeg -i \"%s.192.avi\" -vcodec h264 -b %i -s 800x600 -aspect 4:3 \"%s.mov\"",
			noExtensionFile,
			(int)(.75f*bitrateHQ),
			noExtensionFile
		);
		printf("\t%s\n",cmd);
		system(cmd);
		printf("Checking results of command:\n\t%s\n",cmd);
		sprintf(outputFile,"%s.mov",noExtensionFile);
		assert(LGL_FileExists(outputFile) && LGL_FileLengthBytes(outputFile) > minOutputFileSize);
		printf("ffmpeg: Success! Created output file: '%s.mov'\n",noExtensionFile);
		*/
#if 0
		printf("Running ffmpeg for .mpg conversion...\n");
		sprintf
		(
			cmd,
			"ffmpeg -i \"%s.192.avi\" -b %i -s 640x480 \"%s.mpg\"",
			noExtensionFile,
			bitrateHQ/2,
			noExtensionFile
		);
		printf("\t%s\n",cmd);
		system(cmd);
		printf("Checking results of command:\n\t%s\n",cmd);
		sprintf(outputFile,"%s.mpg",noExtensionFile);
		assert(LGL_FileExists(outputFile) && LGL_FileLengthBytes(outputFile) > minOutputFileSize);
		printf("ffmpeg: Success! Created output file: '%s.mpg'\n",noExtensionFile);
#endif	//0
	}

	printf("Running ffmpeg for .swf conversion...\n");
	sprintf
	(
		cmd,
		"ffmpeg -i \"%s.192.avi\" -vcodec flv -b 1000 -s 426x320 \"%s.swf\"",
		noExtensionFile,
		noExtensionFile
	);
	printf("\t%s\n",cmd);
	system(cmd);
	printf("Checking results of command:\n\t%s\n",cmd);
	sprintf(outputFile,"%s.swf",noExtensionFile);
	assert(LGL_FileExists(outputFile) && LGL_FileLengthBytes(outputFile) > minOutputFileSize);
	printf("ffmpeg: Success! Created output file: '%s.swf'\n",noExtensionFile);

	char htmlFileName[2048];
	sprintf(htmlFileName,"%s.html",noExtensionFile);
	FILE* htmlFile=fopen(htmlFileName,"w");
	assert(htmlFile);
	fprintf(htmlFile,"<html>\n");
	fprintf(htmlFile,"<title>%s</title>\n",noPathNoExtensionFile);
	fprintf(htmlFile,"<object width=\"425\" height=\"320\">\n");
	fprintf(htmlFile,"<param name=\"movie\" value=\"%s.swf\">\n",noPathNoExtensionFile);
	fprintf(htmlFile,"<embed src=\"%s.swf\" width=\"425\" height=\"320\">\n",noPathNoExtensionFile);
	fprintf(htmlFile,"</embed>\n");
	fprintf(htmlFile,"</object>\n");
	fprintf(htmlFile,"</html>\n");
	fclose(htmlFile);

	sprintf(target,"%s.192.avi",noExtensionFile);
	LGL_FileDelete(target);

	printf("logDrawer: Complete!\n");

	return(0);
}

