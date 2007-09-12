#if defined(__APPLE__) || defined(MACOSX)
    #include <GLUT/glut.h>
    #include <OpenGL/glext.h>
#else
    #include <GL/glut.h>
    #include <GL/glext.h>
#endif
#include "cmd/unit_generic.h"

#include "vsfilesystem.h"
#include "vsimage.h"
#include "vs_globals.h"
#include <png.h>
#include "posh.h"
#ifndef png_jmpbuf
#  define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

#if defined( _WIN32) && !defined( __CYGWIN__)
#ifndef HAVE_BOOLEAN
#define HAVE_BOOLEAN
#define FALSE 0
#define TRUE 1
typedef unsigned char boolean;
#endif
#ifndef XMD_H
#define XMD_H
typedef int INT32;
#endif
#endif

#if defined(__CYGWIN__)
#define XMD_H
#endif

#include "gfx/jpeg_memory.h"



#undef VSIMAGE_DEBUG

#ifdef _DEBUG
//#define VSIMAGE_FAILURE(code) VSExit(code)
#define VSIMAGE_FAILURE(code,file) VSFileSystem::vs_fprintf(stderr,"VSImage FAILURE! - trying graceful recovery... (while reading \"%s\")\n",file)
#else
#define VSIMAGE_FAILURE(code,file) VSFileSystem::vs_fprintf(stderr,"VSImage FAILURE! - trying graceful recovery... (while reading \"%s\")\n",file)
#endif

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
using VSFileSystem::VSError;
using VSFileSystem::BadFormat;
using VSFileSystem::VSFileType;
//using VSFileSystem::img_file;
int PNG_HAS_PALETTE =1;
int PNG_HAS_COLOR=2;
int PNG_HAS_ALPHA=4;

LOCALCONST_DEF(VSImage, int,SIZEOF_BITMAPFILEHEADER,sizeof(WORD)+sizeof(DWORD)+sizeof(WORD)+sizeof(WORD)+sizeof(DWORD))
LOCALCONST_DEF(VSImage, int,SIZEOF_BITMAPINFOHEADER,sizeof(DWORD)+sizeof(LONG)+sizeof(LONG)+2*sizeof(WORD)+2*sizeof(DWORD)+2*sizeof(LONG)+2*sizeof(DWORD))
LOCALCONST_DEF(VSImage, int,SIZEOF_RGBQUAD,sizeof(BYTE)*4)

VSImage::VSImage()
{
  this->img_depth=8;
  this->img_color_type=8;
  this->sizeY=1;
  this->sizeX=1;
  this->Init();
  this->mode=_24BIT;
}

void	VSImage::Init()
{
	this->tt = NULL;
	this->img_type = Unrecognized;
	//this->tex->palette = NULL;
	this->strip_16 = false;
}

void	VSImage::Init( VSFile * f, textureTransform *t, bool strip, VSFile *f2)
{
	assert( f!=NULL);

	this->Init();

	this->img_file = f;
	this->img_file2 = f2;
	this->tt = t;
	this->strip_16 = strip;
}

VSImage::VSImage( VSFile * f, textureTransform * t, bool strip, VSFile * f2)
{
	this->mode = _24BIT;
	this->Init( f, t, strip, f2);
}

VSImage::~VSImage()
{
	// img_file? and tt should not be deleted since they are passed as args to the class
}

unsigned char *	VSImage::ReadImage( VSFile * f, textureTransform * t, bool strip, VSFile * f2)
{
    try {

	this->Init( f, t, strip, f2);

	unsigned char * ret = NULL;
	CheckFormat( img_file);
	switch( this->img_type)
	{
		case PngImage :
			ret = this->ReadPNG();
		break;
		case JpegImage :
			ret = this->ReadJPEG();
		break;
		case BmpImage :
			ret = this->ReadBMP();
		break;
		case DdsImage :
			ret = this->ReadDDS();
		break;
		default :
			this->img_type = Unrecognized;
			cerr<<"::VSImage ERROR : Unknown image format"<<endl;
			VSIMAGE_FAILURE(1,img_file->GetFilename().c_str());
			ret = NULL;
	}
	return ret;

    } catch(...) {
        // ReadXXX() already handles exceptions. But, if any exception remains unhandled,
        // this handler will perform a dirty abortion (reclaims no memory, but doesn't crash at least)
        return NULL;
    }
}

VSError	VSImage::CheckPNGSignature( VSFile * file)
{
	VSError ret = Ok;

	// Do standard reading of the file
	unsigned char sig[8];
	file->Begin();
	file->Read(sig, 8);
	if (!png_check_sig(sig, 8))
		ret = BadFormat;
	
	return ret;
}

VSError	VSImage::CheckJPEGSignature( VSFile * file)
{
	VSError ret = Ok;

	// First 4 aren't known to me
	// Next 2 bytes is length
	// Next 5 are JFIF\0
	// Next 2 are version numbers
	char sig[13];
	file->Begin();
	file->Read( sig, 13);
	/*
	for( int i=0; i<13; i++)
		cerr<<sig[i]<<" ";
	cerr<<endl;
	*/
	if( strncmp( sig+6, "JFIF", 4))
		ret = BadFormat;
	file->Begin();
	return ret;
}

VSError	VSImage::CheckBMPSignature( VSFile * file)
{
	VSError ret = Ok;

	char head1;
	char head2;
	file->Begin();
	file->Read(&head1, 1);
	file->Read(&head2,1);
	if (toupper(head1)!='B'||toupper (head2)!='M')
		ret = BadFormat;
	file->Begin();

	return ret;
}

VSError VSImage::CheckDDSSignature(VSFile * file)
{
	VSError ret = Ok;
	char ddsfile[5];
	file->Begin();
	file->Read(&ddsfile,4);
	if(strncmp(ddsfile,"DDS ",4) != 0)
		ret = BadFormat;
	file->Begin();
	return(ret);
}

void	VSImage::CheckFormat( VSFile * file)
{
	if( this->CheckPNGSignature(file)==Ok)
	{
	#ifdef VSIMAGE_DEBUG
		cerr<<"\tFound a PNG file"<<endl;
	#endif
		this->img_type = PngImage;
		return;
	}
	if( this->CheckBMPSignature(file)==Ok)
	{
	#ifdef VSIMAGE_DEBUG
		cerr<<"\tFound a BMP file"<<endl;
	#endif
		this->img_type = BmpImage;
		return;
	}
	if( this->CheckJPEGSignature(file)==Ok)
	{
	#ifdef VSIMAGE_DEBUG
		cerr<<"\tFound a JPEG file"<<endl;
	#endif
		this->img_type = JpegImage;
		return;
	}
	if( this->CheckDDSSignature(file) == Ok)
	{
	#ifdef VSIMAGE_DEBUG
		cerr<<"\tFound a DDS file"<<endl;
	#endif
		this->img_type = DdsImage;
		return;
	}	
}


void PngReadFunc(png_struct *Png, png_bytep buf, png_size_t size)
{
#ifdef VSIMAGE_DEBUG
	cerr<<"PNG DEBUG : preparing to copy "<<size<<" bytes from PngFileBuffer"<<endl;
#endif
    TPngFileBuffer *PngFileBuffer=(TPngFileBuffer*)png_get_io_ptr(Png);
    memcpy(buf,PngFileBuffer->Buffer+PngFileBuffer->Pos,size);
    PngFileBuffer->Pos+=size;
}

/* We can't write in volumes yet so this is useless now
void PngWriteFunc(png_struct *Png, png_bytep buf, png_size_t size)
{
	cerr<<"PNG DEBUG : preparing to write "<<size<<" bytes from PngFileBuffer"<<endl;
}
*/

static void
png_cexcept_error(png_structp png_ptr, png_const_charp msg)
{
   if(png_ptr)
     ;
#ifndef PNG_NO_CONSOLE_IO
   VSFileSystem::vs_fprintf(stderr, "libpng error: %s\n", msg);
#endif

}

unsigned char *	VSImage::ReadPNG()
{
	png_bytepp row_pointers=NULL;
    unsigned char * image=NULL;

    try {

	TPngFileBuffer	PngFileBuffer = {NULL,0};
	palette = NULL;
	png_structp png_ptr;
	png_infop info_ptr;
	int  interlace_type;

	img_file->Begin();
	if( !CheckPNGSignature( img_file))
	{
		cerr<<"VSImage::ReadPNG() ERROR : NOT A PNG FILE"<<endl;
		VSIMAGE_FAILURE(1,img_file->GetFilename().c_str());
		throw(1);
	}
	// Go after sig since we already checked it
	// Only when reading from a buffer otherwise CheckPNGSignature already did the work
	if( img_file->UseVolume())
	{
		PngFileBuffer.Buffer = img_file->get_pk3_data();
		PngFileBuffer.Pos = 8;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, (png_error_ptr)png_cexcept_error, (png_error_ptr)NULL);
	if (png_ptr == NULL)
	{
		VSIMAGE_FAILURE(1,img_file->GetFilename().c_str());
		throw(1);
	}
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		cerr<<"VSImage ERROR : PNG info_ptr == NULL !!!"<<endl;
		VSIMAGE_FAILURE(1,img_file->GetFilename().c_str());
		throw(1);
	}
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		/* Free all of the memory associated with the png_ptr and info_ptr */
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		/* If we get here, we had a problem reading the file */
		cerr<<"VSImage ERROR : problem reading file/buffer -> setjmp !!!"<<endl;
		VSIMAGE_FAILURE(1,img_file->GetFilename().c_str());
		throw(1);
	}

	if( !img_file->UseVolume())
	{
		png_init_io(png_ptr, img_file->GetFP());
	}
	else
	{
		png_set_read_fn (png_ptr,(png_voidp)&PngFileBuffer,(png_rw_ptr)PngReadFunc);
	}

	png_set_sig_bytes(png_ptr, 8);
#ifdef VSIMAGE_DEBUG
	   VSFileSystem::vs_fprintf (stderr,"Loading Done. Decompressing\n");
#endif
	png_read_info(png_ptr, info_ptr);  /* read all PNG info up to image data */
	this->sizeX=1;this->sizeY=1;this->img_depth=8;png_get_IHDR(png_ptr, info_ptr, (png_uint_32 *)&this->sizeX, (png_uint_32 *)&this->sizeY, &this->img_depth, &this->img_color_type, &interlace_type, NULL, NULL);
#ifdef VSIMAGE_DEBUG
	cerr<<"1. Loading a PNG file : width="<<sizeX<<", height="<<sizeY<<", depth="<<img_depth<<", img_color="<<img_color_type<<", interlace="<<interlace_type<<endl;
#endif
	# if __BYTE_ORDER != __BIG_ENDIAN
	if (this->img_depth==16)
		png_set_swap (png_ptr);
	#endif

	if (this->img_depth==16&&strip_16)
		png_set_strip_16(png_ptr);
	if (strip_16&&this->img_color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png_ptr);
		   
	if (this->img_color_type == PNG_COLOR_TYPE_GRAY && this->img_depth < 8)
		png_set_gray_1_2_4_to_8(png_ptr);

	png_set_expand (png_ptr);
	png_read_update_info (png_ptr,info_ptr);
	this->sizeX=1;this->sizeY=1;this->img_depth=8;png_get_IHDR(png_ptr, info_ptr, (png_uint_32 *)&this->sizeX, (png_uint_32 *)&this->sizeY, &this->img_depth, &this->img_color_type, &interlace_type, NULL, NULL);
#ifdef VSIMAGE_DEBUG
	cerr<<"2. Loading a PNG file : width="<<sizeX<<", height="<<sizeY<<", depth="<<img_depth<<", img_color="<<img_color_type<<", interlace="<<interlace_type<<endl;
#endif
        if (img_depth!=16)
          img_depth=8;
	row_pointers = (unsigned char **)malloc (sizeof (unsigned char *) *this->sizeY);
	int numchan=1;
	if (this->img_color_type&PNG_COLOR_MASK_COLOR)
		numchan =3;
	if (this->img_color_type &PNG_COLOR_MASK_PALETTE)
		numchan =1;
	if (this->img_color_type&PNG_COLOR_MASK_ALPHA)
		numchan++;
	unsigned long stride = numchan*sizeof (unsigned char)*this->img_depth/8;
#ifdef VSIMAGE_DEBUG
	cerr<<"3. Allocating image buffer of size="<<(stride*sizeX*sizeY)<<endl;
#endif
	image = (unsigned char *) malloc (stride*this->sizeX*this->sizeY);
	for (unsigned int i=0;i<this->sizeY;i++)
	{
		row_pointers[i] = &image[i*stride*this->sizeX];
	}
	png_read_image (png_ptr,row_pointers);
	unsigned char * result;
	if (tt) {
#ifdef VSIMAGE_DEBUG
		cerr<<"4. Doing a tranformation"<<endl;
#endif
		result = (*tt) (this->img_depth,this->img_color_type,this->sizeX,this->sizeY,row_pointers);
		free (image); image=NULL;
	}
	else
	{
		result = image;
	}
	free (row_pointers); row_pointers=NULL;
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL); png_ptr=NULL; info_ptr=NULL;
#ifdef VSIMAGE_DEBUG
	   VSFileSystem::vs_fprintf (stderr,"Decompressing Done.\n");
#endif
	if(numchan == 1)
		mode=_PNG8BIT;
	else if (numchan == 3)
		mode=_PNG24BIT;
	else 
		mode=_PNG32BIT;		
	if( result)
		this->AllocatePalette();
	return result;

    } catch(...) {
        if (image) free(image); image=NULL;
        if (row_pointers) free(row_pointers); row_pointers=NULL;
        return NULL;
    }
}

struct my_error_mgr
{
  struct jpeg_error_mgr pub;// "public" fields
  jmp_buf setjmp_buffer;      // for return to caller
};

METHODDEF(void) my_error_exit(j_common_ptr cinfo)
{
  // cinfo->err really points to a my_error_mgr struct, so coerce pointer
  my_error_mgr * myerr = (my_error_mgr *) cinfo->err;

  // Always display the message.
  // We could postpone this until after returning, if we chose.
  (*cinfo->err->output_message) (cinfo);

  // Return control to the setjmp point
  longjmp(myerr->setjmp_buffer, 1);
}

unsigned char *	VSImage::ReadJPEG()
{
    unsigned char * image=NULL;
    JSAMPARRAY row_pointers=NULL;// Output row buffer

    try {

	this->img_depth = 8;
	jpeg_decompress_struct cinfo;

	my_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	if (setjmp(jerr.setjmp_buffer))
	{
		// If we get here, the JPEG code has signaled an error.
		// We need to clean up the JPEG object, and return.
		jpeg_destroy_decompress(&cinfo);
#ifdef VSIMAGE_DEBUG
		cerr<<"VSImage ERROR : error reading jpg file"<<endl;
#endif
		VSIMAGE_FAILURE(1,img_file->GetFilename().c_str());
		throw(1);
	}

	jpeg_create_decompress(&cinfo);

	if( !img_file->UseVolume())
		jpeg_stdio_src((j_decompress_ptr)&cinfo, img_file->GetFP());
	else
		jpeg_memory_src(&cinfo, (unsigned char *)img_file->get_pk3_data(), img_file->Size());

	(void) jpeg_read_header(&cinfo, TRUE);
	this->sizeX = cinfo.image_width;
	this->sizeY = cinfo.image_height;

	(void) jpeg_start_decompress(&cinfo);

	this->img_color_type = PNG_COLOR_TYPE_RGB;
	if (cinfo.output_components == 1)
		this->img_color_type = PNG_COLOR_TYPE_GRAY;
	else if (cinfo.output_components==4)
		this->img_color_type = PNG_COLOR_TYPE_RGB_ALPHA;
	else if (cinfo.output_components== 2)
		this->img_color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
	
	switch(this->img_color_type) {
	case PNG_COLOR_TYPE_RGB: this->mode = _24BIT; break;
	case PNG_COLOR_TYPE_RGB_ALPHA: this->mode = _24BITRGBA; break;
	case PNG_COLOR_TYPE_GRAY: this->mode = _8BIT; break;
	case PNG_COLOR_TYPE_GRAY_ALPHA: this->mode = _8BIT; break;
	}

#ifdef VSIMAGE_DEBUG
	cerr<<"1. Loading a JPEG file : width="<<sizeX<<", height="<<sizeY<<", img_color="<<img_color_type<<endl;
#endif
	row_pointers = (unsigned char **)malloc (sizeof (unsigned char *) * cinfo.image_height);

	this->img_depth=8;
	int numchan =cinfo.output_components;

	unsigned long stride = numchan*sizeof (unsigned char)*this->img_depth/8;
	image = (unsigned char *) malloc (stride*cinfo.image_width*cinfo.image_height);

	for (unsigned int i=0;i<cinfo.image_height;i++)
	{
		row_pointers[i] = &image[i*stride*cinfo.image_width];
	}
	unsigned int count=0;
	while (count<this->sizeY)
	{
		count+= jpeg_read_scanlines(&cinfo,&( row_pointers[count]), this->sizeY-count);
	}

	(void) jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	unsigned char * result=image;
	if (tt) {
		result = (*tt) (this->img_depth,this->img_color_type,this->sizeX,this->sizeY,row_pointers);
		free (image); image=NULL;
	}
	free (row_pointers); row_pointers=NULL;
	if( result)
		this->AllocatePalette();
	return result;

    } catch(...) {
        if (image) free(image); image=NULL;
        if (row_pointers) free(row_pointers); row_pointers=NULL;
        return NULL;
    }
}

unsigned char *	VSImage::ReadBMP()
{
  	unsigned char *data = NULL;
    unsigned char *cdata= NULL;
    unsigned char *adata= NULL;

    try {

	if( CheckBMPSignature( img_file)!=Ok)
	{
		cerr<<"VSImage ERROR : BMP signature check failed : this should not happen !!!"<<endl;
		VSIMAGE_FAILURE(1,img_file->GetFilename().c_str());
		throw(1);
	}
	//seek back to beginning
	img_file->GoTo(SIZEOF_BITMAPFILEHEADER);
	//long temp;
	BITMAPINFOHEADER info;
	img_file->Read(&info, SIZEOF_BITMAPINFOHEADER);
	this->sizeX = le32_to_cpu(info.biWidth);	
	this->sizeY = le32_to_cpu(info.biHeight);

	/*
	if (img_file2!=NULL)
	{
		img_file2->GoTo(SIZEOF_BITMAPFILEHEADER);
		  
		  img_file2->Read(&info1,SIZEOF_BITMAPINFOHEADER);
		  if (tex->sizeX != (unsigned int) le32_to_cpu(info1.biWidth)||tex->sizeY!=(unsigned int)le32_to_cpu(info1.biHeight))
		  {
		      return NULL;
		  }
		  RGBQUAD ptemp1;	
		  if (le16_to_cpu(info1.biBitCount) == 8)
		  {
		      for (int i=0; i<256; i++)
				img_file2->Read(&ptemp1, sizeof(RGBQUAD)); //get rid of the palette for a b&w greyscale alphamap
		      
		  }
	}
	*/
	if(le16_to_cpu(info.biBitCount) == 24)
	{
		mode = _24BIT; //Someone said _24BIT isn't supported by most cards, but PNG and JPEG use it widely without problems, so it must be untrue...
		if(img_file2 && img_file2->Valid())
			mode = _24BITRGBA;
        int ncomp = ((mode==_24BIT)?3:4);
        int cstride = ((sizeof(unsigned char)*3*this->sizeX)+3)&~3; //BMP rows must be aligned to 32 bits
        int astride = ((sizeof(unsigned char)*this->sizeX)+3)&~3; //BMP rows must be aligned to 32 bits
        int stride  = (sizeof(unsigned char)*ncomp*this->sizeX);
	    data = (unsigned char *)malloc (stride*this->sizeY);
		if (data==NULL) return NULL;
        if (mode!=_24BIT) {
            cdata=(unsigned char *)malloc (cstride);
            adata=(unsigned char *)malloc (astride);
            if ((cdata==NULL)||(adata==NULL)) throw("memory");
            unsigned char *row = data+(this->sizeY-1)*stride;
            for (unsigned int i=0; i<this->sizeY; i++,row-=stride) {
                img_file->Read(cdata,cstride);
                img_file2->Read(adata,astride);
                unsigned char *cpix=cdata,*apix=adata,*pix=row;
                for (unsigned int j=0; j<this->sizeX; j++,cpix+=3,apix++,pix+=4) {
                    pix[0] = cpix[2];
                    pix[1] = cpix[1];
                    pix[2] = cpix[0];
                    pix[3] = apix[0];
                };
            };
            free(cdata); cdata=NULL;
            free(adata); adata=NULL;
        } else {
            unsigned char *row = data+(this->sizeY-1)*stride;
            unsigned long dummy;
            for (unsigned int i=0; i<this->sizeY; i++,row-=stride) {
                img_file->Read(row,stride);
                if (cstride>stride) {
                    assert(cstride-stride < sizeof(dummy));
                    img_file->Read(&dummy,cstride-stride);
                };
                unsigned char *pix=row;
                for (unsigned int j=0; j<this->sizeX; j++,pix+=3) {
                    unsigned char apix=pix[0];
                    pix[0] = pix[2];
                    pix[2] = apix;
                };
            };
        };
	}
	else if(le16_to_cpu(info.biBitCount) == 8)
	{
		mode = _8BIT;
		data = NULL;
		data= (unsigned char *)malloc(sizeof(unsigned char)*sizeY*sizeX);
		this->palette = (unsigned char *)malloc(sizeof(unsigned char)* (256*4+1));
		memset (this->palette,0,(256*4+1)*sizeof(unsigned char));
		unsigned char *paltemp = this->palette;
		unsigned char ctemp;
		for(int palcount = 0; palcount < 256; palcount++)
		{
			img_file->Read(paltemp, SIZEOF_RGBQUAD);
			ctemp = paltemp[0];
			paltemp[0] = paltemp[2];
			paltemp[2] = ctemp;
			paltemp+=4; // pal size
		}
		if (!data)
			return NULL;
		for (int i=sizeY-1; i>=0;i--)
		{
			for (unsigned int j=0; j<sizeX;j++)
			{
				img_file->Read(data+ j + i * sizeX,sizeof (unsigned char));
			}
		}
	}
	return data;

    } catch(...) {
        if (cdata)free(cdata);cdata=NULL;
        if (adata)free(adata);adata=NULL;
        if (data) free(data); data=NULL;
        return NULL;
    };
}

#define IS_POT(x) (!((x) & ((x) -1)))

unsigned char *VSImage::ReadDDS()
{
	ddsHeader header;
	unsigned int internal = GL_NONE,type = GL_RGB;	
	int blockSize = 16;
	unsigned char *s=NULL;
	unsigned int inputSize = 0;
	int width=0;
	int height=0;
    try {	
	// Probably redundent, we already check this in CheckFormat
		if( CheckDDSSignature( img_file)!=Ok){
			cerr <<"VSImage ERROR : DDS Signature invalid, impossible.  !!!\n";
			VSIMAGE_FAILURE(1,img_file->GetFilename().c_str());
			throw(1);
		}
		// Skip what we already know. 
		img_file->GoTo(4);
		// Read in bytes to header.   Probably not endian-safe
                char ibuffer[4];                
		img_file->Read(ibuffer,4);
                header.size=POSH_ReadU32FromLittle(ibuffer);
		img_file->Read(ibuffer,4);
		header.flags=POSH_ReadU32FromLittle(ibuffer);
		img_file->Read(ibuffer,4);
		header.height=POSH_ReadU32FromLittle(ibuffer);
		img_file->Read(ibuffer,4);
		header.width=POSH_ReadU32FromLittle(ibuffer);
		img_file->Read(ibuffer,4);
		header.linsize=POSH_ReadU32FromLittle(ibuffer);
		img_file->Read(ibuffer,4);
		header.depth=POSH_ReadU32FromLittle(ibuffer);
		img_file->Read(ibuffer,4);
		header.nmips=POSH_ReadU32FromLittle(ibuffer);
		img_file->GoTo(84);
		img_file->Read(&header.pixelFormat.fourcc[0],1);
		img_file->Read(&header.pixelFormat.fourcc[1],1);
		img_file->Read(&header.pixelFormat.fourcc[2],1);
		img_file->Read(&header.pixelFormat.fourcc[3],1);
		img_file->Read(ibuffer,4);
		header.pixelFormat.bpp=POSH_ReadU32FromLittle(ibuffer);
		
		img_file->GoTo(128);
		// Set VSImage attributes 
		this->img_depth = header.pixelFormat.bpp;
		this->sizeX=header.width;
		this->sizeY=header.height;
		bool useDefaultType=false;
		switch(header.pixelFormat.bpp){
                case 4: 
                  type = GL_LUMINANCE;
                  break;
                case 8: 
                  type = GL_LUMINANCE_ALPHA;
                  this->img_alpha = true;
                  break;
                case 24: 
                  type=GL_RGB;
                  this->img_alpha = false;
                  break;
                case 32: 
                  type=GL_RGBA;
                  this->img_alpha = true;
                  break;
                case 0:
                  useDefaultType=true;
                  break;
                default:
               		useDefaultType=true;
					break;

		}
		switch(header.pixelFormat.fourcc[3]){
                case '1': 
				  blockSize = 8;
                  if(type==GL_RGB||useDefaultType) {
				  	this->img_depth = 24;
                    this->mode = _DXT1;
                    this->img_alpha=false;
                    type=GL_RGBA;
                  }else {
                    this->mode = _DXT1RGBA;
                    type=GL_RGBA;
                    this->img_alpha=true;
                  }
                  break;
                case '3': 
                  this->mode = _DXT3;
                  if (useDefaultType){
                    this->img_alpha=true;
					this->img_depth = 32;
                    type=GL_RGBA;
                  }
                  break;
                case '5': 
                  this->mode = _DXT5;
                  if (useDefaultType) {
                    this->img_alpha=true;
					this->img_depth=32;
                    type=GL_RGBA;
                  }
                  break;
                default:
                  cerr <<"VSImage ERROR : DDS Compression Scheme, impossible.[" <<(int)header.pixelFormat.fourcc[0]<<";"<<(int)header.pixelFormat.fourcc[1]<<";"<<(int)header.pixelFormat.fourcc[2]<<";"<<(int)header.pixelFormat.fourcc[3]<<";!\n";
                  VSIMAGE_FAILURE(1,img_file->GetFilename().c_str());
                  throw(1);
		}
		inputSize = 0;
		width=header.width;
		height= header.height;
		for(int i = 0;i<header.nmips;++i){
			inputSize += ((width+3)/4)*((height+3)/4)*blockSize;
			if(width != 1)
				width >>=1;
			if(height != 1)
				height >>=1;
		}
		s = (unsigned char*)malloc(inputSize);
		// the following is probably not endian-safe
		img_file->Read(s,inputSize);   
		// At the end of execution what we have is the following
		// s contains the main texture and all it's mipmaps 
		// sizeY is the height of the initial texture, sizeX is the width.
		// mode is the compressed format of the texture. It is assumed to be rgba
		// depth is the depth of the uncompressed image. not sure where this is used
		// nmaps is the number of mipmaps
		return(s);
    } catch(...) {
		if(s) free(s);
        return NULL;
    };
}
	
	
void	VSImage::AllocatePalette()
{
	  //FIXME deal with palettes and grayscale with alpha
	  if (!(img_color_type&PNG_HAS_COLOR)||(img_color_type&PNG_HAS_PALETTE)) {
	    if (!(img_color_type&PNG_HAS_COLOR)){
	      palette = (unsigned char *) malloc(sizeof(unsigned char)*(256*4+1));
	      for (unsigned int i =0;i<256;i++) {
			palette[i*4]=i;
			palette[i*4+1]=i;
			palette[i*4+2]=i;
			palette[i*4+3]=255;
	      }
	    }
	  } 
}

VSError	VSImage::WriteImage( char * filename, unsigned char * data, VSImageType type, unsigned int width, unsigned int height, bool alpha, char bpp, VSFileType ft, bool flip)
{
	this->img_type = type;
	VSFile f;
	VSError err = f.OpenCreateWrite( filename, ft);
	if( err>Ok)
	{
		cerr<<"VSImage ERROR : failed to open "<<filename<<" for writing"<<endl;
		VSIMAGE_FAILURE(1,img_file->GetFilename().c_str());
		return VSFileSystem::FileNotFound;
	}

	VSError ret = this->WriteImage( &f, data, type, width, height, alpha, bpp,flip);
	f.Close();
	return ret;
}

VSError	VSImage::WriteImage( VSFile * pf, unsigned char * data, VSImageType type, unsigned int width, unsigned int height, bool alpha, char bpp, bool flip)
{
	VSError ret = BadFormat;

	this->img_file = pf;
	this->img_depth = bpp;
	this->sizeX = width;
	this->sizeY = height;
	this->img_alpha = alpha;
        this->flip=flip;
	switch( type)
	{
		case PngImage :
			ret = this->WritePNG( data);
		break;
		case JpegImage :
			ret = this->WriteJPEG( data);
		break;
		case BmpImage :
			ret = this->WriteBMP( data);
		break;
		default :
			cerr<<"VSImage ERROR : Unknown image format"<<endl;
			VSIMAGE_FAILURE(1,img_file->GetFilename().c_str());
			return VSFileSystem::BadFormat;
	}
	this->img_file = NULL;
	this->img_depth = 0;
	this->sizeX = 0;
	this->sizeY = 0;
	this->img_alpha = false;
	return ret;
}

VSError	VSImage::WritePNG( unsigned char * data)
{
  png_structp png_ptr = png_create_write_struct
    (PNG_LIBPNG_VER_STRING, (png_voidp)NULL,NULL,NULL);
  if (!png_ptr)
    return BadFormat;
  
  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    return BadFormat;
  }
  if (setjmp(png_ptr->jmpbuf)) {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return BadFormat;
  }

  //if( !img_file->UseVolume())
  // For now we always write to standard files
  png_init_io(png_ptr, img_file->GetFP());

  png_set_filter(png_ptr, 0, PNG_FILTER_NONE);
  png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

  /* set other zlib parameters */
  png_set_compression_mem_level(png_ptr, 8);
  png_set_compression_strategy(png_ptr, Z_DEFAULT_STRATEGY);
  png_set_compression_window_bits(png_ptr, 15);
  png_set_compression_method(png_ptr, 8);
  
  png_set_IHDR(png_ptr, 
	       info_ptr, 
	       this->sizeX,
	       this->sizeY,
	       this->img_depth, 
	       this->img_alpha?PNG_COLOR_TYPE_RGB_ALPHA:PNG_COLOR_TYPE_RGB, 
	       PNG_INTERLACE_NONE,
	       PNG_COMPRESSION_TYPE_DEFAULT, 
	       PNG_FILTER_TYPE_DEFAULT);
  
  png_write_info(png_ptr, info_ptr);
# if __BYTE_ORDER != __BIG_ENDIAN  
  if (this->img_depth==16) {
    png_set_swap(png_ptr);
  }
#endif
  int stride = (this->img_depth/8)*(this->img_alpha?4:3);
  png_byte **row_pointers = new png_byte*[this->sizeY];
  if (this->flip) {
    for (int i=this->sizeY-1,j=0;i>=0;i--,++j) {
      row_pointers[j]= (png_byte *)&data[stride*i*sizeX];
    }
  }else {
    for (unsigned int i=0;i<this->sizeY;i++) {
      row_pointers[i]= (png_byte *)&data[stride*i*sizeX];
    }
  }
  png_write_image (png_ptr,row_pointers);
  png_write_end(png_ptr, info_ptr);
  png_write_flush(png_ptr);
  png_destroy_write_struct(&png_ptr, &info_ptr);

  free (data);
  delete [] row_pointers;
  return Ok;
}

VSError	VSImage::WriteJPEG( unsigned char * data)
{
	return BadFormat;
}

VSError	VSImage::WriteBMP( unsigned char * data)
{
	return BadFormat;
}

