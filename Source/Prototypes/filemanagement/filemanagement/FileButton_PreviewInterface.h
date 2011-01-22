#ifndef __FileButton_PreviewInterface__
#define __FileButton_PreviewInterface__

//***************************************************************************************************************************************************
class FileButton_PreviewInterface
{	public:			virtual bool GeneratePreview(	const char *FileToRead,
													const char *FileToWrite,

													unsigned DestinationXRes,
													unsigned DestinationYRes,

													unsigned DeferredID1_Success,
													unsigned DeferredID2_Success,

													unsigned DeferredID1_Failure,
													unsigned DeferredID2_Failure,

													char *ImageToOverlay,

													BaseWindowClass *BWC,

													bool DelayAllCurrentIconsBeingBuild
													)=0;

					virtual void RemovePreview(		const char *FileToRead,
													BaseWindowClass *BWC)=0;
};

//***************************************************************************************************************************************************
class FileButton_MouseOverPreviewInterface
{	public:			virtual bool GeneratePreview(	const char *FileName,
													BaseWindowClass *BWC_Parent,
													RECT RectWithinParent)=0;
					virtual bool RemoveAllPreviews(BaseWindowClass *BWC_Parent)=0;
};

#endif __FileButton_PreviewInterface__