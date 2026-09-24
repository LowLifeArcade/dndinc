# DND INC

A small game I'm making to learn C

### Install
[SDL3](https://gist.github.com/NoxFly/1067c9fc24024d26b51a6825de5cff74)
> you can install via home brew with
```sh
brew install sdl3
```
> you might need
```sh
brew info sdl3
brew --prefix sdl3
```
### Build and Run
```sh
clang  ./src/*.c -o main $(pkg-config --cflags --libs sdl3)
./main
```