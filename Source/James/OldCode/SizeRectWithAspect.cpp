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
