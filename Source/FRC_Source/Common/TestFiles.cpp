
struct FileStuff
{
	const char * const GroupID;
	const size_t NoSelections;
	size_t DefaultSelection;
	const char * const * const inputfile;
};
//Here is an easy way for us to share the same groups... just put your drive here:
#define __Drive__ "C:\\"

const size_t James_GGArt_NoSelections=8;
const char * const James_GGArt_Filelist[James_GGArt_NoSelections] = 
{
	"Sounds\\QuickBleep.wav",
	"Sounds\\lthrust.wav",
	"Sounds\\LoopTest.WAV",
	"Music\\TestTron2\\Tron2_PrisonIntro.MP3",
	"Music\\TestTron2\\Tron2_Prison.MP3",
	"Music\\TestTron2\\Tron2_PrimaryDigitization1.MP3",
	"Music\\TestTron2\\Tron2_PrimaryDigitization2.MP3",
	"Music\\STALKER.MID"
};
FileStuff James_GGArt_Group=
{
	"James_GGArt_Group",James_GGArt_NoSelections,1,James_GGArt_Filelist
};

const size_t James_WAV_NoSelections=5;
const char * const James_WAV_Filelist[James_WAV_NoSelections] = 
{
	//Length=1.06 8 bit unsigned PCM	Sample rate=11025 Channels=1
	__Drive__"Media\\Audio\\WAVE\\be_back.wav",
		//Length=838.35 (13:58:10) 16 bit	Sample rate=44100 Channels=2
		__Drive__"Media\\Audio\\WAVE\\Beethoven9thPart2.wav",
		//Length=4.79 8 bit unsigned PCM	Sample rate=24000 Channels=1
		__Drive__"Media\\Audio\\WAVE\\tadacrap.wav",
		//This is actually an embedded mp3
		//Length=160.60 (2:40:18)	mp3		Sample rate=12000 Channels 1
		__Drive__"Media\\Audio\\WAVE\\GoodBadUgly.wav",
		__Drive__"Media\\Audio\\WAVE\\DBTest\\0_16.wav"
};
FileStuff James_WAVGroup=
{
	"James_WAV_Group",James_WAV_NoSelections,0,James_WAV_Filelist
};

const size_t James_AVI_NoSelections=18;
const char * const James_AVI_Filelist[James_AVI_NoSelections] = 
{
	__Drive__"Media\\Video\\AVI\\Short0.avi",
	__Drive__"Media\\Video\\AVI\\Piano0.avi",
	//This one is 14minutes good for random seek stress test
	__Drive__"Media\\Video\\AVI\\2GBDV_DirectShow1.avi",
	//Tests the 80108010 bad blocks of audio (listen for my favorite things this year is going to the "golf-ball")
	__Drive__"Media\\Video\\AVI\\Justin.avi",
	//Actually 2 channel type 2... with 4 channel embedded in IAVS stream
	__Drive__"Media\\Video\\AVI\\DV4channel32k12bit.avi",
	//PAL DV- with a bad video frame
	__Drive__"Media\\Video\\AVI\\capture0000.avi",
	//16-9 clip? maybe
	__Drive__"Media\\Video\\AVI\\DV_002.AVI",
	__Drive__"Media\\Video\\AVI\\BorisFX.avi",
	__Drive__"Media\\Video\\AVI\\capture0000_dv.avi",
	//Here is a clip that has some dropped frames at 300 and 452
	__Drive__"Media\\Video\\AVI\\S007.avi",
	//This codec takes advantage of the extradata in the BMI structure (just like how palette clips do)
	__Drive__"Media\\Video\\AVI\\Huffy.avi",
	//uncompressed
	__Drive__"Media\\Video\\AVI\\UYVY.avi",
	//This one tests the avcodec_find_decoder() when it fails for video (this could change later)
	__Drive__"Media\\Video\\AVI\\capture.avi",
	__Drive__"Media\\Video\\AVI\\Leaves1_LG.avi",
	//This one tests the bounds of both chunk indexes by providing bogus data any anything exceeding
	__Drive__"Media\\Video\\AVI\\wahlstrom_intro.avi",
	//This one has 8 bit pcm audio
	__Drive__"Media\\Video\\AVI\\Video1.avi",
	//Audio only stream
	__Drive__"Media\\Video\\AVI\\BigAVI_AudioOnly.avi",
	//Video only stream
	__Drive__"Media\\Video\\AVI\\MavFlybyCarFade30b.avi"
};
FileStuff James_AVIGroup=
{
	"James_AVI_Group",James_AVI_NoSelections,0,James_AVI_Filelist
};

const size_t James_MPG_NoSelections=7;
const char * const James_MPG_Filelist[James_MPG_NoSelections] = 
{
	__Drive__"Media\\Video\\MPEG\\snow[1].mpeg",
		__Drive__"Media\\Video\\MPEG\\Britney - Toxic {smg}.mpg",
		__Drive__"Media\\Video\\MPEG\\AlienSong.mpeg",
		//FFmpeg is not picking up any video stream (Example code same results)
		__Drive__"Media\\Video\\MPEG\\mpeg1_plays_on_VT_not_SE.mpg",
		__Drive__"Media\\Video\\MPEG2\\PS\\tartwar.mpg",
		//This one has a drifting audio stream
		__Drive__"Media\\Video\\MPEG2\\PS\\The Victoria's Secret Fashion Show.mpg",
		__Drive__"Media\\Video\\m2t\\HD\\HDV-JVC_Footage\\Cap0002(0001).m2t"
};
FileStuff James_MPG_Group=
{
	"James_MPG_Group",James_MPG_NoSelections,4,James_MPG_Filelist
};

const size_t James_WMV_NoSelections=20;
const char * const James_WMV_Filelist[James_WMV_NoSelections] = 
{
	//FFmpeg WMV issues... audio does not seek
	__Drive__"Media\\Video\\WMV\\Picture 020.wmv",
	__Drive__"Media\\Video\\WMV\\Picture 001.wmv",
	//Audio only (for the wmv)
	__Drive__"Media\\Video\\WMV\\Run Dmc - Its Tricky.wma",
	__Drive__"Media\\Video\\WMV\\MVI_1346.wmv",
	__Drive__"Media\\Video\\WMV\\RevoDisco.wmv",
	__Drive__"Media\\Video\\WMV\\tartwar.wmv",
	__Drive__"Media\\Video\\WMV\\wc1hand-44_98.wmv",
	//Andrew created this... with window media encoder (this cannot seek)
	//It can only get duration... all other capabilities are not supported (e.g. cannot play after stop)
	__Drive__"Media\\Video\\WMV\\TCStudio Quick Tour.wmv",
	//These are Brian's clips:
	__Drive__"Media\\Video\\WMV\\mpeg4v3\\clip-cats.wmv",
	__Drive__"Media\\Video\\WMV\\vc-1\\FlightSimX_720p60.wmv",
	__Drive__"Media\\Video\\WMV\\vc-1\\TheNotebook-1080p.wmv",
	__Drive__"Media\\Video\\WMV\\wmv7\\clip-itsgonnarain.wmv",
	__Drive__"Media\\Video\\WMV\\wmv7\\clip-poona4.wmv",
	__Drive__"Media\\Video\\WMV\\wmv9\\WMVHD-720p-AdrenalineRush.wmv",
	__Drive__"Media\\Video\\WMV\\wmv9\\WMVHD-720p-CBS.wmv",
	__Drive__"Media\\Video\\WMV\\wmv9\\WMVHD-720p-CoralReefAdventure.wmv",
	__Drive__"Media\\Video\\WMV\\wmv9\\WMVHD-720p-FighterPilot.wmv",
	__Drive__"Media\\Video\\WMV\\wmv9\\WMVHD-720p-MysteryOfTheNile.wmv",
	__Drive__"Media\\Video\\WMV\\wmv9\\WMVHD-720p-T2.wmv",
	__Drive__"Media\\Video\\WMV\\wmv9\\WMVHD-1080p-CoralReefAdventure.wmv"
};
FileStuff James_WMV_Group=
{
	"James_WMV_Group",James_WMV_NoSelections,0,James_WMV_Filelist
};

const size_t James_Other_NoSelections=10;
const char * const James_Other_Filelist[James_Other_NoSelections] = 
{
	__Drive__"Media\\Video\\MOV\\Wassup_SF[1].mov",
	__Drive__"Media\\Video\\MOV\\zapruder_stable.mov",
	__Drive__"Media\\Video\\MOV\\Dude.mov",	
	//Apple BMP video codec,  1920x1080... This tests the upper limit of the filereader-- AspectTest
	//Audio Codec: MACE 3:1
	__Drive__"Media\\Video\\MOV\\WontPlay\\warthog.mov",
	__Drive__"Media\\Video\\MOV\\WontPlay\\HVXbamboo1080i.mov",
	__Drive__"Media\\Video\\MOV\\WontPlay\\ep3.mov",
	__Drive__"Media\\Video\\MOV\\WontPlay\\warthog_revisited.mov",
	//Contains AAC audio which is not currently supported
	__Drive__"Media\\Video\\MOV\\WontPlay\\crankygeeks.015.mp4",
	__Drive__"Media\\Video\\Others\\RenderTest004.flv",
	__Drive__"Media\\Video\\Others\\ShortFlash.swf"
};
FileStuff James_Other_Group=
{
	"James_Other_Group",James_Other_NoSelections,1,James_Other_Filelist
};


const size_t c_NoGroups=6;
FileStuff * const c_Groups[c_NoGroups]=
{
	&James_GGArt_Group,
	&James_WAVGroup,
	&James_AVIGroup,
	&James_MPG_Group,
	&James_WMV_Group,
	&James_Other_Group,
};

FileStuff *g_FileStuff=c_Groups[0];

const char *c_outpath = "C:\\Temp\\";
