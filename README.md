This is the repo that builds a Windows server that supports linking between radios and nodes on the ASL network. 

[Most of the Ampersand project documentation is here](https://mackinnon.info/ampersand/).

> [!IMPORTANT]
> **If you are just looking to install/run the server, you probably want to</span> [start here](https://github.com/Ampersand-ASL/amp-win/blob/main/docs/user.md)!**

> [!IMPORTANT]
> If you are using the AllStarLink system please [make a dontation](https://www.allstarlink.org/about/donate.php) to support the network. 

# Setup MINGW Environment

Ampersand is developed using the MinGW environment on Windows. Once MinGW is 
setup the build process is almost identical to that on Linux.

        # Some package prerequisites:
        pacman -S mingw-w64-ucrt-x86_64-gcc
        pacman -S mingw-w64-ucrt-x86_64-curl
        pacman -S make zip
        # Needed to get the xxd command
        pacman -S vim
        # This was needed to get GIT to remember credentials. Probably
        # not the most secure method.
        git config --global credential.helper store

# Building the Windows Server

Builds are performed from the bash prompt using the MinGW environment.

        git clone https://github.com/Ampersand-ASL/amp-win.git
        cd amp-win
        mkdir build
        cd build
        cmake ..
        make amp-win        

# Packaging the Windows Server

Packaging is performed from the bash prompt using the MinGW environment.

        export AMP_WIN_VERSION=20260513
        export AMP_ARCH=$(uname -m)
        scripts/make-package.sh
        # Move the .zip file to S3

# Sound

After trying a few things I ended up using the Windows WASAPI interface.

[Samples From MSFT](https://learn.microsoft.com/en-us/samples/microsoft/windows-universal-samples/windowsaudiosession/)

https://medium.com/@shahidahmadkhan86/sound-in-windows-the-wasapi-in-c-23024cdac7c6

# WebSockets

https://pages.ably.com/hubfs/the-websocket-handbook.pdf
