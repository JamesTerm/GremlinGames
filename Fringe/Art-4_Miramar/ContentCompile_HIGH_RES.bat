xcopy .\Content-Source .\HIGH_RES /S /Y /E /I /Exclude:Exclude_Raw.txt 

convert ".\Content-Source\Space_Ring01\ring_01.PNG" -resize 512 ".\HIGH_RES\Space_Ring01\ring_01.PNG"