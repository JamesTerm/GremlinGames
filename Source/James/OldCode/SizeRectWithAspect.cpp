RECT SizeRectWithAspectRatio(const RECT &rcScreen, SIZE sizePicture, bool bCenter)
{
	RECT rect=rcScreen;
	const double dWidth = rcScreen.right-rcScreen.left;
	const double dHeight = rcScreen.bottom-rcScreen.top;
	const double dAspectRatio = dWidth/dHeight;

	double dPictureWidth = sizePicture.cx;
	double dPictureHeight = sizePicture.cy;
	double dPictureAspectRatio = dPictureWidth/dPictureHeight;

	if (dPictureAspectRatio > dAspectRatio)
	{
		double dNewHeight = (dWidth/dPictureWidth)*dPictureHeight;
		int nCenteringFactor = 0;
		if (bCenter)
			nCenteringFactor = (int)((dHeight - dNewHeight) / 2.0);
		rect.top+=nCenteringFactor;
		rect.bottom=rect.top+ (int)dNewHeight + nCenteringFactor;
	}
	else if (dPictureAspectRatio < dAspectRatio)
	{
		double dNewWidth =  ((dHeight/dPictureHeight)*dPictureWidth);
		int nCenteringFactor = 0;
		if (bCenter)
			nCenteringFactor = (int)((dWidth - dNewWidth) / 2.0);
		rect.left+=nCenteringFactor;
		rect.right=rect.left + (int)dNewWidth + nCenteringFactor;
	}

	return rect;
}

RECT SizeRectWithAspectRatio(double AspectRatio,const RECT &CurrentWindow)
{
	RECT ret=CurrentWindow;
	double width=CurrentWindow.right-CurrentWindow.left;
	double height=CurrentWindow.bottom-CurrentWindow.top;
	double newAr = (width / height); // the current aspect ratio

	if (newAr > AspectRatio)		// fit to width.
		height = (int) (width / AspectRatio);
	else	// fit to height
		width = (int) (height * AspectRatio);

	//Now to translate back... we can do centering mode here if we wanted
	ret.right=ret.left+(int)width;
	ret.bottom=ret.top+(int)height;
	return ret;
}
