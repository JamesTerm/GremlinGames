xcopy .\Content-Source .\LOW_RES /S /Y /E /I /Exclude:Exclude_Raw.txt 

convert ".\Content-Source\Space\Space Boxes\SpaceBox_B_w_Planets\boxImages_000.png" -resize 1024 ".\LOW_RES\Space\Space Boxes\SpaceBox_B_w_Planets\boxImages_000.png"
convert ".\Content-Source\Space\Space Boxes\SpaceBox_B_w_Planets\boxImages_001.png" -resize 1024 ".\LOW_RES\Space\Space Boxes\SpaceBox_B_w_Planets\boxImages_001.png"
convert ".\Content-Source\Space\Space Boxes\SpaceBox_B_w_Planets\boxImages_002.png" -resize 1024 ".\LOW_RES\Space\Space Boxes\SpaceBox_B_w_Planets\boxImages_002.png"
convert ".\Content-Source\Space\Space Boxes\SpaceBox_B_w_Planets\boxImages_003.png" -resize 1024 ".\LOW_RES\Space\Space Boxes\SpaceBox_B_w_Planets\boxImages_003.png"
convert ".\Content-Source\Space\Space Boxes\SpaceBox_B_w_Planets\boxImages_004.png" -resize 1024 ".\LOW_RES\Space\Space Boxes\SpaceBox_B_w_Planets\boxImages_004.png"
convert ".\Content-Source\Space\Space Boxes\SpaceBox_B_w_Planets\boxImages_005.png" -resize 1024 ".\LOW_RES\Space\Space Boxes\SpaceBox_B_w_Planets\boxImages_005.png"

convert ".\Content-Source\Space_Ring01\ring_01.PNG" -resize 128 ".\LOW_RES\Space_Ring01\ring_01.PNG"

convert ".\Content-Source\MiramarStation\ScafoldTrans1.PNG" -resize 16 ".\LOW_RES\MiramarStation\ScafoldTrans1.PNG"
convert ".\Content-Source\MiramarStation\Miramar01.PNG" -resize 1024 ".\LOW_RES\MiramarStation\Miramar01.PNG"
convert ".\Content-Source\MiramarStation\Engine Flame Alpha.PNG" -resize 64 ".\LOW_RES\MiramarStation\Engine Flame Alpha.PNG"
convert ".\Content-Source\MiramarStation\Engine Flame Color.PNG" -resize 256 ".\LOW_RES\MiramarStation\Engine Flame Color.PNG"