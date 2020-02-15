# DNA OpenGL screensaver
This is the Linux port of my student OpenGL screensaver written in 2003.
The program was created in the development environment Netbeans 8.2.

# To compile the program:
1. apt install clang libxrandr-dev libglew-dev
2. git clone https://github.com/Naezzhy/dna_screensaver.git
3. cd dna_screensaver
4. clang++ -lGL -lGLU -lXfixes -lX11 -lXrandr -o screensaver_dna screensaver_dna.cpp
