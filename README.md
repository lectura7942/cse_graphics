# 2022-1 서강대학교 기초컴퓨터그래픽스 CSE4170 (임인성 교수)

## HW1_polygon

* 다각형 편집 기능 구현

|키|설명|
|--|----|
|SHIFT + 왼쪽마우스|점 그리기|
|ㅔ|다각형 완성하기|
|화살표키, 오른쪽마우스|다각형 이동하기|
|r|다각형 회전하기/멈추기|
|c|다각형 지우기|
|f|프로그램 종료하기|

## HW2_modelingTransformation

* 2차원 기하 변환 구현
* UFO 제외한 물체 코드는 학교에서 제공함  
![image](https://user-images.githubusercontent.com/81620001/189945189-820819b4-7d31-4d23-b356-df7224c0aa2f.png)

## HW3_projection_shading_light

* HW3: 물체 및 카메라 배치&이동  
* HW5: 광원 배치. Phong/Gouraud 쉐이딩. 쉐이더 효과 구현
* NVIDIA “Amazon Lumberyard Bistro” (https://developer.nvidia.com/orca/amazon-lumberyard-bistro) 배경사용  
* 물체 코드는 학교에서 제공함

### 카메라 조작법
|키|설명|효과|
|---|---|---|
|0|CAMERA_0. dragon과 tank를 위에서 바라보는 카메라 모드|- tank Gouraud shading<br/>- dragon Phong shading<br/>- 땡땡이 블라인드 효과|
|1|CAMERA_1. ironman과 spider를 바라보는 카메라 모드.|먼지 낀 렌즈 효과|
|2|CAMERA_2. ben과 cow를 바라보는 카메라 모드|cow 투명한 효과|
|3|CAMERA_3. godzilla와 tiger를 바라보는 카메라 모드|- tiger 고정 광원<br/>- 줌인/아웃에 따른 카메라 필름 색 변화|
|m|CAMERA_M. 움직이는 카메라 모드. 처음에는 dragon과 tank를 앞에서 바라보고 있다.|- tank Gouraud shading<br/>- dragon Phong shading<br/>- 움직이는 카메라 고정광원|
|t|CAMERA_T. 호랑이 관점 카메라|tiger 고정 광원|
|g|CAMERA_G. 호랑이 관찰 카메라|tiger 고정 광원|


### 세상 카메라 조작법
|키|설명|
|---|---|
|s/f|u축 따라 좌우 이동|
|e/d|n축 따라 전후 이동|
|q/a|v축 따라 상하 이동|
|왼쪽 마우스 버튼 + 좌우 이동|v축 둘레로 회전|
|왼쪽 마우스 버튼 + 위아래 이동|u축 둘레로 회전|
|오른쪽 마우스 버튼 + 위아래이동|n축 둘레로 회전|

### 쉐이더 효과 관련 조작법
|키|설명|
|--|----|
|m (CAMERA_M 모드에서)|움직이는 카메라에 고정된 광원 on/off|
|7|tank Gouraud shading, Phong shading 변환|
|스페이스 바|호랑이 이동/멈춤|
|w|세상 광원 on/off|
|b|호랑이 고정 광원 on/off|
|z|cow 투명 효과 on/off|
|오른쪽./위 화살표 키|cow 불투명도 증가|
|왼쪽/아래 화살표 키|cow 불투명도 감소|  

>**Data, Scene, Shaders 폴더는 로컬에만 존재! (크기 제한)**

## TODO
* 샘플 이미지 추가
