Introduction
======

This is a IP GeoLocation Application developed in C programming language.

The goal of the application is to show location of any internet connected computer running this code without accessing GPS hardware.
It returns the lattitude and longitude of the system's current location based on the system's external IP address. It also returns the accuracy of the retrieved location as a radius measured in Kilometers.

To talk with the internet curl library is used. Curl is a cmd line utility. Curl helps fetch and put information to the internet. The same people who developed Curl, have provided the Libcurl library for the C language.

Official website of curl: [https://curl.se/](https://curl.se/)

Prerequisites
======

### Step 1: SETTING ENVIROMENT

* Assuming you have 64-bit Windows, to get started. 
* Head to ControlPanel > Programs > TurnWindowsFeatures On/Off. 
* Check “Windows Subsystem for Linux” & click “OK”.
* Click “Restart now” when prompted to restart computer.

* Now install Windows 10 Ubuntu Bash Shell from Microsoft Store.
* Search "Ubuntu" in store & get the 1st one installed.

---

### Step 2: INSTALL NECESSARY LIBRARIES
	
- Launch Windows 10 Ubuntu Bash Shell.
- Run commands:
```bash
sudo apt-get update && sudo apt-get upgrade
sudo apt-get install libcurl4-gnutls-dev
sudo apt install clang
```
> We'll be using the 'clang' compiler to compile our c code

---

### Step 3: TEST INSTALLATION

- Save the following code as 'test.c'
```c
#include <stdio.h>
#include <curl/curl.h>
int main(){
	printf("libcurl version %s\n",LIBCURL_VERSION);
	return(0);
}
```
- Now launch Windows 10 Ubuntu Bash Shell
- Navigate to the directory 'test.c' is located on local drive
- Run commands:
```bash
cd ../../mnt/
ls -a
cd "driveName/desiredPath"
```
> Notice the forwardSlash('/'), Windows uses backslash('\\')
- We compile 'test.c' to test curl installation
- Run command to compile:
```bash
 clang -Wall -lcurl test.c -lm
```
> If no err is displayed then compilation is sucessful

> '-Wall' is the all warning switch

> '-lcurl' is for linkining curl library

> '-lm' flag is required if using math functions & including 'math.h'
- Atlast! We execute the program with the following command:
```bash
./a.out
```
