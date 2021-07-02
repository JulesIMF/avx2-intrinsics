# AVX2 intrinsics

Exploring AVX2 intrinsics. Project consists of two Win32 applications making some calculations. With `tab` you can switch SISD calculation (no optimisaton) to SIMD (AVX 2 optimisation) to compare changes in performance.

# `Intrinsic_1` - Mandelbrot fractal
This application calculates Mandelbrot fractal. You can move the center position using pointer keys and zoom it using `+` and `-`.

# `Intrinsic_2` - Cat on the table
This application places cat image over the table image using alpha-blending of BMP. You can move the center position using pointer keys.

# Prerequisites
These applications are builded in Visual Studio 2019 and run under Windows NT.
