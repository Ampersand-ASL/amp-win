# ASL Ampersand Project
# Copyright (C) 2025, Bruce MacKinnon KC1FSZ
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# ( at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 23)
project(amp-win C CXX ASM)

add_compile_options(-fstack-protector-all -Wall -Wpedantic)

add_executable(main
  src/main.cpp
  src/MainWindow.cpp
  src/amp-thread.cpp
  amp-core/src/EventLoop.cpp
  amp-core/src/Message.cpp
  amp-core/src/Line.cpp
#  amp-core/src/LineUsb.cpp
  amp-core/src/LineIAX2.cpp
  amp-core/src/IAX2FrameFull.cpp
  amp-core/src/IAX2Util.cpp
  amp-core/src/MessageBus.cpp
  amp-core/src/RegisterTask.cpp
#  amp-core/src/StatsTask.cpp
#  amp-core/src/ManagerTask.cpp
  amp-core/src/RetransmissionBufferStd.cpp
  amp-core/src/Transcoder_G711_ULAW.cpp
  amp-core/src/Transcoder_SLIN_48K.cpp
  amp-core/src/Transcoder_SLIN.cpp
  amp-core/src/Transcoder_SLIN_16K.cpp
  amp-core/src/AdaptorIn.cpp
  amp-core/src/AdaptorOut.cpp
  amp-core/src/Resampler.cpp
  kc1fsz-tools-cpp/src/Common.cpp
  kc1fsz-tools-cpp/src/NetUtils.cpp
  kc1fsz-tools-cpp/src/DTMFDetector2.cpp
  kc1fsz-tools-cpp/src/MicroDNS.cpp
  kc1fsz-tools-cpp/src/StdPollTimer.cpp
  kc1fsz-tools-cpp/src/linux/StdClock.cpp
  kc1fsz-tools-cpp/src/md5/md5c.c
  itu-g711-codec/src/codec.cpp
  itu-g711-codec/src/Plc.cpp
  cmsis-dsp-mock/src/main.cpp
#  amp-core/sound-map/src/sound-map.c
)

target_include_directories(main PRIVATE src)
target_include_directories(main PRIVATE include)
target_include_directories(main PRIVATE kc1fsz-tools-cpp/include)
target_include_directories(main PRIVATE itu-g711-codec/src)
target_include_directories(main PRIVATE cmsis-dsp-mock/include)
target_include_directories(main PRIVATE amp-core/src)
target_include_directories(main PRIVATE amp-core/include)
#target_include_directories(main PRIVATE sound-map/include)
target_link_libraries(main -lws2_32 -lcurl)
target_link_options(main PRIVATE -mwindows)

# ----- hello-w32 ----------------------------------------------------

add_executable(hello-w32
  src/tests/hello-w32.cpp
)
target_include_directories(hello-w32 PRIVATE src)
target_include_directories(hello-w32 PRIVATE include)
target_include_directories(hello-w32 PRIVATE kc1fsz-tools-cpp/include)
target_link_options(hello-w32 PRIVATE -mwindows)

