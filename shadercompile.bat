@echo off
set "SHADER_DIR=shaders"

for %%f in ("%SHADER_DIR%\*.vert.hlsl") do (
    shadercross "%%f" -o "%%~dpnf.dxil"
    shadercross "%%f" -o "%%~dpnf.msl"
)
for %%f in ("%SHADER_DIR%\*.frag.hlsl") do (
    shadercross "%%f" -o "%%~dpnf.dxil"
    shadercross "%%f" -o "%%~dpnf.msl"
)
for %%f in ("%SHADER_DIR%\*.comp.hlsl") do (
    shadercross "%%f" -o "%%~dpnf.dxil"
    shadercross "%%f" -o "%%~dpnf.msl"
)