
# Setup MINGW Environment

        pacman -S mingw-w64-ucrt-x86_64-gcc
        pacman -S mingw-w64-ucrt-x86_64-curl
        pacman -S make zip
        # Needed to get the xxd command
        pacman -S vim
        # This was needed to get GIT to remember credentials. Probably
        # not the most secure method.
        git config --global credential.helper store

# Building the Windows Server

        git clone https://github.com/Ampersand-ASL/amp-win.git
        cd amp-win
        mkdir build
        cd build
        cmake ..
        make amp-win        

# Packaging the Windows Server

        export AMP_WIN_VERSION=20260207
        export AMP_ARCH=$(uname -m)
        scripts/make-package.sh
        # Move the .zip file to S3

# Sound

After trying a few things I ended up using the Windows WASAPI interface.

[Samples From MSFT](https://learn.microsoft.com/en-us/samples/microsoft/windows-universal-samples/windowsaudiosession/)

https://medium.com/@shahidahmadkhan86/sound-in-windows-the-wasapi-in-c-23024cdac7c6

# WebSockets

https://pages.ably.com/hubfs/the-websocket-handbook.pdf
