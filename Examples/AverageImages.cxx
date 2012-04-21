/*=========================================================================

  Program:   Advanced Normalization Tools
  Module:    $RCSfile: AverageImages.cxx,v $
  Language:  C++
  Date:      $Date: 2009/01/27 23:25:24 $
  Version:   $Revision: 1.21 $

  Copyright (c) ConsortiumOfANTS. All rights reserved.
  See accompanying COPYING.txt or
 http://sourceforge.net/projects/advants/files/ANTS/ANTSCopyright.txt for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

// We divide the 2nd input image by its mean and add it to the first
// input image with weight 1/n.
// The output overwrites the 1st img with the sum.

// Note: could easily add variance computation
// http://people.revoledu.com/kardi/tutorial/RecursiveStatistic/Time-Variance.htm

#include "antscout.hxx"
#include <algorithm>

#include "itkArray.h"
#include "itkVariableLengthVector.h"
#include "itkImage.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkOptimalSharpeningImageFilter.h"
#include "itkLaplacianSharpeningImageFilter.h"
#include <algorithm>

namespace ants
{
template <unsigned int ImageDimension, unsigned int NVectorComponents>
int AverageImages1(unsigned int argc, char *argv[])
{
  typedef float                                        PixelType;
  typedef itk::Image<PixelType, ImageDimension>        ImageType;
  typedef itk::ImageRegionIteratorWithIndex<ImageType> Iterator;
  typedef itk::ImageFileReader<ImageType>              ImageFileReader;
  typedef itk::ImageFileWriter<ImageType>              writertype;

  bool  normalizei = atoi(argv[3]);
  float numberofimages = (float)argc - 4.;
  typename ImageType::Pointer averageimage = NULL;
  typename ImageType::Pointer image2 = NULL;

  typename ImageType::SizeType size;
  size.Fill(0);
  unsigned int bigimage = 0;
  for( unsigned int j = 4; j < argc; j++ )
    {
    // Get the image dimension
    std::string fn = std::string(argv[j]);
    antscout << " fn " << fn << std::endl;
    typename itk::ImageIOBase::Pointer imageIO =
      itk::ImageIOFactory::CreateImageIO(fn.c_str(), itk::ImageIOFactory::ReadMode);
    imageIO->SetFileName(fn.c_str() );
    imageIO->ReadImageInformation();
    for( unsigned int i = 0; i < imageIO->GetNumberOfDimensions(); i++ )
      {
      if( imageIO->GetDimensions(i) > size[i] )
        {
        size[i] = imageIO->GetDimensions(i);
        bigimage = j;
        antscout << " bigimage " << j << " size " << size << std::endl;
        }
      }
    }

  antscout << " largest image " << size << std::endl;
  typename ImageFileReader::Pointer reader = ImageFileReader::New();
  reader->SetFileName(argv[bigimage]);
  reader->Update();
  averageimage = reader->GetOutput();
  unsigned int vectorlength = reader->GetImageIO()->GetNumberOfComponents();
  antscout << " Averaging " << numberofimages << " images with dim = " << ImageDimension << " vector components "
           << vectorlength << std::endl;
  PixelType meanval = 0;
  averageimage->FillBuffer(meanval);
  for( unsigned int j = 4; j < argc; j++ )
    {
    antscout << " reading " << std::string(argv[j]) << std::endl;
    typename ImageFileReader::Pointer rdr = ImageFileReader::New();
    rdr->SetFileName(argv[j]);
    rdr->Update();
    image2 = rdr->GetOutput();
    Iterator      vfIter2( image2,  image2->GetLargestPossibleRegion() );
    unsigned long ct = 0;
    if( normalizei )
      {
      meanval = 0;
      for(  vfIter2.GoToBegin(); !vfIter2.IsAtEnd(); ++vfIter2 )
        {
        PixelType localp = image2->GetPixel( vfIter2.GetIndex() );
        meanval = meanval + localp;
        ct++;
        }
      if( ct > 0 )
        {
        meanval = meanval / (float)ct;
        }
      if( meanval <= 0 )
        {
        meanval = (1);
        }
      }
    for(  vfIter2.GoToBegin(); !vfIter2.IsAtEnd(); ++vfIter2 )
      {
      PixelType val = vfIter2.Get();
      if( normalizei )
        {
        val /= meanval;
        }
      val = val / (float)numberofimages;
      PixelType oldval = averageimage->GetPixel(vfIter2.GetIndex() );
      averageimage->SetPixel(vfIter2.GetIndex(), val + oldval );
      }
    }

  //  typedef itk::OptimalSharpeningImageFilter<ImageType,ImageType > sharpeningFilter;
  typedef itk::LaplacianSharpeningImageFilter<ImageType, ImageType> sharpeningFilter;
  typename sharpeningFilter::Pointer shFilter = sharpeningFilter::New();
  if( normalizei && argc > 3 && vectorlength == 1 )
    {
    shFilter->SetInput( averageimage );
    //    shFilter->SetSValue(0.5);
    averageimage =  shFilter->GetOutput();
    }

  antscout << " writing output ";
    {
    typename writertype::Pointer writer = writertype::New();
    writer->SetFileName(argv[2]);
    writer->SetInput( averageimage );
    writer->Update();
    }

  return 0;
}

template <unsigned int ImageDimension, unsigned int NVectorComponents>
int AverageImages(unsigned int argc, char *argv[])
{
  typedef itk::Vector<float, NVectorComponents>        PixelType;
  typedef itk::Image<PixelType, ImageDimension>        ImageType;
  typedef itk::ImageRegionIteratorWithIndex<ImageType> Iterator;
  typedef itk::ImageFileReader<ImageType>              ImageFileReader;
  typedef itk::ImageFileWriter<ImageType>              writertype;

  //  bool  normalizei = atoi(argv[3]);
  float numberofimages = (float)argc - 4.;
  typename ImageType::Pointer averageimage = NULL;
  typename ImageType::Pointer image2 = NULL;

  typename ImageType::SizeType size;
  size.Fill(0);
  unsigned int bigimage = 4;
  for( unsigned int j = 4; j < argc; j++ )
    {
    // Get the image dimension
    std::string fn = std::string(argv[j]);
    antscout << " fn " << fn << " " << ImageDimension << " " << NVectorComponents << std::endl;
    typename itk::ImageIOBase::Pointer imageIO =
      itk::ImageIOFactory::CreateImageIO(fn.c_str(), itk::ImageIOFactory::ReadMode);
    imageIO->SetFileName( fn.c_str() );
    imageIO->ReadImageInformation();
    for( unsigned int i = 0; i < imageIO->GetNumberOfDimensions(); i++ )
      {
      if( imageIO->GetDimensions(i) > size[i] )
        {
        size[i] = imageIO->GetDimensions(i);
        bigimage = j;
        antscout << " bigimage " << j << " size " << size << std::endl;
        }
      }
    }

  antscout << " largest image " << size << std::endl;
  typename ImageFileReader::Pointer reader = ImageFileReader::New();
  reader->SetFileName(argv[bigimage]);
  reader->Update();
  averageimage = reader->GetOutput();
  unsigned int vectorlength = reader->GetImageIO()->GetNumberOfComponents();
  antscout << " Averaging " << numberofimages << " images with dim = " << ImageDimension << " vector components "
           << vectorlength << std::endl;
  typename ImageType::IndexType zindex; zindex.Fill(0);
  PixelType meanval = reader->GetOutput()->GetPixel(zindex);
  meanval.Fill(0);
  averageimage->FillBuffer(meanval);
  for( unsigned int j = 4; j < argc; j++ )
    {
    antscout << " reading " << std::string(argv[j]) << " for average " << std::endl;
    typename ImageFileReader::Pointer rdr = ImageFileReader::New();
    rdr->SetFileName(argv[j]);
    rdr->Update();
    image2 = rdr->GetOutput();
    Iterator vfIter2( image2,  image2->GetLargestPossibleRegion() );
    for(  vfIter2.GoToBegin(); !vfIter2.IsAtEnd(); ++vfIter2 )
      {
      PixelType val = vfIter2.Get();
      double    valnorm = val.GetNorm();
      if( !vnl_math_isnan( valnorm  ) &&  !vnl_math_isinf( valnorm  )   )
        {
        val = val / (float)numberofimages;
        PixelType oldval = averageimage->GetPixel( vfIter2.GetIndex() );
        averageimage->SetPixel(vfIter2.GetIndex(), val + oldval );
        }
      }
    }

    {
    typename writertype::Pointer writer = writertype::New();
    writer->SetFileName(argv[2]);
    writer->SetInput( averageimage );
    writer->Update();
    }

  return 0;
}

// entry point for the library; parameter 'args' is equivalent to 'argv' in (argc,argv) of commandline parameters to
// 'main()'
int AverageImages( std::vector<std::string> args, std::ostream* out_stream = NULL )
{
  // put the arguments coming in as 'args' into standard (argc,argv) format;
  // 'args' doesn't have the command name as first, argument, so add it manually;
  // 'args' may have adjacent arguments concatenated into one argument,
  // which the parser should handle
  args.insert( args.begin(), "AverageImages" );
  std::remove( args.begin(), args.end(), std::string( "" ) );
  std::remove( args.begin(), args.end(), std::string( "" ) );
  int     argc = args.size();
  char* * argv = new char *[args.size() + 1];
  for( unsigned int i = 0; i < args.size(); ++i )
    {
    // allocate space for the string plus a null character
    argv[i] = new char[args[i].length() + 1];
    std::strncpy( argv[i], args[i].c_str(), args[i].length() );
    // place the null character in the end
    argv[i][args[i].length()] = '\0';
    }
  argv[argc] = 0;
  // class to automatically cleanup argv upon destruction
  class Cleanup_argv
  {
public:
    Cleanup_argv( char* * argv_, int argc_plus_one_ ) : argv( argv_ ), argc_plus_one( argc_plus_one_ )
    {
    }

    ~Cleanup_argv()
    {
      for( unsigned int i = 0; i < argc_plus_one; ++i )
        {
        delete[] argv[i];
        }
      delete[] argv;
    }

private:
    char* *      argv;
    unsigned int argc_plus_one;
  };
  Cleanup_argv cleanup_argv( argv, argc + 1 );

  antscout->set_stream( out_stream );

  if( argc < 3 )
    {
    antscout << "\n" << std::endl;
    antscout << "Usage: \n" << std::endl;
    antscout << argv[0] << " ImageDimension Outputfname.nii.gz Normalize <images> \n" << std::endl;
    antscout << " Compulsory arguments: \n" << std::endl;
    antscout << " ImageDimension: 2 or 3 (for 2 or 3 dimensional input).\n " << std::endl;
    antscout << " Outputfname.nii.gz: the name of the resulting image.\n" << std::endl;
    antscout
      <<
      " Normalize: 0 (false) or 1 (true); if true, the 2nd image is divided by its mean. This will select the largest image to average into.\n"
      << std::endl;
    antscout << " Example Usage:\n" << std::endl;
    antscout << argv[0] << " 3 average.nii.gz  1  *.nii.gz \n" << std::endl;
    antscout << " \n" << std::endl;
    return 1;
    }

  int                       dim = atoi( argv[1] );
  itk::ImageIOBase::Pointer imageIO =
    itk::ImageIOFactory::CreateImageIO(argv[4], itk::ImageIOFactory::ReadMode);
  imageIO->SetFileName(argv[4]);
  imageIO->ReadImageInformation();
  unsigned int ncomponents = imageIO->GetNumberOfComponents();

  // Get the image dimension
  switch( atoi(argv[1]) )
    {
    case 2:
      {
      switch( ncomponents )
        {
        case 2:
          {
          AverageImages<2, 2>(argc, argv);
          }
          break;
        default:
          {
          AverageImages1<2, 1>(argc, argv);
          }
          break;
        }
      }
      break;
    case 3:
      {
      switch( ncomponents )
        {
        case 7:
          {
          AverageImages<3, 7>(argc, argv);
          }
          break;
        case 6:
          {
          AverageImages<3, 6>(argc, argv);
          }
          break;
        case 3:
          {
          AverageImages<3, 3>(argc, argv);
          }
          break;
        case 2:
          {
          AverageImages<3, 2>(argc, argv);
          }
          break;
        default:
          {
          AverageImages1<3, 1>(argc, argv);
          }
          break;
        }
      }
      break;
    case 4:
      {
      switch( ncomponents )
        {
        case 7:
          {
          AverageImages<4, 7>(argc, argv);
          }
          break;
        case 6:
          {
          AverageImages<4, 6>(argc, argv);
          }
          break;
        case 4:
          {
          AverageImages<4, 4>(argc, argv);
          }
          break;
        case 3:
          {
          AverageImages<4, 3>(argc, argv);
          }
          break;
        default:
          {
          AverageImages1<4, 1>(argc, argv);
          }
          break;
        }
      }
      break;
    default:
      antscout << " You passed ImageDimension: " << dim << " . Please use only image domains of 2, 3 or 4  "
               << std::endl;
      return EXIT_FAILURE;
    }

  return 0;
}
} // namespace ants
