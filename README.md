
Setup MINGW Environment
-----------------------

        pacman -S mingw-w64-ucrt-x86_64-gcc
        pacman -S mingw-w64-ucrt-x86_64-curl
        pacman -S make
        # This was needed to get GIT to remember credentials. Probably
        # not the most secure method.
        git config --global credential.helper store

Sound
-----

After trying a few things I ended up using the Windows WASAPI interface.

[Samples From MSFT](https://learn.microsoft.com/en-us/samples/microsoft/windows-universal-samples/windowsaudiosession/)

https://medium.com/@shahidahmadkhan86/sound-in-windows-the-wasapi-in-c-23024cdac7c6

WebSockets
-----------

https://pages.ably.com/hubfs/the-websocket-handbook.pdf
