transformPath=./cmake-build-debug/FourierTransform
folder=transformedImages
mkdir "$folder"
(
cd $folder
mkdir images
)
for file in images/*;
do
  newPath="$folder/$file"
  $transformPath $file $newPath
done