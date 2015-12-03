# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

# - Find Apache Portable Runtime
# Find the LOG4CPP includes and libraries
# This module defines
#  LOG4CPP_INCLUDE_DIR and LOG4CPPUTIL_INCLUDE_DIR, where to find apr.h, etc.
#  LOG4CPP_LIBRARIES and LOG4CPPUTIL_LIBRARIES, the libraries needed to use LOG4CPP.
#  LOG4CPP_FOUND and LOG4CPPUTIL_FOUND, If false, do not try to use LOG4CPP.
# also defined, but not for general use are
#  LOG4CPP_LIBRARY and LOG4CPPUTIL_LIBRARY, where to find the LOG4CPP library.

# LOG4CPP first.

FIND_PATH(LOG4CPP_INCLUDE_DIR OstreamAppender.hh
  /usr/local/include/log4cpp
  /usr/include/log4cpp
  /usr/local/log4cpp/include/log4cpp
)

SET(LOG4CPP_NAMES ${LOG4CPP_NAMES} log4cpp)
FIND_LIBRARY(LOG4CPP_LIBRARY
  NAMES ${LOG4CPP_NAMES}
  HINTS
    /opt/homebrew/opt/log4cpp/lib
  PATHS
    /usr/lib
    /usr/local/lib
    /usr/local/log4cpp/lib
  )

IF (LOG4CPP_LIBRARY AND LOG4CPP_INCLUDE_DIR)
    SET(LOG4CPP_LIBRARIES ${LOG4CPP_LIBRARY})
    SET(LOG4CPP_FOUND "YES")
ELSE (LOG4CPP_LIBRARY AND LOG4CPP_INCLUDE_DIR)
  SET(LOG4CPP_FOUND "NO")
ENDIF (LOG4CPP_LIBRARY AND LOG4CPP_INCLUDE_DIR)


IF (LOG4CPP_FOUND)
   IF (NOT LOG4CPP_FIND_QUIETLY)
      MESSAGE(STATUS "Found log4cpp headers: ${LOG4CPP_INCLUDE_DIR}")
      MESSAGE(STATUS "Found log4cpp library: ${LOG4CPP_LIBRARIES}")
   ENDIF (NOT LOG4CPP_FIND_QUIETLY)
ELSE (LOG4CPP_FOUND)
   IF (LOG4CPP_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find log4cpp library")
   ENDIF (LOG4CPP_FIND_REQUIRED)
ENDIF (LOG4CPP_FOUND)
