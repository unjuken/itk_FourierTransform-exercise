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
#include "itkMaskImageFilter.h"
#include "itkFFTShiftImageFilter.h"
#include "itkInverseFFTImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkMinimumMaximumImageFilter.h"
#include "itkShiftScaleImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

int main( int argc, char* argv[] )
{
  if( argc != 4 )
  {
    std::cerr << "Usage: "<< std::endl;
    std::cerr << argv[0];
    std::cerr << " <InputFileName> <MaskFileName> <OutputFileName>";
    std::cerr << std::endl;
    return EXIT_FAILURE;
  }
  
  const char * inputFileName = argv[1];
  const char * maskFileName = argv[2];
  const char * outputFileName = argv[3];
  
  const unsigned int Dimension = 2;
  typedef float                                  PixelType;
  typedef itk::Image< PixelType, Dimension >     RealImageType;
  typedef unsigned char                          CharPixelType;
  typedef itk::Image< CharPixelType, Dimension > CharImageType;
  
  typedef itk::ImageFileReader< RealImageType >  ReaderType;
  ReaderType::Pointer inputReader = ReaderType::New();
  inputReader->SetFileName( inputFileName );
  inputReader->Update();
  
  ReaderType::Pointer maskReader = ReaderType::New();
  maskReader->SetFileName( maskFileName );
  maskReader->Update();

  typedef itk::ForwardFFTImageFilter< RealImageType > ForwardFFTFilterType;
  typedef ForwardFFTFilterType::OutputImageType ComplexImageType;
  ForwardFFTFilterType::Pointer forwardFFTFilter = ForwardFFTFilterType::New();
  forwardFFTFilter->SetInput( inputReader->GetOutput() );
  try
  {
    forwardFFTFilter->UpdateOutputInformation();
  }
  catch( itk::ExceptionObject & error )
  {
    std::cerr << "Error: " << error << std::endl;
    return EXIT_FAILURE;
  }

  typedef itk::FFTShiftImageFilter< RealImageType, RealImageType > FFTShiftFilterType;
  FFTShiftFilterType::Pointer fftShiftFilter = FFTShiftFilterType::New();
  fftShiftFilter->SetInput( maskReader->GetOutput() );
  fftShiftFilter->Update();
    
  typedef itk::MaskImageFilter< ComplexImageType, RealImageType, ComplexImageType >  MaskFilterType;
  MaskFilterType::Pointer maskFilter = MaskFilterType::New();
  maskFilter->SetInput1( forwardFFTFilter->GetOutput() );
  maskFilter->SetInput2( fftShiftFilter->GetOutput() );
  maskFilter->Update();

  typedef itk::InverseFFTImageFilter< ComplexImageType, RealImageType > InverseFilterType;
  InverseFilterType::Pointer inverseFFTFilter = InverseFilterType::New();
  inverseFFTFilter->SetInput( maskFilter->GetOutput() );
  inverseFFTFilter->Update();
  
  typedef itk::MinimumMaximumImageFilter< RealImageType > MinMaxFilterType;
  MinMaxFilterType::Pointer minMaxFilter = MinMaxFilterType::New();
  minMaxFilter->SetInput( inverseFFTFilter->GetOutput() );
  minMaxFilter->Update();

  PixelType min_int = minMaxFilter->GetMinimum();
  PixelType max_int = minMaxFilter->GetMaximum();
  PixelType int_range = max_int - min_int;

  typedef itk::ShiftScaleImageFilter< RealImageType, RealImageType > IntShiftFilterType;
  IntShiftFilterType::Pointer intShiftFilter = IntShiftFilterType::New();
  intShiftFilter->SetInput( inverseFFTFilter->GetOutput() );
  intShiftFilter->SetShift( - min_int );
  intShiftFilter->Update();

  typedef itk::RescaleIntensityImageFilter< RealImageType, RealImageType > RescaleFilterType;
  RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New( );
  if ( int_range > 255 && min_int < 0 )
    rescaleFilter->SetInput( intShiftFilter->GetOutput() );
  else
    rescaleFilter->SetInput( inverseFFTFilter->GetOutput() );
  rescaleFilter->SetOutputMinimum( 0 );
  rescaleFilter->SetOutputMaximum( 255 );
  rescaleFilter->Update();

  typedef itk::CastImageFilter< RealImageType, CharImageType > CastFilterType;
  CastFilterType::Pointer castFilter = CastFilterType::New();
  if ( int_range > 255 )
    castFilter->SetInput( rescaleFilter->GetOutput() );
  else if ( min_int < 0 )
    castFilter->SetInput( intShiftFilter->GetOutput() );
  else
    castFilter->SetInput( inverseFFTFilter->GetOutput() );
  castFilter->Update();

  typedef itk::ImageFileWriter< CharImageType > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName( outputFileName );
  writer->SetInput( castFilter->GetOutput() );
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
