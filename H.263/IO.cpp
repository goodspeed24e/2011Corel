#include"stdafx.h"
#include"Global.h"

/**********************************************************************
 *
 *	Name:        FillImage
 *	Description:	fills Y, Cb and Cr of a PictImage struct
 *	
 *	Input:        pointer to raw image
 *        
 *	Returns:	pointer to filled PictImage
 *	Side effects:	allocates memory to PictImage
 *                      raw image is freed
 *
 *	Date: 940109	Author:	Karl.Lillevold@nta.no
 *
 ***********************************************************************/

PictImage *FillImage(unsigned char *in)
{
  PictImage *Pict;

  Pict = InitImage(pels*lines);

  memcpy(Pict->lum, in, pels*lines);
  memcpy(Pict->Cb, in + pels*lines, pels*lines/4);
  memcpy(Pict->Cr, in + pels*lines + pels*lines/4, pels*lines/4);

//  free(in);
  return(Pict);
}




/**********************************************************************
 *
 *	Name:        InitImage
 *	Description:	Allocates memory for structure of 4:2:0-image
 *	
 *	Input:	        image size
 *	Returns:	pointer to new structure
 *	Side effects:	memory allocated to structure
 *
 *
 ***********************************************************************/
PictImage *InitImage(int size)
{
  PictImage *newImage;

  if ((newImage = (PictImage *)malloc(sizeof(PictImage))) == NULL) {
    fprintf(stderr,"Couldn't allocate (PictImage *)\n");
    exit(-1);
  }
  if ((newImage->lum = (unsigned char *)malloc(sizeof(char)*size))
      == NULL) {
    fprintf(stderr,"Couldn't allocate memory for luminance\n");
    exit(-1);
  }
  if ((newImage->Cr = (unsigned char *)malloc(sizeof(char)*size/4))
      == NULL) {
    fprintf(stderr,"Couldn't allocate memory for Cr\n");
    exit(-1);
  }
  if ((newImage->Cb = (unsigned char *)malloc(sizeof(char)*size/4))
      == NULL) {
    fprintf(stderr,"Couldn't allocate memory for Cb\n");
    exit(-1);
  }

  return newImage;
}


/**********************************************************************
 *
 *	Name:        FreeImage
 *	Description:	Frees memory allocated to structure of 4:2:0-image
 *	
 *	Input:        pointer to structure
 *	Returns:
 *	Side effects:	memory of structure freed
 *
 *
 ***********************************************************************/

void FreeImage(PictImage *image)

{
  free(image->lum);
  free(image->Cr);
  free(image->Cb);
}

