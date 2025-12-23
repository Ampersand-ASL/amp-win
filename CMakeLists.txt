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

add_compile_options(-fstack-protector-all -Wall -Wpedantic -g)

add_executable(main
  src/main.cpp
  src/MainWindow.cpp
  src/amp-thread.cpp
  src/service-thread.cpp
  src/LineRadioWin.cpp
  amp-core/src/EventLoop.cpp
  amp-core/src/Message.cpp
  amp-core/src/Line.cpp
  amp-core/src/LineRadio.cpp
  amp-core/src/LineIAX2.cpp
  amp-core/src/IAX2FrameFull.cpp
  amp-core/src/IAX2Util.cpp
  amp-core/src/RegisterTask.cpp
  amp-core/src/StatsTask.cpp
  amp-core/src/RetransmissionBufferStd.cpp
  amp-core/src/Transcoder_G711_ULAW.cpp
  amp-core/src/Transcoder_SLIN_48K.cpp
  amp-core/src/Transcoder_SLIN_16K.cpp
  amp-core/src/Bridge.cpp
  amp-core/src/BridgeCall.cpp
  amp-core/src/BridgeIn.cpp
  amp-core/src/BridgeOut.cpp
  amp-core/src/Resampler.cpp
  kc1fsz-tools-cpp/src/Common.cpp
  kc1fsz-tools-cpp/src/NetUtils.cpp
  kc1fsz-tools-cpp/src/DTMFDetector2.cpp
  kc1fsz-tools-cpp/src/MicroDNS.cpp
  kc1fsz-tools-cpp/src/StdPollTimer.cpp
  kc1fsz-tools-cpp/src/linux/StdClock.cpp
  kc1fsz-tools-cpp/src/win32/Win32MTLog.cpp
  kc1fsz-tools-cpp/src/md5/md5c.c
  itu-g711-codec/src/codec.cpp
  itu-g711-codec/src/Plc.cpp
  cmsis-dsp-mock/src/main.cpp
  kc1fsz-sdrc/sw/src/TestToneGenerator.cpp
  kc1fsz-sdrc/sw/src/IDToneGenerator.cpp
  kc1fsz-sdrc/sw/src/CourtesyToneGenerator.cpp
  kc1fsz-sdrc/sw/src/Tx.cpp
  kc1fsz-sdrc/sw/src/TxControl.cpp
  ed25519/src/add_scalar.c
  ed25519/src/ge.c
  ed25519/src/keypair.c
  ed25519/src/seed.c
  ed25519/src/sign.c
  ed25519/src/fe.c
  ed25519/src/key_exchange.c
  ed25519/src/sc.c
  ed25519/src/sha512.c
  ed25519/src/verify.c
)

target_include_directories(main PRIVATE src)
target_include_directories(main PRIVATE include)
target_include_directories(main PRIVATE kc1fsz-tools-cpp/include)
target_include_directories(main PRIVATE itu-g711-codec/src)
target_include_directories(main PRIVATE cmsis-dsp-mock/include)
target_include_directories(main PRIVATE amp-core/src)
target_include_directories(main PRIVATE amp-core/include)
target_include_directories(main PRIVATE kc1fsz-sdrc/sw/include)
target_include_directories(main PRIVATE kc1fsz-sdrc/sw/src)
target_include_directories(main PRIVATE ed25519/src)
target_link_libraries(main -lws2_32 -lcurl -lole32)
target_link_options(main PRIVATE -mwindows)
# Needed to expand the size of the stack in the MinGW environment
target_link_options(main PRIVATE -Wl,--stack,4096000)

# ----- The Web UI version ---------------------------------------------------

add_executable(main2
  src/main2.cpp
  src/amp-thread.cpp
  src/service-thread.cpp
  src/LineRadioWin.cpp
  amp-core/src/EventLoop.cpp
  amp-core/src/Message.cpp
  amp-core/src/Line.cpp
  amp-core/src/LineRadio.cpp
  amp-core/src/LineIAX2.cpp
  amp-core/src/IAX2FrameFull.cpp
  amp-core/src/IAX2Util.cpp
  amp-core/src/RegisterTask.cpp
  amp-core/src/StatsTask.cpp
  amp-core/src/RetransmissionBufferStd.cpp
  amp-core/src/Transcoder_G711_ULAW.cpp
  amp-core/src/Transcoder_SLIN_48K.cpp
  amp-core/src/Transcoder_SLIN_16K.cpp
  amp-core/src/Bridge.cpp
  amp-core/src/BridgeCall.cpp
  amp-core/src/BridgeIn.cpp
  amp-core/src/BridgeOut.cpp
  amp-core/src/Resampler.cpp
  amp-core/src/ui-thread.cpp
  kc1fsz-tools-cpp/src/Common.cpp
  kc1fsz-tools-cpp/src/NetUtils.cpp
  kc1fsz-tools-cpp/src/DTMFDetector2.cpp
  kc1fsz-tools-cpp/src/MicroDNS.cpp
  kc1fsz-tools-cpp/src/StdPollTimer.cpp
  kc1fsz-tools-cpp/src/linux/StdClock.cpp
  kc1fsz-tools-cpp/src/win32/Win32MTLog.cpp
  kc1fsz-tools-cpp/src/md5/md5c.c
  itu-g711-codec/src/codec.cpp
  itu-g711-codec/src/Plc.cpp
  cmsis-dsp-mock/src/main.cpp
  kc1fsz-sdrc/sw/src/TestToneGenerator.cpp
  kc1fsz-sdrc/sw/src/IDToneGenerator.cpp
  kc1fsz-sdrc/sw/src/CourtesyToneGenerator.cpp
  kc1fsz-sdrc/sw/src/Tx.cpp
  kc1fsz-sdrc/sw/src/TxControl.cpp
  ed25519/src/add_scalar.c
  ed25519/src/ge.c
  ed25519/src/keypair.c
  ed25519/src/seed.c
  ed25519/src/sign.c
  ed25519/src/fe.c
  ed25519/src/key_exchange.c
  ed25519/src/sc.c
  ed25519/src/sha512.c
  ed25519/src/verify.c
)

target_include_directories(main2 PRIVATE src)
target_include_directories(main2 PRIVATE include)
target_include_directories(main2 PRIVATE kc1fsz-tools-cpp/include)
target_include_directories(main2 PRIVATE itu-g711-codec/src)
target_include_directories(main2 PRIVATE cmsis-dsp-mock/include)
target_include_directories(main2 PRIVATE amp-core/src)
target_include_directories(main2 PRIVATE amp-core/include)
target_include_directories(main2 PRIVATE kc1fsz-sdrc/sw/include)
target_include_directories(main2 PRIVATE kc1fsz-sdrc/sw/src)
target_include_directories(main2 PRIVATE ed25519/src)
target_include_directories(main2 PRIVATE json/include)
target_include_directories(main2 PRIVATE cpp-httplib)
target_link_libraries(main2 -lws2_32 -lcurl -lole32)
target_link_options(main2 PRIVATE -mwindows)
# Needed to expand the size of the stack in the MinGW environment
target_link_options(main2 PRIVATE -Wl,--stack,4096000)

# ----- Local Parrot ------------------------------------------------------

add_executable(main-local-parrot
  src/demos/main-local-parrot.cpp
  src/demos/amp-thread-local-parrot.cpp
  src/MainWindow.cpp
  src/LineRadioWin.cpp
  amp-core/src/EventLoop.cpp
  amp-core/src/Message.cpp
  amp-core/src/Line.cpp
  amp-core/src/LineRadio.cpp
  amp-core/src/IAX2Util.cpp
  amp-core/src/Transcoder_G711_ULAW.cpp
  amp-core/src/Transcoder_SLIN_48K.cpp
  amp-core/src/Transcoder_SLIN_16K.cpp
  amp-core/src/Bridge.cpp
  amp-core/src/BridgeCall.cpp
  amp-core/src/BridgeIn.cpp
  amp-core/src/BridgeOut.cpp
  amp-core/src/Resampler.cpp
  kc1fsz-tools-cpp/src/Common.cpp
  kc1fsz-tools-cpp/src/StdPollTimer.cpp
  kc1fsz-tools-cpp/src/linux/StdClock.cpp
  kc1fsz-tools-cpp/src/win32/Win32MTLog.cpp
  kc1fsz-tools-cpp/src/DTMFDetector2.cpp
  itu-g711-codec/src/codec.cpp
  itu-g711-codec/src/Plc.cpp
  cmsis-dsp-mock/src/main.cpp
  kc1fsz-sdrc/sw/src/TestToneGenerator.cpp
  kc1fsz-sdrc/sw/src/IDToneGenerator.cpp
  kc1fsz-sdrc/sw/src/CourtesyToneGenerator.cpp
  kc1fsz-sdrc/sw/src/Tx.cpp
  kc1fsz-sdrc/sw/src/TxControl.cpp
)

target_include_directories(main-local-parrot PRIVATE src)
target_include_directories(main-local-parrot PRIVATE include)
target_include_directories(main-local-parrot PRIVATE kc1fsz-tools-cpp/include)
target_include_directories(main-local-parrot PRIVATE itu-g711-codec/src)
target_include_directories(main-local-parrot PRIVATE cmsis-dsp-mock/include)
target_include_directories(main-local-parrot PRIVATE amp-core/src)
target_include_directories(main-local-parrot PRIVATE amp-core/include)
target_include_directories(main-local-parrot PRIVATE kc1fsz-sdrc/sw/include)
target_include_directories(main-local-parrot PRIVATE kc1fsz-sdrc/sw/src)
target_link_libraries(main-local-parrot -lws2_32 -lole32)
target_link_options(main-local-parrot PRIVATE -mwindows)

# ----- hello-w32 ----------------------------------------------------

add_executable(hello-w32
  src/tests/hello-w32.cpp
)
target_include_directories(hello-w32 PRIVATE src)
target_include_directories(hello-w32 PRIVATE include)
target_include_directories(hello-w32 PRIVATE kc1fsz-tools-cpp/include)
target_link_options(hello-w32 PRIVATE -mwindows)

# ----- hello-audio ----------------------------------------------------

add_executable(hello-audio
  src/tests/hello-audio-1.cpp
)
target_include_directories(hello-audio PRIVATE src)
target_include_directories(hello-audio PRIVATE include)
target_include_directories(hello-audio PRIVATE kc1fsz-tools-cpp/include)
target_link_libraries(hello-audio -lws2_32 -lcurl -lwinmm)
target_link_options(hello-audio PRIVATE -mwindows)

# ----- hello-audio-2 ----------------------------------------------------

add_executable(hello-audio-2
  src/tests/hello-audio-2.cpp
  kc1fsz-tools-cpp/src/linux/StdClock.cpp
)
target_include_directories(hello-audio-2 PRIVATE src)
target_include_directories(hello-audio-2 PRIVATE include)
target_include_directories(hello-audio-2 PRIVATE kc1fsz-tools-cpp/include)
target_link_libraries(hello-audio-2 -lole32)
target_link_options(hello-audio-2 PRIVATE -mwindows)

# ----- hello-http-server -------------------------------------------------

add_executable(hello-http-server
  src/demos/hello-http-server.cpp
  kc1fsz-tools-cpp/src/linux/StdClock.cpp
)
target_include_directories(hello-http-server PRIVATE src)
target_include_directories(hello-http-server PRIVATE include)
target_include_directories(hello-http-server PRIVATE kc1fsz-tools-cpp/include)
target_include_directories(hello-http-server PRIVATE cpp-httplib)
target_link_libraries(hello-http-server -lws2_32)
target_link_options(hello-http-server PRIVATE -mwindows)
