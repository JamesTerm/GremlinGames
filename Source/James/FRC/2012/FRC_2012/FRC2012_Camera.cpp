#include "WPILib.h"
#include "nivision.h"

#include "Base/Base_Includes.h"
#include <math.h>
#include <assert.h>
#include "Base/Vec2d.h"
#include "Base/Misc.h"
#include "Base/Event.h"
#include "Base/EventMap.h"
#include "Common/Entity_Properties.h"
#include "Common/Physics_1D.h"
#include "Common/Physics_2D.h"
#include "Common/Entity2D.h"
#include "Common/Goal.h"
#include "Common/Ship_1D.h"
#include "Common/Ship.h"
#include "Common/Vehicle_Drive.h"
#include "Common/PIDController.h"
#include "Common/Tank_Robot.h"
#include "Common/AI_Base_Controller.h"
#include "Common/Robot_Control_Interface.h"
#include "Common/Rotary_System.h"
#include "Base/Joystick.h"
#include "Base/JoystickBinder.h"
#include "Common/UI_Controller.h"
#include "Common/PIDController.h"
#include "FRC2011_Robot.h"
#include "FRC2012_Robot.h"
#include "FRC2012_Camera.h"
#include "InOut_Interface.h"
#include "FRC2012_Goals.h"

#if 0
//I found this code here... what luck!
//http://www.google.com/url?sa=t&rct=j&q=&esrc=s&frm=1&source=web&cd=2&sqi=2&ved=0CCQQFjAB&url=http%3A%2F%2Fchopshop-166.googlecode.com%2Fhg-history%2F9569eb980656e13f3219cda57a129679dc32fc60%2Fchopshop12%2FTarget2.h&ei=LyIqT-PXLuHosQK0wPGrDg&usg=AFQjCNHDicqEdUnYcP504dKcSzuqdW6aJA

typedef enum {
	UNINITIALIZED=0,
	TOP_MOST, 
	LEFT_MOST, 
	RIGHT_MOST,
	BOTTOM_MOST
} corner_t;

#define DPRINTF

//M's potential debug checker:
static bool FailCheck(int Returned, char* Description)
/*	Description: Prints out errors.
	Syntax to call:
		if(FailCheck(imaqFxn, "Describe function Failed")) return 0;
*/
{/* Error report logic */
	if(!Returned)
	{
		int errCode = GetLastVisionError();
		DPRINTF(LOG_INFO, Description);
		DPRINTF(LOG_INFO, "Error code: ", errCode);
		char *errString = GetVisionErrorText(errCode);
		DPRINTF(LOG_INFO, "errString= %s", errString);
		return true;
	}
	else
	{
		return false;
	}
}


int ProcessImage(Image* CameraInput, int DV, float* HeightOfTarget, float* WidthOfTarget)
/*	Description: This function takes an image and analyzes it for the target.
	Return Value:
		0: failed (seeing too many rectangles or none)
		1-4: sees 1-4 rect
*/
{
	//create destination/editable image

	Image* ProcessedImage = frcCreateImage(IMAQ_IMAGE_U8);

	/*Step 1: Color Threshold 
	(Red: 0-255, Green: 174-255,  Blue: 210-255)
	int imaqColorThreshold(Image* dest, const Image* source, int replaceValue, ColorMode mode, const Range* plane1Range, const Range* plane2Range, const Range* plane3Range); */
	/* Setup Threshold Values */
	Range RR, GR, BR;
		RR.minValue=0  ; RR.maxValue=255;
		GR.minValue=174; GR.maxValue=255;
		BR.minValue=210; BR.maxValue=255;
	FailCheck(imaqColorThreshold(ProcessedImage, CameraInput, 255, IMAQ_RGB, &RR, &GR, &BR), "Color Threshold Failed");

	/*Step 2: Basic Morphology Dilation
	int imaqMorphology(Image* dest, Image* source, MorphologyMethod method, const StructuringElement* structuringElement); */
	/* Setup for Morphology*/
		StructuringElement StructEle;
		StructEle.matrixCols = 3; StructEle.matrixRows = 3; StructEle.hexa = FALSE;
		StructEle.kernel = (int*) malloc(9 * sizeof(int));
		for(int s=0;s<9;s++) StructEle.kernel[s] = 1;
	imaqMorphology(ProcessedImage, ProcessedImage, IMAQ_DILATE, &StructEle);
	free(StructEle.kernel);

	/*Step 3: Fill Holes
	int imaqFillHoles(Image* dest, const Image* source, int connectivity8); */
	imaqFillHoles(ProcessedImage, ProcessedImage, TRUE); 

	/*Step 4: Particle Filter (Area: min.5%, max100% of image)
	int imaqParticleFilter4(Image* dest, Image* source, const ParticleFilterCriteria2* criteria, int criteriaCount, const ParticleFilterOptions2* options, const ROI* roi, int* numParticles); */
	/* Setup for Particle Filter */
	ParticleFilterCriteria2 CRIT;
		CRIT.parameter=IMAQ_MT_AREA_BY_IMAGE_AREA;
		CRIT.lower=0.5; CRIT.upper=100;
		CRIT.calibrated=FALSE; CRIT.exclude=FALSE;
	ParticleFilterOptions2 OPTS;
		OPTS.rejectMatches=FALSE; OPTS.rejectBorder=TRUE;
		OPTS.fillHoles=FALSE; OPTS.connectivity8=TRUE;
	int NP;
	imaqParticleFilter4(ProcessedImage, ProcessedImage, &CRIT, 1, &OPTS, NULL, &NP); 

	/*Step 5: Convert to 16-bit image, max contrast
	int imaqLookup(Image* dest, const Image* source, const short* table, const Image* mask); */
	/* Setup for Lookup Table */
	short TBL[256];
		TBL[0]=0;
		for(int i=1;i<257;i++) TBL[i]=255;
	imaqLookup(ProcessedImage, ProcessedImage, &TBL[0], NULL); 

	/* Step 6: Find Rectangles
	RectangleMatch* imaqDetectRectangles(const Image* image, const RectangleDescriptor* rectangleDescriptor, const CurveOptions* curveOptions, const ShapeDetectionOptions* shapeDetectionOptions, const ROI* roi, int* numMatchesReturned); */
	/* Setup for Rectangle Detection */
		RectangleDescriptor RCDS;
			RCDS.minWidth =0;RCDS.maxWidth =1000000;
			RCDS.minHeight=0;RCDS.maxHeight=1000000;
		CurveOptions CVOP;
			CVOP.extractionMode=IMAQ_UNIFORM_REGIONS;
			CVOP.threshold=1;
			CVOP.filterSize=IMAQ_NORMAL;
			CVOP.minLength=10;
			CVOP.rowStepSize=10;
			CVOP.columnStepSize=10;
			CVOP.maxEndPointGap=10;
			CVOP.onlyClosed=FALSE;
			CVOP.subpixelAccuracy=TRUE;//ignored
		ShapeDetectionOptions SHDTOP;
			SHDTOP.mode=IMAQ_GEOMETRIC_MATCH_SCALE_INVARIANT;
				RangeFloat ARF; ARF.minValue=-8; ARF.maxValue=8;
			SHDTOP.angleRanges=&ARF;
			SHDTOP.numAngleRanges=1;
				RangeFloat SRF; SRF.minValue=.25; SRF.maxValue=200;
			SHDTOP.scaleRange=SRF;
			SHDTOP.minMatchScore=400;
	int NumRect; //number of rectangles seen
	RectangleMatch* RectMPtr=imaqDetectRectangles(ProcessedImage, &RCDS, &CVOP, &SHDTOP, NULL, &NumRect);
	if(NumRect==0 || NumRect>4) return 0; 

	/* Step 7: Find desired Rectangle */ 
	/* Find Desired Value (DV) rectange's number logic */
		int BestDVRectPtr;
		float BestDVSoFar=0;
		switch (DV)//Find the rectangle with the desired value
		{
			case TOP_MOST:
				for (int i = 0; i < NumRect; i++) 
				{
					if ((RectMPtr+i)->corner[0].y < BestDVSoFar)
					{ BestDVSoFar = (RectMPtr+i)->corner[0].y; BestDVRectPtr = i; }
				}
			break;
			case LEFT_MOST:
				for (int i = 0; i < NumRect; i++) 
				{
					if ((RectMPtr+i)->corner[0].x < BestDVSoFar)
					{ BestDVSoFar = (RectMPtr+i)->corner[0].x; BestDVRectPtr = i; }
				}
			break;
			case RIGHT_MOST:
				for (int i = 0; i < NumRect; i++) 
				{
					if ((RectMPtr+i)->corner[0].x > BestDVSoFar)
					{ BestDVSoFar = (RectMPtr+i)->corner[0].x; BestDVRectPtr = i; }
				}
			case BOTTOM_MOST:
				for (int i = 0; i < NumRect; i++) 
				{
					if ((RectMPtr+i)->corner[0].y > BestDVSoFar)
					{ BestDVSoFar = (RectMPtr+i)->corner[0].y; BestDVRectPtr = i; }
				}
			break;
			default:
				printf("\n\nSome fool put a bad value into M's image processing program!!\n\n");
			break;
		}
		RectangleMatch* TargetRect = RectMPtr + BestDVRectPtr;
	
	
	/* Step 8: Take desired measurements and return number of rectangles seen*/
	/* Take measurements, return */
	*HeightOfTarget= ((TargetRect->corner[0].y + TargetRect->corner[1].y)/2);
	*WidthOfTarget= TargetRect->corner[0].x - TargetRect->corner[1].x;
	TPRINTF(LOG_INFO, "TargetRect:\n\t1: %f, %f\n\t2: %f, %f\n\t3: %f, %f\n\t4: %f, %f", 
			(double) TargetRect->corner[0].x,
			(double) TargetRect->corner[0].y,
			(double) TargetRect->corner[1].x,
			(double) TargetRect->corner[1].y,
			(double) TargetRect->corner[2].x,
			(double) TargetRect->corner[2].y,
			(double) TargetRect->corner[3].x,
			(double) TargetRect->corner[3].y
	);
	return NumRect;
	
	
	imaqDispose(RectMPtr);

}
#endif


  /***********************************************************************************************************************************/
 /*															FrameProcessing															*/
/***********************************************************************************************************************************/
void FrameProcessing::TestMinMax(MonoImage &image,BYTE &TestMin,BYTE &TestMax)
{
	TestMin=TestMax=0x80;
	MonoImage &cr=image;
	;
	for (int y=0; y<cr.GetHeight(); y++)
	{
		for (int x=0; x<cr.GetWidth(); x++)
		{
			//pixel_ycbcr_u8 pixel=cr(x,y);
			//BYTE color=PixelColor(pixel);
			//BYTE color=imaqGetPixel(cr,);
			BYTE color=1;
			if (color<TestMin)
				TestMin=color;
			if (color>TestMax)
				TestMax=color;
		}
	}
}

void FrameProcessing::operator()(HSLImage &image,TestChorminance color,double AspectRatio)
{
	MonoImage  *ColorPlane;
	if (color==eTestPr)
		ColorPlane = image.GetRedPlane();
	else 
		ColorPlane = image.GetBluePlane();

	#if 0
	{
		BYTE TestMin,TestMax;
		TestMinMax(*ColorPlane,TestMin,TestMax);
		printf("\r min=%d max=%d           ",TestMin,TestMax);
	}
	#endif
	//Try to use this!
	//imaqDetectRectangles();
}
  /***********************************************************************************************************************************/
 /*													FRC_2012_CameraProcessing														*/
/***********************************************************************************************************************************/

FRC_2012_CameraProcessing::FRC_2012_CameraProcessing() : m_Camera(NULL),m_LastTime(0.0)
{
	//Default will use the team ip 10.34.81.11  with 11
	m_Camera=&AxisCamera::GetInstance();
	//m_Camera->WriteResolution(AxisCamera::kResolution_320x240);
	m_Camera->WriteResolution(AxisCamera::kResolution_640x480);
	//m_Camera->WriteCompression(20);
	//m_Camera->WriteBrightness(0);
	
	switch(m_Camera->GetResolution())
	{
	case AxisCamera::kResolution_640x480:	m_Xres=640,m_Yres=480;	break;
	case AxisCamera::kResolution_640x360:	m_Xres=640,m_Yres=360;	break;
	case AxisCamera::kResolution_320x240:	m_Xres=320,m_Yres=240;	break;
	case AxisCamera::kResolution_160x120:	m_Xres=160,m_Yres=120;	break;
	}
}

FRC_2012_CameraProcessing::~FRC_2012_CameraProcessing()
{
	m_Camera=NULL;  //We don't own this, but I do wish to treat it like we do
}

void FRC_2012_CameraProcessing::CameraProcessing_TimeChange(double dTime_s)
{
	if (m_Camera->IsFreshImage())
	{
		double time=GetTime() - m_LastTime;
		//Throttle down the iterations to a reasonable workload
		if (time>=1001.0/30000.0)
		{
			m_LastTime=GetTime();
			printf("\rCamera Time %f           ",time);
			
			HSLImage *image=m_Camera->GetImage(); 
			m_FrameProcessor(*image);
			delete image;
		}
	}
}

