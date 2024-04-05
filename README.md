WINDOWS ONLY!

Get the releases here: https://github.com/giacomoh/Random-Reference/tags

# Screenshots

![image](https://github.com/giacomoh/reference-picker/assets/53836108/3abf46bc-7e30-4734-9261-a3271796297e)

![image](https://github.com/giacomoh/reference-picker/assets/53836108/df37ab6c-b44b-4415-a4c1-4f7c54d56d2a)
It's decently lightweight, the only limit is the size of your current image. 

![image](https://github.com/giacomoh/reference-picker/assets/53836108/fe020aed-7b25-43cb-90ba-115d8b9fdc1c)

![image](https://github.com/giacomoh/reference-picker/assets/53836108/7981ad24-ff63-45ae-be84-78f02ebf86cf)

![image](https://github.com/giacomoh/reference-picker/assets/53836108/1d7de2db-1e8d-4858-9d1f-ae0650628d44)

# Info

Hi, this is a program for randomly selecting and displaying images.
The first time you open it, it will ask you a folder from which to pick the images. Afterwards it will remember it.
You can view images, flip them, add a bunch of filters and more. When the timer reaches 0 it will start again and pick a new image.
Since i moved from python to c++ it's able to read the image color profiles (ICCs) and show them automatically images that used to look dull on the old app now look better.

You can stop the timer by clicking on it. It's going to turn red when it's paused.

Keep everything in the same folder! When installing too you will need to keep the MSI file and the EXE file in the same folder. 

To download click on the setup.exe and set the install location.
To uninstall click on setup.exe and click then select uninstall.

the c++ version looks worse than the python one, but believe me it runs much better. I'll make a graphic overhaul for the actual release.
# To do

- image rotation
- more file supported (for now all the popular files are supported: pngs, jpgs, webps, etc)
- better filters? i'm still testing how much i can push them while keeping it all lightweight 
- i need to make it look cooler!
- add icons
- better installation folder organization
- add open>recent (or save/delete paths)
- fix images with special characters in the path won't load (rename them or the directory they're in to fix this. you can use power rename or just a powershell script)
