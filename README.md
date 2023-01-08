
# web-stack-project

## Build project

- Open project in terminal
- Run `meson setup builddir`, or `CXX=clang-<version> meson setup builddir` if clang and clang-version is different
- Navigate into builddir, `cd builddir`
- Compile `meson compile`
- Run, `./browser` or `./server`

## VS Code development setup

### Install
- `clang-15` recommended, `clang-15` based, check with `clang --version`
- [Meson](https://mesonbuild.com/), build system
- [Meson VS Code extension](https://marketplace.visualstudio.com/items?itemName=mesonbuild.mesonbuild)
- [VSCode - clangd](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd), linter
- [VSCode - CodeLLDB](https://marketplace.visualstudio.com/items?itemName=vadimcn.vscode-lldb), debugger
- [VSCode - Better C++ Syntax](https://marketplace.visualstudio.com/items?itemName=jeff-hykin.better-cpp-syntax), syntax higlighting

### Config

Add these to `settings.json` if necessary
```json
{
    // probably necessary
    "clangd.onConfigChanged": "restart",

    // only necessary if default `/usr/bin/clangd` isn't the correct version
    "clangd.path": "/usr/bin/clangd-<version>", // <version> is a place holder, eg, `clangd-15`
}
``` 
