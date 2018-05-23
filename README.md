# Android_O_MTK_GMC303

Requirements
-----------
- MTK MT6575 
- Version: Android O
- GMC303

I2C Connections
-----------
- Use I2C1
- Slave address 0x0C / 0x0D
- CAD---GND
- RSTN---VID

File Location
-----------
- put gmc303 to path alps/kernel-4.4/drivers/misc/mediatek/sensors-1.0/gmc303 
- put libsensor to path alps/vendor/mediatek/proprietary/hardware/libsensor

Kernel Build
-----------
- source ./build/envsetup.sh
- lunch full_k57pv1_pre-eng
- make -j4
