
Setup MINGW Environment
-----------------------

        pacman -S mingw-w64-ucrt-x86_64-gcc
        pacman -S make
        # This was needed to get GIT to remember credentials. Probably
        # not the most secure method.
        git config --global credential.helper store

