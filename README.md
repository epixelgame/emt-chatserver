# emt-chatserver
A chatserver broadcasting messages incoming from some sources to subscribers

# Dependencies
## List of dependencies
* cmake
* apr (Apache Portable Runtime)
* log4cpp
* jsoncpp

## Fetch deps on Debian/Ubuntu

	apt-get install liblog4cpp5-dev libjsoncpp-dev libapr1-dev

## Fetch deps on FreeBSD

	pkg install log4cpp jsoncpp apr

# How to build
	mkdir build
	cd build
	cmake ..
	make
