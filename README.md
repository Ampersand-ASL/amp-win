
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

https://medium.com/@shahidahmadkhan86/sound-in-windows-the-wasapi-in-c-23024cdac7c6
