cmake_minimum_required(VERSION 2.8.10)

# This can be read from ${PROJECT_NAME} after project() is called
project(emt-chatserver)
set(PROJECT_NAME_CAPITALIZED "EMT-chatserver")

set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 1)
set(VERSION_EXTRA "" CACHE STRING "Stuff to append to version string")

# Change to false for releases
set(DEVELOPMENT_BUILD TRUE)

execute_process(COMMAND git rev-list --tags --max-count=1
        WORKING_DIRECTORY "${GENERATE_VERSION_SOURCE_DIR}"
        OUTPUT_VARIABLE VERSION_GITTAG_COMMIT OUTPUT_STRIP_TRAILING_WHITESPACE
	ERROR_QUIET)
execute_process(COMMAND git describe --tags "${VERSION_GITTAG_COMMIT}"
	WORKING_DIRECTORY "${GENERATE_VERSION_SOURCE_DIR}"
	OUTPUT_VARIABLE VERSION_GITTAG OUTPUT_STRIP_TRAILING_WHITESPACE
	ERROR_QUIET)
set(VERSION_STRING "${VERSION_GITTAG}")
if(VERSION_EXTRA)
	set(VERSION_STRING ${VERSION_STRING}-${VERSION_EXTRA})
elseif(DEVELOPMENT_BUILD)
	set(VERSION_STRING "${VERSION_STRING}-dev")
endif()

message(STATUS "*** Will build version ${VERSION_STRING} ***")


# Configuration options
set(DEFAULT_RUN_IN_PLACE FALSE)
if(WIN32)
	set(DEFAULT_RUN_IN_PLACE TRUE)
endif()
set(RUN_IN_PLACE ${DEFAULT_RUN_IN_PLACE} CACHE BOOL
	"Run directly in source directory structure")

set(WARN_ALL TRUE CACHE BOOL "Enable -Wall for Release build")

if(NOT CMAKE_BUILD_TYPE)
	# Default to release
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type: Debug or Release" FORCE)
endif()


# Included stuff
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# Modules
find_library(APR_LIBRARY REQUIRED)
find_package("Apr")

find_library(LOG4CPP_LIBRARY REQUIRED)
find_package("Log4cpp")

find_library(JSON_LIBRARY REQUIRED)
find_package("Json")

# Installation

if(WIN32)
	set(SHAREDIR ".")
	set(BINDIR "bin")
	set(DOCDIR "doc")
	set(EXAMPLE_CONF_DIR ".")
elseif(UNIX) # Linux, BSD etc
	set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++11")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -pthread -std=c++11")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pthread -std=c++11")

	set(BINDIR "${CMAKE_INSTALL_PREFIX}/bin")
	set(DOCDIR "${CMAKE_INSTALL_PREFIX}/share/doc/${PROJECT_NAME}")
	set(MANDIR "${CMAKE_INSTALL_PREFIX}/share/man")
	set(EXAMPLE_CONF_DIR ${DOCDIR})
endif()

set(CUSTOM_SHAREDIR "" CACHE STRING "Directory to install data files into")
if(NOT CUSTOM_SHAREDIR STREQUAL "")
	set(SHAREDIR "${CUSTOM_SHAREDIR}")
	message(STATUS "Using SHAREDIR=${SHAREDIR}")
endif()

set(CUSTOM_BINDIR "" CACHE STRING "Directory to install binaries into")
if(NOT CUSTOM_BINDIR STREQUAL "")
	set(BINDIR "${CUSTOM_BINDIR}")
	message(STATUS "Using BINDIR=${BINDIR}")
endif()

set(CUSTOM_DOCDIR "" CACHE STRING "Directory to install documentation into")
if(NOT CUSTOM_DOCDIR STREQUAL "")
	set(DOCDIR "${CUSTOM_DOCDIR}")
	message(STATUS "Using DOCDIR=${DOCDIR}")
endif()

set(CUSTOM_MANDIR "" CACHE STRING "Directory to install manpages into")
if(NOT CUSTOM_MANDIR STREQUAL "")
	set(MANDIR "${CUSTOM_MANDIR}")
	message(STATUS "Using MANDIR=${MANDIR}")
endif()

set(CUSTOM_EXAMPLE_CONF_DIR "" CACHE STRING "Directory to install example config file into")
if(NOT CUSTOM_EXAMPLE_CONF_DIR STREQUAL "")
	set(EXAMPLE_CONF_DIR "${CUSTOM_EXAMPLE_CONF_DIR}")
	message(STATUS "Using EXAMPLE_CONF_DIR=${EXAMPLE_CONF_DIR}")
endif()

# Subdirectories
# Be sure to add all relevant definitions above this

add_subdirectory(src)

install(FILES "README.md" DESTINATION "${DOCDIR}")
install(FILES "conf/config.json.example" DESTINATION "${EXAMPLE_CONF_DIR}")
install(FILES "conf/log4cpp.properties.example" DESTINATION "${EXAMPLE_CONF_DIR}")

if (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
	install(FILES "service/freebsd"
		DESTINATION "/usr/local/etc/rc.d"
		PERMISSIONS OWNER_EXECUTE OWNER_READ
			GROUP_READ
		RENAME "emt-chatserver")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "OpenBSD")
	install(FILES "service/openbsd"
		DESTINATION "/etc/rc.d"
		PERMISSIONS OWNER_EXECUTE OWNER_READ
			GROUP_READ
		RENAME "emt-chatserver")
endif()

# CPack

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A chatserver based on REST messages for Epixel/Minetest servers")
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
set(CPACK_PACKAGE_VENDOR "nerzhul")
set(CPACK_PACKAGE_CONTACT "Loic Blot <loic.blot@unix-experience.fr>")

if(WIN32)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${VERSION_STRING}-win64")
	else()
		set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${VERSION_STRING}-win32")
	endif()

	set(CPACK_GENERATOR ZIP)
elseif(APPLE)
	set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)
	set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${VERSION_STRING}-osx")
	set(CPACK_GENERATOR ZIP)
else()
	set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${VERSION_STRING}-linux")
	set(CPACK_GENERATOR TGZ)
	set(CPACK_SOURCE_GENERATOR TGZ)
endif()
