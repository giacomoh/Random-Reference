WINDOWS ONLY!

Get the releases here: https://github.com/giacomoh/Random-Reference/tags

# Screenshots
![image](https://github.com/giacomoh/Random-Reference/assets/53836108/532fdc6c-6b3c-4523-9850-110ac08bea17)

![image](https://github.com/giacomoh/Random-Reference/assets/53836108/535d292c-f9e8-41bd-b35d-cf3b60841009)

It's decently lightweight, the only limit is the size of your current image. 
![image](https://github.com/giacomoh/Random-Reference/assets/53836108/fd8113e9-560f-4922-97c9-d3ab53ad56ea)

![image](https://github.com/giacomoh/Random-Reference/assets/53836108/931f70b1-0236-4e95-bde1-d7af9800717d)

![image](https://github.com/giacomoh/Random-Reference/assets/53836108/152dd767-a2ea-43eb-95ff-5523eb6517a5)

# Info

Hi, this is a program for randomly selecting and displaying images.
The first time you open it, it will ask you a folder from which to pick the images. Afterwards it will remember it.
You can view images, flip them, add a bunch of filters and more. When the timer reaches 0 it will start again and pick a new image.
Since i moved from python to c++ it's able to read the image color profiles (ICCs) and show them automatically images that used to look dull on the old app now look better.
There could still be problems opening certain files, try renaming them without special characters!
You can stop the timer by clicking on it. It's going to turn red when it's paused.

Keep everything in the same folder!

# To do

- more file supported (for now all the popular files are supported: pngs, jpgs, webps, etc)
- better filters? i'm still testing how much i can push them while keeping it all lightweight 
- i need to make it look cooler!
- add icons
- better installation folder organization
- fix images with special characters in the path won't load (rename them or the directory they're in to fix this. you can use power rename or just a powershell script)
