# PhysicsSandbox
This started out as an experiment to render falling objects, using Box2D, with a subclassed QOpenGLWidget, and turned into a challenge to integrate Skia into a framebuffer used by a custom QQuickItem class.

I apologize in advance for the quality of the code; it's intended as a proof of concept. If its working well enough, I'll tidy up the code.

## Credits
Parts of the code were based on QtSkia: [https://github.com/QtSkia/QtSkia](https://github.com/QtSkia/QtSkia).

## Notes
I built Skia for my Linux OS (endeavourOS), using this command line passed to `bin/gn' inside the cloned git repository for Skia:

```
bin/gn gen out/Static-Debug --args='is_debug=true skia_use_vulkan=true skia_use_system_expat=false skia_use_system_libjpeg_turbo=false skia_use_system_libpng=false skia_use_system_libwebp=false skia_use_system_zlib=false skia_use_system_freetype2=false skia_use_system_harfbuzz=false skia_pdf_subset_harfbuzz=true skia_use_system_icu=false skia_enable_skottie=true'
ninja -C out/Static-Debug
```
