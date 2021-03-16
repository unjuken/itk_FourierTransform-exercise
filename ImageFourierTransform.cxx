/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkForwardFFTImageFilter.h"
#include "itkComplexToModulusImageFilter.h"
#include "itkIntensityWindowingImageFilter.h"
#include "itkFFTShiftImageFilter.h"

int main( int argc, char* argv[] )
{
  if( argc != 3 )
  {
    std::cerr << "Usage: "<< std::endl;
    std::cerr << argv[0];
    std::cerr << " <InputFileName> <OutputFileName>";
    std::cerr << std::endl;
    return EXIT_FAILURE;
  }

  const char * inputFileName = argv[1];
  const char * outputFileName = argv[2];
  
  const unsigned int Dimension = 2;
  typedef float                                  PixelType;
  typedef itk::Image< PixelType, Dimension >     RealImageType;
  typedef unsigned short                         IntPixelType;
  typedef itk::Image< IntPixelType, Dimension >  IntImageType;

  typedef itk::ImageFileReader< RealImageType >  ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName( inputFileName );
  reader->Update();

  typedef itk::ForwardFFTImageFilter< RealImageType > ForwardFFTFilterType;
  typedef ForwardFFTFilterType::OutputImageType ComplexImageType;
  ForwardFFTFilterType::Pointer forwardFFTFilter = ForwardFFTFilterType::New();
  forwardFFTFilter->SetInput( reader->GetOutput() );
  forwardFFTFilter->Update();
  
  typedef itk::ComplexToModulusImageFilter< ComplexImageType, RealImageType > ComplexToModulusFilterType;
  ComplexToModulusFilterType::Pointer complexToModulusFilter = ComplexToModulusFilterType::New();
  complexToModulusFilter->SetInput( forwardFFTFilter->GetOutput() );
  complexToModulusFilter->Update();
  
  typedef itk::IntensityWindowingImageFilter< RealImageType, IntImageType > WindowingFilterType;
  WindowingFilterType::Pointer windowingFilter = WindowingFilterType::New();
  windowingFilter->SetInput( complexToModulusFilter->GetOutput() );
  windowingFilter->SetWindowMinimum( 0 );
  windowingFilter->SetWindowMaximum( 20000 );
  windowingFilter->Update();

  typedef itk::FFTShiftImageFilter< IntImageType, IntImageType > FFTShiftFilterType;
  FFTShiftFilterType::Pointer fftShiftFilter = FFTShiftFilterType::New();
  fftShiftFilter->SetInput( windowingFilter->GetOutput() );
  fftShiftFilter->Update();
  
  typedef itk::ImageFileWriter< IntImageType > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName( outputFileName );
  writer->SetInput( fftShiftFilter->GetOutput() );
  try
  {
    writer->Update();
  }
  catch( itk::ExceptionObject & error )
  {
    std::cerr << "Error: " << error << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
