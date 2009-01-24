/*
 *
 * importMusicVideo
 *
 */

#include "LGL.module/LGL.h"

std::vector<char*> srcNames;
std::vector<char*> outNames;

char musicDir[1024];
char videoDir[1024];

bool ambient=false;
bool mellow=false;

int userDefinedWidth=0;
int userDefinedHeight=0;

char dvdDevice[1024];
void pickDVDTracks();
bool convertMusicVideo(char* src, char* dst, int index);
int mencoderThread(void* obj);
LGL_Semaphore* mencoderSemaphore;

int main(int argc, char** argv)
{
	//Verify there's at least one argument

	printf("\n");

	sprintf(musicDir,"../../data/music");
	if(LGL_DirectoryExists(musicDir)==false)
	{
		printf("Error! Directory '%s' must exist!\n",musicDir);
		exit(0);
	}
	sprintf(videoDir,"../../data/video/tracks");
	if(LGL_DirectoryExists(videoDir)==false)
	{
		printf("Error! Directory '%s' must exist!\n",videoDir);
		exit(0);
	}
	
	bool help=false;
	for(int a=1;a<argc;a++)
	{
		if(strcasecmp("--help",argv[a])==0)
		{
			help=true;
			break;
		}
	}

	if
	(
		argc<2 ||
		help
	)
	{
		printf("importMusicVideo usage:\n");
		printf("\timportMusicVideo input.avi\n");
		printf("\timportMusicVideo input.avi --width 640 --height 480\n");
		printf("\timportMusicVideo input.mp4 --ambient\n");
		printf("\timportMusicVideo input.mp4 --mellow\n");
		printf("\timportMusicVideo --dvd\n");
		printf("\timportMusicVideo --dvd --dvd-device ~/dvdrip/VIDEO_TS/\n");
		printf("\timportMusicVideo input1.mov --dvd-device ~/dvdrip/folder-with-vobs input2.mpg --dvd input3.ogm input4.mkv --ambient --mellow\n\n");
		exit(0);
	}
	
	dvdDevice[0]='\0';
	bool badArgs=false;
	for(int a=1;a<argc;a++)
	{
		if(strcasecmp("--dvd-device",argv[a])==0)
		{
			if(a+1>=argc)
			{
				printf("Error! --dvd-device requires an argument!\n");
				exit(0);
			}
			a++;
			strcpy(dvdDevice,argv[a]);
		}
		else if(strcasecmp("--ambient",argv[a])==0)
		{
			ambient=true;
		}
		else if(strcasecmp("--mellow",argv[a])==0)
		{
			ambient=true;
			mellow=true;
		}
		else if(strcasecmp("--dvd",argv[a])==0)
		{
			//Meh
		}
		else if(strcasecmp("--width",argv[a])==0)
		{
			if(a+1>=argc)
			{
				printf("Error! --width requires an argument!\n");
				exit(0);
			}
			a++;
			userDefinedWidth=atoi(argv[a]);
		}
		else if(strcasecmp("--height",argv[a])==0)
		{
			if(a+1>=argc)
			{
				printf("Error! --height requires an argument!\n");
				exit(0);
			}
			a++;
			userDefinedHeight=atoi(argv[a]);
		}
		else
		{
			if(LGL_FileExists(argv[a])==false)
			{
				printf("importMusicVideo: Error! File doesn't exist: %s\n",argv[a]);
				badArgs=true;
			}
		}
	}
	if((userDefinedWidth==0)!=(userDefinedHeight==0))
	{
		printf("Error! --width and --height arguments requires both!\n");
		badArgs=true;
	}
	if(badArgs)
	{
		printf("\n");
		exit(0);
	}

	if(ambient)
	{
		if(mellow)
		{
			sprintf(videoDir,"../../data/video/ambient/mellow");
		}
		else
		{
			sprintf(videoDir,"../../data/video/ambient");
		}
		if(LGL_DirectoryExists(videoDir)==false)
		{
			printf("Error! Directory 'luminescence/data/video/ambient' and 'luminescence/data/video/ambient/mellow' must exist!\n");
			exit(0);
		}
	}

	LGL_Init(1024,768,false,0,"importMusicVideo");
	mencoderSemaphore=new LGL_Semaphore("baka");

	bool dvdFound=false;
	for(int a=1;a<argc;a++)
	{
		if
		(
			dvdFound==false &&
			strcasecmp("--dvd",argv[a])==0
		)
		{
			//Let's rip a DVD!
			pickDVDTracks();
			dvdFound=true;
		}
		else if(strcasecmp("--dvd-device",argv[a])==0)
		{
			//Meh
			a++;
		}
		else if(strcasecmp("--ambient",argv[a])==0)
		{
			//Meh
		}
		else if(strcasecmp("--mellow",argv[a])==0)
		{
			//Meh
		}
		else if(strcasecmp("--width",argv[a])==0)
		{
			//Meh
			a++;
		}
		else if(strcasecmp("--height",argv[a])==0)
		{
			//Meh
			a++;
		}
		else
		{
			//Let's convert a file!
			if(LGL_FileExists(argv[a])==false)
			{
				printf("Error! File '%s' doesn't exist!\n",argv[a]);
				assert(LGL_FileExists(argv[a]));
			}

			char* neo=new char[1024];
			sprintf(neo,"\"%s\"",argv[a]);
			srcNames.push_back(neo);

			//Get Output Name
			char suggestion[1024];
			int firstCharIndex=0;
			int nullCharIndex=strlen(argv[a]);
			for(unsigned int z=0;z<strlen(argv[a]);z++)
			{
				if(argv[a][z]=='/')
				{
					firstCharIndex=z+1;
				}
				else if(argv[a][z]=='.')
				{
					nullCharIndex=z;
				}
			}
			char safe=argv[a][nullCharIndex];
			argv[a][nullCharIndex]='\0';
			strcpy(suggestion,&(argv[a][firstCharIndex]));
			argv[a][nullCharIndex]=safe;

			LGL_InputBuffer outputBuffer;
			outputBuffer.SetString(suggestion);
			outputBuffer.GrabFocus();
			bool firstKeyPressed=false;
			for(;;)
			{
				LGL_ProcessInput();
				LGL_GetFont().DrawString
				(
					.05,.3,.025,
					1,1,1,1,
					false,
					.75,
					"Enter Output Name (No Extension) For:"
				);
				LGL_GetFont().DrawString
				(
					.05,.25,.025,
					1,1,1,1,
					false,
					.75,
					"'%s'",
					argv[a]
				);
				LGL_GetFont().DrawString
				(
					.05,.20,.015,
					1,1,1,1,
					false,
					.75,
					"> %s",
					outputBuffer.GetString()
				);
				if(LGL_KeyStroke(SDLK_RETURN))
				{
					outputBuffer.ReleaseFocus();
					break;
				}

				if(!firstKeyPressed)
				{
					if(strlen(LGL_KeyStream())>0)
					{
						if(LGL_KeyStream()[0]==8)	//8==backspace
						{
							outputBuffer.SetString(NULL);
						}
						firstKeyPressed=true;
					}
				}

				if(LGL_KeyStroke(SDLK_ESCAPE)) exit(0);

				LGL_DelaySeconds(1.0f/60.0f-LGL_SecondsSinceThisFrame());	//Limit framerate to 60 fps
				LGL_SwapBuffers();
			}

			neo=new char[1024];
			strcpy(neo,outputBuffer.GetString());
			outNames.push_back(neo);
		}
	}

	LGL_SwapBuffers();

	for(unsigned int a=0;a<srcNames.size();a++)
	{
		/*
		LGL_ProcessInput();

		if(LGL_KeyStroke(SDLK_ESCAPE)) exit(0);

		LGL_GetFont().DrawString
		(
			0.5f,0.115f,0.02f,
			1,1,1,1,
			true,
			.75f,
			"Converting"
		);
		LGL_GetFont().DrawString
		(
			0.5f,0.065f,0.015f,
			1,1,1,1,
			true,
			.75f,
			outNames[a]
		);
		float pct=a/(float)srcNames.size();
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

		LGL_DelaySeconds(1.0f/60.0f-LGL_SecondsSinceThisFrame());	//Limit framerate to 60 fps
		LGL_SwapBuffers();
		*/

		while(convertMusicVideo(srcNames[a],outNames[a],a)==false)
		{
			for(;;)
			{
				LGL_ProcessInput();
				if(LGL_KeyStroke(SDLK_RETURN))
				{
					break;
				}
				LGL_GetFont().DrawString
				(
					.05,.25,.025,
					1,1,1,1,
					false,
					.75,
					"Press [Return] to retry."
				);
				LGL_GetFont().DrawString
				(
					0.49f,0.065f,0.02f,
					1,1,1,1,
					true,
					.75f,
					"Failed to convert '%s'",
					outNames[a]
				);

				if(LGL_KeyStroke(SDLK_ESCAPE)) exit(0);

				LGL_DelaySeconds(1.0f/60.0f-LGL_SecondsSinceThisFrame());	//Limit framerate to 60 fps
				LGL_SwapBuffers();
			}
			LGL_SwapBuffers();
		}
	}

	printf("\nimportMusicVideo: Success!\n");
	if(dvdFound) system("eject");

	return(0);
}

void
pickDVDTracks()
{
	//Get DVD Name
	LGL_InputBuffer dvdBuffer;
	dvdBuffer.GrabFocus();
	for(;;)
	{
		LGL_ProcessInput();
		if(LGL_KeyStroke(SDLK_RETURN))
		{
			dvdBuffer.ReleaseFocus();
			break;
		}
		LGL_GetFont().DrawString
		(
			.05,.25,.025,
			1,1,1,1,
			false,
			.75,
			"Enter DVD Name:"
		);
		LGL_GetFont().DrawString
		(
			.05,.20,.015,
			1,1,1,1,
			false,
			.75,
			"> %s",
			dvdBuffer.GetString()
		);

		if(LGL_KeyStroke(SDLK_ESCAPE)) exit(0);

		LGL_DelaySeconds(1.0f/60.0f-LGL_SecondsSinceThisFrame());	//Limit framerate to 60 fps
		LGL_SwapBuffers();
	}
	LGL_SwapBuffers();

	//Get title choices
	int trackNum=1;
	for(unsigned int titleNow=1;titleNow<=99;titleNow++)
	{
		char cmdTitle[1024];
		char extraArg[1024];
		extraArg[0]='\0';
		if(dvdDevice[0]!='\0')
		{
			sprintf(extraArg,"-dvd-device \"%s\"",dvdDevice);
		}
		sprintf(cmdTitle,"mplayer dvd://%i %s -loop 0 -vf pp=fd",titleNow,extraArg);
		printf("Running Command '%s'\n",cmdTitle);
		system(cmdTitle);

		bool doneWithTitles=false;
		for(;;)
		{
			LGL_ProcessInput();

			if(LGL_KeyStroke(SDLK_ESCAPE)) exit(0);
			if(LGL_KeyStroke(SDLK_r))
			{
				//Rip Title
				char* neo=new char[128];
				sprintf(neo,"dvd://%i",titleNow);
				srcNames.push_back(neo);

				//Get Title Name
				LGL_InputBuffer titleBuffer;
				titleBuffer.GrabFocus();
				for(;;)
				{
					LGL_ProcessInput();
					if(LGL_KeyStroke(SDLK_RETURN))
					{
						titleBuffer.ReleaseFocus();
						LGL_SwapBuffers();
						break;
					}
					LGL_GetFont().DrawString
					(
						.05,.25,.025,
						1,1,1,1,
						false,
						.75,
						"Enter Title %i Name:",
						titleNow
					);
					LGL_GetFont().DrawString
					(
						.05,.20,.015,
						1,1,1,1,
						false,
						.75,
						"> %s",
						titleBuffer.GetString()
					);

					if(LGL_KeyStroke(SDLK_ESCAPE)) exit(0);

					LGL_DelaySeconds(1.0f/60.0f-LGL_SecondsSinceThisFrame());	//Limit framerate to 60 fps
					LGL_SwapBuffers();
				}

				neo=new char[1024];
				if(strlen(titleBuffer.GetString())>0)
				{
					sprintf
					(
						neo,
						"%s (%02i) - %s",
						dvdBuffer.GetString(),
						trackNum,
						titleBuffer.GetString()
					);
				}
				else
				{
					sprintf
					(
						neo,
						"%s (%02i)",
						dvdBuffer.GetString(),
						trackNum
					);
				}
				outNames.push_back(neo);

				trackNum++;

				break;
			}
			else if(LGL_KeyStroke(SDLK_c))
			{
				//Rip individual chapters
				LGL_SwapBuffers();

				for(int chapterNow=1;chapterNow<=99;chapterNow++)
				{
					//Play the chapter
					char cmdChapter[1024];
					sprintf(cmdChapter,"%s -chapter %i-%i",cmdTitle,chapterNow,chapterNow);
					printf("Running Command '%s'\n",cmdChapter);
					system(cmdChapter);

					bool doneWithChapters=false;
					for(;;)
					{
						//Main loop, waiting for user's decision on this chapter
						LGL_ProcessInput();

						if(LGL_KeyStroke(SDLK_ESCAPE)) exit(0);

						if(LGL_KeyStroke(SDLK_r))
						{
							//Rip Chapter
							char* neo=new char[128];
							sprintf(neo,"dvd://%i -chapter %i-%i",titleNow,chapterNow,chapterNow);
							srcNames.push_back(neo);

							//Get Chapter Name
							LGL_InputBuffer chapterBuffer;
							chapterBuffer.GrabFocus();
							for(;;)
							{
								LGL_ProcessInput();
								if(LGL_KeyStroke(SDLK_RETURN))
								{
									chapterBuffer.ReleaseFocus();
									LGL_SwapBuffers();
									break;
								}
								LGL_GetFont().DrawString
								(
									.05,.25,.025,
									1,1,1,1,
									false,
									.75,
									"Enter Chapter %i Name:",
									chapterNow
								);
								LGL_GetFont().DrawString
								(
									.05,.20,.015,
									1,1,1,1,
									false,
									.75,
									"> %s",
									chapterBuffer.GetString()
								);

								if(LGL_KeyStroke(SDLK_ESCAPE)) exit(0);

								LGL_DelaySeconds(1.0f/60.0f-LGL_SecondsSinceThisFrame());	//Limit framerate to 60 fps
								LGL_SwapBuffers();
							}

							neo=new char[1024];
							if(strlen(chapterBuffer.GetString())>0)
							{
								sprintf
								(
									neo,
									"%s (%02i) - %s",
									dvdBuffer.GetString(),
									trackNum,
									chapterBuffer.GetString()
								);
							}
							else
							{
								sprintf
								(
									neo,
									"%s (%02i)",
									dvdBuffer.GetString(),
									trackNum
								);
							}
							outNames.push_back(neo);

							trackNum++;

							break;
						}
						else if(LGL_KeyStroke(SDLK_s))
						{
							//Skip Chapter
							LGL_SwapBuffers();
							break;
						}
						else if(LGL_KeyStroke(SDLK_n))
						{
							//Done With All Chapters
							doneWithChapters=true;
							break;
						}
						else if(LGL_KeyStroke(SDLK_d))
						{
							//Done With All Titles
							doneWithChapters=true;
							doneWithTitles=true;
							break;
						}

						LGL_GetFont().DrawString
						(
							.05,.75,.025,
							1,1,1,1,
							false,
							.75,
							"Chapter %i: [R]ip, [S]kip, [N]ext Title, [D]one",
							chapterNow
						);

						LGL_DelaySeconds(1.0f/60.0f-LGL_SecondsSinceThisFrame());	//Limit framerate to 60 fps
						LGL_SwapBuffers();
					}
					if(doneWithChapters)
					{
						break;
					}
				}
				LGL_SwapBuffers();
				break;
			}
			else if(LGL_KeyStroke(SDLK_s))
			{
				//Skip Title
				LGL_SwapBuffers();
				break;
			}
			else if(LGL_KeyStroke(SDLK_d))
			{
				//Done With All Titles
				doneWithTitles=true;
				break;
			}

			LGL_GetFont().DrawString
			(
				.05,.75,.025,
				1,1,1,1,
				false,
				.75,
				"Title %i: [R]ip, [C]hapters, [S]kip, [D]one",
				titleNow
			);

			LGL_DelaySeconds(1.0f/60.0f-LGL_SecondsSinceThisFrame());	//Limit framerate to 60 fps
			LGL_SwapBuffers();
		}
		if(doneWithTitles)
		{
			break;
		}
	}
	LGL_SwapBuffers();
}

bool convertMusicVideo
(
	char*	src,
	char*	dst,
	int	index
)
{
	//Define variables
	char cmd[1024];
	char videoOutputName[1024];
	sprintf
	(
		videoOutputName,
		"\"%s/%s.mp3.mjpeg.avi\"",
		videoDir,
		dst
	);
	char musicOutputName[1024];
	sprintf
	(
		musicOutputName,
		"\"%s/%s.mp3\"",
		musicDir,
		dst
	);
	
	char srcStr[1024];
	sprintf(srcStr,"%s",src);

	//Make the video
	int vBitrate=5000;
	char extraArg[1024];
	char zoomArg[1024];
	sprintf(zoomArg,"-vf harddup,scale,pp=fd -zoom -xy 512");
	extraArg[0]='\0';
	if(strstr(src,"dvd://"))
	{
		if(dvdDevice[0]!='\0')
		{
			sprintf(extraArg,"-dvd-device \"%s\"",dvdDevice);
		}
	}
	else if
	(
		userDefinedWidth>0 &&
		userDefinedHeight>0
	)
	{
		if(userDefinedWidth<=512)
		{
			zoomArg[0]='\0';
		}
		else
		{
			sprintf(zoomArg,"-vf harddup,scale,pp=fd -zoom -xy 512");
		}
	}
	else
	{
		char vidFile[2048];
		if(src[0]=='"')
		{
			strcpy(vidFile,&(src[1]));
			assert(vidFile[strlen(vidFile)-1]=='"');
			vidFile[strlen(vidFile)-1]='\0';
		}
		else
		{
			strcpy(vidFile,src);
		}
		if(LGL_FileExists(vidFile)==false)
		{
			printf("importMusicVideo: Error! File '%s' doesn't exist!\n",vidFile);
			return(false);
		}
		LGL_Video vid(vidFile);
		vid.SetPrimaryDecoder();
		vid.SetTime(1.0f);
		LGL_DelayMS(1000);
		LGL_Image* image=vid.LockImage();
		if(image==NULL)
		{
			printf("Error! Bad Video! '%s'\n",vidFile);
			return(false);
		}
		if(image->GetWidth()<=512)
		{
			sprintf(zoomArg,"-vf harddup,pp=fd");
		}
		else
		{
			sprintf(zoomArg,"-vf harddup,scale,pp=fd -zoom -xy 512");
		}
		vid.UnlockImage(image);
	}
	sprintf
	(
		cmd,
		"mencoder %s -o %s %s -ovc lavc -lavcopts vcodec=mjpeg:vbitrate=%i %s -oac mp3lame -lameopts preset=insane -af resample=44100:0:1",
		srcStr,
		videoOutputName,
		extraArg,
		vBitrate,
		zoomArg
	);
	printf("Running Command '%s'\n",cmd);
	if(mencoderSemaphore!=NULL)
	{
		delete mencoderSemaphore;
		mencoderSemaphore=NULL;
	}
	mencoderSemaphore=new LGL_Semaphore("baka");
	mencoderSemaphore->Lock("baka","baka");
	SDL_Thread* thread = LGL_ThreadCreate(mencoderThread,cmd);
	if(thread==NULL)
	{
		return(false);
	}
	else
	{
		for(;;)
		{
			if(mencoderSemaphore->IsLocked()==false)
			{
				int result=LGL_ThreadWait(thread);
				if(result!=0)
				{
					return(false);
				}
				break;
			}
			LGL_ProcessInput();

			LGL_GetFont().DrawString
			(
				0.5f,0.115f,0.02f,
				1,1,1,1,
				true,
				.75f,
				"Converting"
			);
			LGL_GetFont().DrawString
			(
				0.5f,0.065f,0.015f,
				1,1,1,1,
				true,
				.75f,
				dst
			);
			float pct=index/(float)srcNames.size();
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

			LGL_DelaySeconds(30.0f/60.0f);	//Limit framerate to 2 fps
			LGL_SwapBuffers();
		}
	}

	if(ambient)
	{
		return(true);
	}

	assert(videoOutputName[0]=='"');
	assert(videoOutputName[strlen(videoOutputName)-1]=='"');
	char testVideoFile[2048];
	strcpy(testVideoFile,&(videoOutputName[1]));
	testVideoFile[strlen(testVideoFile)-1]='\0';
	if(LGL_FileExists(testVideoFile)==false)
	{
		printf
		(
			"\nimportMusicVideo: Error! mplayer failed to create video output file '%s'\n\n",
			testVideoFile
		);
		printf("importMusicVideo: Failed command:\n");
		printf("\t%s\n\n",cmd);
		return(false);
	}

	//Dump the audio
	if(ambient==false)
	{
		sprintf
		(
			cmd,
			"mplayer %s -dumpaudio -dumpfile %s",
			videoOutputName,
			musicOutputName
		);
		printf("Running Command '%s'\n",cmd);
		system(cmd);
	}
	assert(musicOutputName[0]=='"');
	assert(musicOutputName[strlen(musicOutputName)-1]=='"');
	char testAudioFile[2048];
	strcpy(testAudioFile,&(musicOutputName[1]));
	testAudioFile[strlen(testAudioFile)-1]='\0';
	if(LGL_FileExists(testAudioFile)==false)
	{
		printf
		(
			"\nimportMusicVideo: Error! mplayer failed to create audio output file '%s'\n\n",
			testAudioFile
		);
		printf("importMusicVideo: Failed command:\n");
		printf("\t%s\n\n",cmd);
		return(false);
	}

	//Make a symlink
	char videoCrateDir[1024];
	sprintf(videoCrateDir,"%s/crates/video",musicDir);
	if(LGL_DirectoryExists(videoCrateDir))
	{
		char symlinkFilename[1024];
		sprintf
		(
			symlinkFilename,
			"\"%s/%s.mp3\"",
			videoCrateDir,
			dst
		);
		char testSymlinkFile[2048];
		sprintf
		(
			testSymlinkFile,
			"%s/%s.mp3",
			videoCrateDir,
			dst
		);
		if(LGL_FileExists(testSymlinkFile))
		{
			LGL_FileDelete(testSymlinkFile);
		}

		sprintf
		(
			cmd,
			"ln -s \"../../%s.mp3\" %s",
			dst,
			symlinkFilename
		);
		printf("Running Command '%s'\n",cmd);
		system(cmd);
	}

	return(true);
}

int
mencoderThread
(
	void*	obj
)
{
	char* cmd = (char*)obj;
	int ret = system(cmd);
	mencoderSemaphore->Unlock();
	return(ret);
}

